#include "patricia.h"

#if defined(DEBUG)
#include <cstdio>
#endif

namespace shutup {
//Patricia::Node
void Patricia::Node::merge(allocator<Node*> &a, Node *n) {
	if (terminal() || root()) {
		return;
	}
	//create merged node (マージされる時、必ず終端ノードとなる)
	Node *merged = Node::new_node(a, parent_, bytes_, len_ + n->len_, true);
	std::memcpy(merged->bytes_ + len_, n->bytes_, n->len_);
	//nに子供があればmergedに付け直す.
	if (n->children_.size() > 0) {
		for (Node *c : n->children_) {
			c->parent_ = merged;
		}
		merged->children_ = std::move(n->children_); //moveしてくれるのでcopy発生しないはず.
	}
	//replace self with merged node in siblings
	remove_from_siblings();
	parent_->children_.push_back(merged);
	parent_->sort_children();
	//マージ元になったオブジェクトは破棄される
	n->free(a); //nのchildはすでにmergedに引き渡されている
	free(a); //thisはchildをn以外に持たないはず.
}
void Patricia::Node::destroy(allocator<Node*> &a) {
	for (Node *child : children_) {
		child->destroy(a);
	}
	free(a);
}
void Patricia::Node::add_child(allocator<Node*> &a, IMatcher &m, const u8 *b, int l) {
	if (l == 0) {
		//l == 0でここに来るということは,このノード自身とマッチしているということなので、_termをtrueにする. 
		term_ = true;
		return; 
	}
	for (auto iter = children_.begin(); iter != children_.end(); iter++) {
		Node *child = *iter;
		int ofs = 0, i = m.match(b, l, child->bytes_, child->len_, &ofs);
		if (i > 0) {
			//child->bytes_のi byteまでで部分一致し、その時bはofs byte読まれた.
			//一般的なmatchについてi != ofsだが、短いものにマッチしているということは、短い方がより汎用的な表現であると仮定する.
			//したがって、短い方をこのノードのchildとして残すノードのデータとして採用する.child->bytes_, bをスプリットして,
			//child->bytes_[0:i] or b[0:ofs]のうち短い方をこのノードのchildとして残し、child->bytes_[i + 1:]とb[ofs + 1:]をその子供に追加する.
			//child->bytes_[i + 1:]を保持するノードは今のノードの子供を引き継ぐ.
			Node *r, *b1, *b2, *new_parent;
			bool term;
			//rが終端ノードになるケース = child->len_ == i or ofs == l
			//child->len_ == iとなるケース: b, lがchild->bytes_, len_を含んでいる場合. r = (bytes_, len_)となるため終端となる
			//ofs == lとなるケース: b, lがchild->bytes_, len_に含まれている場合, r = (b, l)となるため終端となる.
			//終端となった場合、長さ0のデータを含むノードは作成しない
			if (i < ofs) {
				term = (child->len_ == i);
				r = Node::new_node(a, this, child->bytes_, i, term);
				b1 = Node::new_node(a, r, b + ofs, l - ofs, true);
				if (term) {
					//b2が作成されないため、rを子供を引き継ぐノードにする.
					new_parent = r;
				} else {
					b2 = Node::new_node(a, r, child->bytes_ + i, child->len_ - i, true);
					new_parent = b2;
				}
			} else {
				term = (l == ofs);
				r = Node::new_node(a, this, b, ofs, term);
				b1 = Node::new_node(a, r, child->bytes_ + i, child->len_ - i, true);
				//b1は必ず作成される(l == ofs かつ len_ == iなら一致してしまうため、このようなことは発生しない)
				new_parent = b1;
				if (!term) {
					b2 = Node::new_node(a, r, b + ofs, l - ofs, true);
				}
			}
			//このchildを取り除く.全てのリンクはまだ生きている.
			children_.erase(iter);
			//b1に今のchildのchild_を全部くっつける.
			for (Node *n : child->children_) {
				n->parent_ = new_parent; //parentをnew_parentにとっかえる.
			}
			new_parent->children_ = std::move(child->children_); //moveしてくれるのでcopy発生しないはず.
			//tにb1, b2を追加する.(new_parentはb1かb2のどちらかであるため、追加しなくて良い)
			r->children_.push_back(b1);
			if (!term) {
				r->children_.push_back(b2);
				r->sort_children();//辞書順に並べる.
			}
			//tを新しく追加する.
			children_.push_back(r);
			sort_children();//辞書順に並べる.
			//childは解放する.
			child->free(a);
			return;
		}
	}
	//全く一致するchildがなかったので新しく追加する.このノードは終端となる.
	children_.push_back(Node::new_node(a, this, b, l, true));
}
bool Patricia::Node::compare(const Node *left, const Node *right) {
	int l = std::min(left->len_, right->len_);
	int r = std::memcmp(left->bytes_, right->bytes_, l);
	if (r == 0) {
		return left->len_ < right->len_;
	} else {
		return r < 0;
	}
}
void Patricia::Node::sort_children() {
	std::sort(children_.begin(), children_.end(), compare);
}
Patricia::Node *Patricia::Node::find_child(IMatcher &m, const u8 *b, int l, int *ofs) {
	if (l == 0) {
		return terminal() ? this : nullptr;
	}
	for (Node *child : children_) {
		//完全に一致したノードがある時のみ.
		int r = m.match(b, l, child->bytes_, child->len_, ofs);
		/*#if defined(DEBUG)
		char buffer[256];
		std::strncpy(buffer, reinterpret_cast<const char *>(b), l + 1);
		std::printf("match with %s ", buffer); child->dump();
		std::printf("result ofs/r %d/%d\n", *ofs, r);
		#endif */
		if (child->len_ == r) {
			return child;
		}
	}
	return nullptr;
}
void Patricia::Node::remove_from_siblings() {
	NodeList &siblings = parent_->children_;
	for (auto iter = siblings.begin(); iter != siblings.end(); iter++) {
		if (this == *iter) {
			siblings.erase(iter);
			break;
		}
	}
}
void Patricia::Node::remove_self(allocator<Node*> &a) {
	if (root()) {
		//rootノードは除去できない.
		return;
	}
	//兄弟ノード
	NodeList &siblings = parent_->children_;
	//マージするケース：
	size_t sz = children_.size();
	//std::printf("node: remove: %lu %s ", sz, terminal() ? "t" : "-"); dump();
	if (sz == 0) {
		remove_from_siblings();
		//terminal() == trueであるはず.
		//- このノードが子供を持たない場合は単純に除去される。結果兄弟ノードの数が１になってしまった場合には,
		//そのノードと親とのマージを試みる(親がrootないし終端ノードの場合マージされない)
		/* eg) +ab +ac +abc +abd -ac => 
			__root__
				ab
					c
					d
		*/
		parent_->dump();
		if (siblings.size() == 1) {
			//merge this child node with parent.
			//parent_とsiblings[0]はこの中で解放される.
			parent_->merge(a, siblings[0]);
		}
		destroy(a);
	} else if (terminal()) {
		//子供を持つ場合で終端ノードの場合.
		term_ = false; //終端ノードではなくなるだけ.(子供があるため削除できない).
		if (sz == 1) {
			//- このノードが1つの子供を持つ場合で、終端ノードだった => このノードと子供をマージしてparent_に追加する.
			// eg) +ab +abc +abcd -ab => abc
			// thisとchildren_[0]はこのmergeの中で解放される.従ってthisにはこれ以降触らないこと.
			merge(a, children_[0]);
		}
	} else {
		//実用上、このケースは発生しない(remove_sliceで終端ノードが見つかり、かつ対象スライスを被覆していることをチェックしているため).
		//エラーにしても良いが、ライブラリ内部での利用で便利な可能性があるため残す。このノード以下のすべてのノードを削除する.
		remove_from_siblings();
		destroy(a);
	}
}
//Patricia
Patricia::~Patricia() {
	root_->destroy(alloc_);
	root_ = nullptr;
}
void Patricia::add_slice(const u8 *b, int l) {
	int ofs;
	Node *n = find_node(b, l, &ofs);
	if (ofs <= l) {
		//ノードに含まれているsliceによって被覆されなかった残りの部分を木構造に追加する.
		n->add_child(alloc_, *matcher_, b + ofs, l - ofs);
	}
}
void Patricia::remove_slice(const u8 *b, int l) {
	int ofs;
	Node *n = find_node(b, l, &ofs);
	if (ofs < l || !n->terminal()) {
		//ノードが見つからなかった.
		return;
	} else {
		n->remove_self(alloc_);
	}
}
//b, lを含むtrie木の「パス」の最後のノードを見つける.その場合にパスを通る間に何バイトが取り除かれたかを*ofsに返す
Patricia::Node *Patricia::find_node(const u8 *b, int l, int *ofs) {
	Node *cur = root_;
	*ofs = 0;
	while (*ofs < l) {
		int n_read;
		Node *next = cur->find_child(*matcher_, b + *ofs, l - *ofs, &n_read);
		if (next == nullptr) {
			break;
		}
		cur = next;
		*ofs += n_read;
	}
	return cur;
}
//見つかったノードが子供を持たない or 見つかったノードの時点でlが全て被覆されている場合.
bool Patricia::contains(const u8 *b, int l, int *ofs) {
	Node *n = find_node(b, l, ofs);
	return n->terminal();
}
bool Patricia::traverse(Node *cur, int depth, std::function<bool(Node*, int)> iter) {
	if (!iter(cur, depth)) { return false; }
	for (Node *n : cur->children_) {
		if (!traverse(n, depth + 1, iter)) { return false; }
	}
	return true;
}
#if defined(DEBUG)
void Patricia::Node::dump() const { 
	char buffer[len_ + 1];
	std::memcpy(buffer, bytes_, len_);
	buffer[len_] = 0;
	std::printf("node:%p[%s]%s:%d:%lu\n", this, buffer, terminal() ? "t" : "-", len_, children_.size());
}
void Patricia::dump() {
	std::printf("dump patricia tree at %p\n", this);
	std::printf("[*root*]\n");
	traverse([] (shutup::Patricia::NodeData *n, int depth) -> bool {
		char buffer[256];
		std::memcpy(buffer, n->bytes(), n->len());
		buffer[n->len()] = 0;
		for (int i = 0; i < depth; i++) { std::putc('\t', stdout); }
		std::printf("[%s]%s\n", buffer, n->terminal() ? "t" : "");
		return true;
	});
}
#else
void Patricia::dump() {}
#endif
} //namespace shutup

#include "patricia.h"

#if defined(DEBUG)
#include <cstdio>
#endif

namespace shutup {
//Patricia::Node
void Patricia::Node::merge(allocator<Node*> &a, Node *n) {
	//create merged node
	Node *merged = Node::new_node(a, parent_, bytes_, len_ + n->len_);
	std::memcpy(merged->bytes_ + len_, n->bytes_, n->len_);
	//replace self with merged node in siblings
	NodeList &siblings = parent_->children_;
	for (auto iter = siblings.begin(); iter != siblings.end(); iter++) {
		if (*iter == this) {
			siblings.erase(iter);
			break;
		}
	}
	siblings.push_back(merged);
	parent_->sort_children();
}
void Patricia::Node::destroy(allocator<Node*> &a) {
	for (Node *child : children_) {
		child->destroy(a);
	}
	free(a);
}
void Patricia::Node::add_child(allocator<Node*> &a, IMatcher &m, const u8 *b, int l) {
	for (auto iter = children_.begin(); iter != children_.end(); iter++) {
		Node *child = *iter;
		int ofs, i = m.match(b, l, child->bytes_, child->len_, &ofs);
		if (i > 0) {
			//child->bytes_のi byteまでで部分一致し、その時bはofs byte読まれた.
			//一般的にi != ofsだが、短いものにマッチしているということは、短い方がより汎用的な表現であると言える.
			//したがって、短い方をこのノードのchildとして残すノードのデータとして採用する.child->bytes_, bをスプリットして,
			//child->bytes_[0:i] or b[0:ofs]のうち短い方をこのノードのchildとして残し、child->bytes_[i + 1:]とb[ofs + 1:]をその子供に追加する.
			//child->bytes_[i + 1:]を保持するノードは今のノードの子供を引き継ぐ.
			Node *r;
			if (i < ofs) {
				r = Node::new_node(a, this, child->bytes_, i);
			} else {
				r = Node::new_node(a, this, b, ofs);
			}
			//i == child->len_となる時がある。b, lが完全に含まれるようなエントリーがあった場合.
			//この場合、b1はこのノードでb, lが終了してしまっても良いという「終端記号」の役割をするため、普通に追加する.
			Node *b1 = Node::new_node(a, r, child->bytes_ + i, child->len_ - i);
			Node *b2 = Node::new_node(a, r, b + ofs, l - ofs);
			//このchildを取り除く.全てのリンクはまだ生きている.
			children_.erase(iter);
			//b1に今のchildのchild_を全部くっつける.
			for (Node *n : children_) {
				n->parent_ = b1; //parentをb1にとっかえる.
			}
			b1->children_ = child->children_; //moveしてくれるのでcopy発生しないはず.
			//tにb1, b2を追加する.
			r->children_.push_back(b1);
			r->children_.push_back(b2);
			r->sort_children();//辞書順に並べる.
			//tを新しく追加する.
			children_.push_back(r);
			sort_children();//辞書順に並べる.
			//childは解放する.
			child->free(a);
			return;
		}
	}
	//全く一致するchildがなかったので新しく追加する.
	if (!root()) {
		children_.push_back(Node::new_node(a, this, b, 0));
	}
	children_.push_back(Node::new_node(a, this, b, l));
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
		//残り長さが0の場合、終端ノード（長さ0のノード。子ノードを持つが、このノードでも終端できることを示す）があるかチェック.
		for (Node *child : children_) {
			if (child->len_ == 0) {
				*ofs = 0;
				return child;
			}
		}
		return nullptr;
	}
	for (Node *child : children_) {
		//完全に一致したノードがある時のみ.
		if (child->len_ > 0 && child->len_ == m.match(b, l, child->bytes_, child->len_, ofs)) {
			return child;
		}
	}
	return nullptr;
}
void Patricia::Node::remove_self(allocator<Node*> &a) {
	if (root()) {
		//rootノードは除去できない.
		return;
	}
	NodeList &siblings = parent_->children_;
	for (auto iter = siblings.begin(); iter != siblings.end(); iter++) {
		if (this == *iter) {
			siblings.erase(iter);
			break;
		}
	}
	//親がrootの場合はただ取り除かれるだけとなる(分岐していないから).
	if (siblings.size() == 1 && !parent_->root()) {
		//merge this child node with parent.
		parent_->merge(a, siblings[0]);
	}
}
void Patricia::Node::dump() const { 
	char buffer[len_ + 1];
	std::memcpy(buffer, bytes_, len_);
	buffer[len_] = 0;
	std::printf("node:%p[%s]%lu\n", this, buffer, children_.size());
}
//Patricia
Patricia::~Patricia() {
	root_->destroy(alloc_);
	root_ = nullptr;
}
void Patricia::add_slice(const u8 *b, int l) {
	int ofs;
	Node *n = find_node(b, l, &ofs);
	if (ofs < l) {
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
	//本来 *ofs < lでもいいのだが、"abc", "abcd"とaddされたりした場合に、abcの下に終端記号的なデータを含まないノードが存在しているため、
	//そのcheckを行うため*ofs == lになってももう一回find_childする.
	while (*ofs <= l) {
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
	return !n->root() && n->terminal();
}
bool Patricia::traverse(Node *cur, int depth, std::function<bool(Node*, int)> iter) {
	if (!iter(cur, depth)) { return false; }
	for (Node *n : cur->children_) {
		if (!traverse(n, depth + 1, iter)) { return false; }
	}
	return true;
}
#if defined(DEBUG)
void Patricia::dump() {
	std::printf("dump patricia tree at %p\n", this);
	std::printf("[*root*]\n");
	traverse([] (shutup::Patricia::NodeData *n, int depth) -> bool {
		char buffer[256];
		std::memcpy(buffer, n->bytes(), n->len());
		buffer[n->len()] = 0;
		for (int i = 0; i < depth; i++) { std::putc('\t', stdout); }
		std::printf("[%s]\n", buffer);
		return true;
	});
}
#else
void Patricia::dump() {}
#endif
} //namespace shutup

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
void Patricia::Node::add_child(allocator<Node*> &a, const u8 *b, int l) {
	for (auto iter = children_.begin(); iter != children_.end(); iter++) {
		Node *child = *iter;
		// == はないはず（あるならこのノードではなくその子供への追加になっているはず)
		if (child->bytes_[0] == b[0]) {
			for (int i = 1; i < child->len_; i++) {
				if (child->bytes_[i] == b[i]) {
					continue;
				}
				//i byteまでで部分一致した.
				//child->bytes_をスプリットして,
				//child->bytes_[0:i]をこのノードのchildとして残し、child->bytes_[i + 1:]とb[i + 1:]をその子供に追加する.
				//child->bytes_[i + 1:]は今のノードのchild_を引き継ぐ.
				//TODO: 元データをshared_ptrとして扱って複数ノードで共有し、ofsとlenをviewとして各ノードに持たせることでmallocの回数をb[i + 1:]のための１回に減らせるだろう.
				//ただし、ノードを削除する際にNodeをparentにさかのぼってunref的なことをする必要があるため削除のパフォーマンスは低下するかも
				//いずれにせよ相当数のノードがないと効果は実感できないだろうと考えられるためTODOとしている.
				Node *t = Node::new_node(a, this, b, i); //childとb, lの共通部分.
				Node *b1 = Node::new_node(a, t, child->bytes_ + i, child->len_ - i);
				Node *b2 = Node::new_node(a, t, b + i, l - i);
				//このchildを取り除く.全てのリンクはまだ生きている.
				children_.erase(iter);
				//b1に今のchildのchild_を全部くっつける.
				for (Node *n : children_) {
					n->parent_ = b1; //parentをb1にとっかえる.
				}
				b1->children_ = child->children_; //moveしてくれるのでcopy発生しないはず.
				//tにb1, b2を追加する.
				t->children_.push_back(b1);
				t->children_.push_back(b2);
				t->sort_children();//辞書順に並べる.
				//tを新しく追加する.
				children_.push_back(t);
				sort_children();//辞書順に並べる.
				//childは解放する.
				child->free(a);
				return;
			}
		}
	}
	//全く一致するchildがなかったので新しく追加する.
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
Patricia::Node *Patricia::Node::find_child(imatcher &m, const u8 *b, int l, int *ofs) {
	for (Node *child : children_) {
		if (m.match(b, l, child->bytes_, child->len_, ofs)) {
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
		n->add_child(alloc_, b + ofs, l - ofs);
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
//見つかったノードが子供を持たなければ、完全に含まれているということ.
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

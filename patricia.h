#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib> 
#include <vector>
#include <algorithm>
#include <functional>
#include "types.h"
#include "allocator.h"
#include "matcher.h"

namespace shutup {
class Patricia {
	class Node {
	public:
		typedef std::vector<Node*, allocator<Node*>> NodeList;
		Node *parent_;
		NodeList children_;
		int len_;
		u8 bytes_[0];
		inline Node(allocator<Node*> &a, Node *p, const u8 *b, int l) : parent_(p), children_(a) {
			std::memcpy(bytes_, b, l);
			len_ = l;
		}
		inline Node(allocator<Node*> &a) : parent_(nullptr), children_(a), len_(0) {} //for creating root node
		inline ~Node() {} 
		void merge(allocator<Node*> &a, Node *with);
		void destroy(allocator<Node*> &a);
		void add_child(allocator<Node*> &a, const u8 *b, int l);
		Node *find_child(imatcher &m, const u8 *b, int l, int *ofs);
		static bool compare(const Node *left, const Node *right);
		void sort_children();
		void remove_self(allocator<Node*> &a);
		//inlines
		inline void free(allocator<Node*> &a) { 
			operator delete(this, this);
			a.pool().free(this);
		}
		inline bool terminal() const { return children_.size() == 0; }
		inline bool root() const { return parent_ == nullptr; }
		static inline void *operator new(std::size_t, void *buf) { return buf; }
		static inline void operator delete(void *p, void *buf) {}
		static inline Node *new_node(allocator<Node*> &a, Node *parent, const u8 *b, int l) {
			return new(a.pool().malloc(sizeof(Node) + l)) Node(a, parent, b, l);
		}
		static inline Node *new_root(allocator<Node*> &a) {
			return new(a.pool().malloc(sizeof(Node))) Node(a);
		}
	};
public:
	imatcher *matcher_;
	allocator<Node*> alloc_;
	Node *root_;
	inline Patricia() : matcher_(new byte_matcher()), alloc_(new system_mempool()), root_(Node::new_root(alloc_)) {}
	inline Patricia(imatcher *mch) : matcher_(mch), alloc_(new system_mempool()), root_(Node::new_root(alloc_)) {}
	inline Patricia(imempool *m) : matcher_(new byte_matcher()), alloc_(m), root_(Node::new_root(alloc_)) {}
	inline Patricia(imatcher *mch, imempool *m) : matcher_(mch), alloc_(m), root_(Node::new_root(alloc_)) {}
	~Patricia();
	void add_slice(const u8 *b, int l);
	void remove_slice(const u8 *b, int l);
	bool contains(const u8 *b, int l, int *ofs);
	//inlines
	inline void add(const char *s) { add_slice(reinterpret_cast<const u8*>(s), std::strlen(s)); }
	inline void remove(const char *s) { remove_slice(reinterpret_cast<const u8*>(s), std::strlen(s)); }
	inline bool contains(const char *s, int *ofs) { return contains(reinterpret_cast<const u8*>(s), std::strlen(s), ofs); }
	void dump();
protected:
	Node *find_node(const u8 *b, int l, int *ofs);
	bool traverse(Node *n, int depth, std::function<bool(Node*, int)> iter);
#if defined(TEST)
public:
	class NodeData : public Node {
	public:
		int len() const { return len_; }
		const u8 *bytes() const { return bytes_; }
	};
	//test helpers
	inline bool traverse(std::function<bool(NodeData*, int)> iter) { 
		auto tmp = [&iter] (Node *n, int depth) -> bool { return iter(reinterpret_cast<NodeData*>(n), depth); };
		for (Node *n : root_->children_) {
			if (!traverse(n, 1, tmp)) { return false; }
		}
		return true;
	}
#endif
};
}

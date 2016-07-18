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
		void *param_;
		int len_;
		u8 bytes_[0];
		inline Node(allocator<Node*> &a, Node *p, const u8 *b, int l, void *param) : parent_(p), children_(a), param_(param) {
			std::memcpy(bytes_, b, l);
			len_ = l;
		}
		inline Node(allocator<Node*> &a) : parent_(nullptr), children_(a), param_(nullptr), len_(0) {} //for creating root node
		inline ~Node() {} 
		void merge(allocator<Node*> &a, Node *with);
		void destroy(allocator<Node*> &a);
		void add_child(allocator<Node*> &a, IMatcher &m, const u8 *b, int l, void *p);
		Node *find_child(IMatcher &m, const u8 *b, int l, int *ofs);
		static bool compare(const Node *left, const Node *right);
		void sort_children();
		void remove_self(allocator<Node*> &a);
		void remove_from_siblings();
		void dump() const;
		//inlines
		inline void free(allocator<Node*> &a) { 
			operator delete(this, this);
			a.pool().free(this);
		}
		inline bool terminal() const { return param_ != nullptr; }
		inline bool root() const { return parent_ == nullptr; }
		static inline void *operator new(std::size_t, void *buf) { return buf; }
		static inline void operator delete(void *p, void *buf) {}
		static inline Node *new_node(allocator<Node*> &a, Node *parent, const u8 *b, int l, void *param) {
			return new(a.pool().malloc(sizeof(Node) + l)) Node(a, parent, b, l, param);
		}
		static inline Node *new_root(allocator<Node*> &a) {
			return new(a.pool().malloc(sizeof(Node))) Node(a);
		}
	};
public:
	static void *DEFAULT_PARAM_PTR;
	IMatcher *matcher_;
	allocator<Node*> alloc_;
	Node *root_;
	inline Patricia(IMatcher *mch = nullptr, IMempool *m = nullptr) : 
		matcher_(mch == nullptr ? new ByteMatcher() : mch), alloc_(m), root_(Node::new_root(alloc_)) {}
	~Patricia();
	void add_slice(const u8 *b, int l, void *p = nullptr);
	void remove_slice(const u8 *b, int l);
	void *get(const u8 *b, int l, int *ofs);
	//inlines
	inline void add(const char *s, void *p = nullptr) { add_slice(reinterpret_cast<const u8*>(s), std::strlen(s), p); }
	inline void remove(const char *s) { remove_slice(reinterpret_cast<const u8*>(s), std::strlen(s)); }
	inline bool contains(const char *s, int *ofs) { return get(reinterpret_cast<const u8*>(s), std::strlen(s), ofs) != nullptr; }
	inline void *get(const char *s, int *ofs) { return get(reinterpret_cast<const u8*>(s), std::strlen(s), ofs); }
	void dump() const;
protected:
	Node *find_node(const u8 *b, int l, int *ofs);
	bool traverse(Node *n, int depth, std::function<bool(Node*, int)> iter) const;
#if defined(TEST) || defined(DEBUG)
public:
	class NodeData : protected Node {
	public:
		int len() const { return len_; }
		const u8 *bytes() const { return bytes_; }
		bool terminal() const { return param_ != nullptr; }
		void *param() { return param_; }
	};
	//test helpers
	inline bool traverse(std::function<bool(NodeData*, int)> iter) const { 
		auto tmp = [&iter] (Node *n, int depth) -> bool { return iter(reinterpret_cast<NodeData*>(n), depth); };
		for (Node *n : root_->children_) {
			if (!traverse(n, 1, tmp)) { return false; }
		}
		return true;
	}
#endif
};
}

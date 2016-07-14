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
		Node *parent_;
		std::vector<Node*, allocator<Node*>> children_;
		int len_;
		u8 bytes_[0];
		Node(allocator<Node*> &a, Node *p, const u8 *b, int l) : parent_(p), children_(a) {
			std::memcpy(bytes_, b, l);
			len_ = l;
		}
		~Node() {} 
		void destroy(allocator<Node*> &a);
		void add_child(allocator<Node*> &a, const u8 *b, int l);
		Node *find_child(imatcher &m, const u8 *b, int l, int *ofs);
		static bool compare(const Node *left, const Node *right);
		void sort_children();
		//inlines
		inline void free(allocator<Node*> &a) { 
			operator delete(this, this);
			a.pool().free(this);
		}
		inline bool terminal() {
			return children_.size() == 0;
		}
		static inline void *operator new(std::size_t, void *buf) { return buf; }
		static inline void operator delete(void *p, void *buf) {}
	};
public:
	Node *root_;
	imatcher *matcher_;
	allocator<Node*> alloc_;
	Patricia() : root_(nullptr), matcher_(new byte_matcher()), alloc_(new system_mempool()) {}
	Patricia(imatcher *mch) : root_(nullptr), matcher_(mch), alloc_(new system_mempool()) {}
	Patricia(imempool *m) : root_(nullptr), matcher_(new byte_matcher()), alloc_(m) {}
	Patricia(imatcher *mch, imempool *m) : root_(nullptr), matcher_(mch), alloc_(m) {}
	~Patricia();
	void add_slice(const u8 *b, int l);
	void remove_slice(const u8 *b, int l) {}
	bool contains(const u8 *b, int l, int *ofs);
	//inlines
	inline void add(const char *s) { add_slice(reinterpret_cast<const u8*>(s), std::strlen(s)); }
	inline void remove(const char *s) { remove_slice(reinterpret_cast<const u8*>(s), std::strlen(s)); }
	inline bool contains(const char *s, int *ofs) { return contains(reinterpret_cast<const u8*>(s), std::strlen(s), ofs); }
protected:
	Node *find_node(const u8 *b, int l, int *ofs);
	bool traverse(Node *n, std::function<bool(Node*)> iter);
#if defined(TEST)
public:
	class ExportNode : public Node {
	public:
		int len() const { return len_; }
		const u8 *bytes() const { return bytes_; }
	};
	//test helpers
	inline bool traverse(std::function<bool(ExportNode*)> iter) { 
		return traverse(root_, [&iter](Node *n) -> bool { return iter(reinterpret_cast<ExportNode*>(n)); }); 
	}
#endif
};
}

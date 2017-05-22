#pragma once
#include <functional>
#include <string>
#include <vector>
#include <map>
#include "matcher.h"
#include "allocator.h"
//declare per language setting of "improper word"
namespace shutup {
class Checker;
namespace language {
class WordChecker : public IMatcher {
public:
	typedef std::function<int(const u8 *in, int ilen, u8 *out, int *olen)> normalizer;
    typedef std::basic_string<char, std::char_traits<char>, allocator<char>> str;
	typedef allocator<str> stralloc;
	typedef allocator<normalizer> normalloc;
	typedef std::vector<str, stralloc> strvec;
	typedef std::vector<normalizer, normalloc> normvec;
	typedef allocator<std::pair<str, strvec>> pairalloc;
	typedef std::map<str, strvec, std::less<str>, pairalloc> svmap;
protected:
	stralloc pstr_alloc_;
	pairalloc pair_alloc_;
	normalloc norm_alloc_;
	svmap aliases_map_;
	normvec normalizers_;
    strvec empty_list_;
	char *ignore_glyphs_;
public:
	WordChecker(IMempool *p) : pstr_alloc_(p), pair_alloc_(p), norm_alloc_(p),
		aliases_map_(pair_alloc_), normalizers_(norm_alloc_), empty_list_(pstr_alloc_),
        ignore_glyphs_(nullptr) {
		normalizers_.push_back(std::bind(&WordChecker::remove_ignored, this, 
			std::placeholders::_1, std::placeholders::_2, 
			std::placeholders::_3, std::placeholders::_4));
	}
	virtual ~WordChecker() {}
	//from IMatcher
	virtual int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs);
	//from WordChecker
	virtual int init();
	virtual void fin();
	virtual int normalize(const u8 *in, int ilen, u8 *out, int olen);
	virtual void add_synonym(const char *pattern, Checker &c, void *ctx);
	//helpers
	void ignore_glyphs(const char *glyphs, bool reset = false);
	bool ignored(const char *g);
	void add_alias(const char *target, const char *alias);
	void set_alias(const char *pattern, strvec &vec);
	void link_alias(const char *pattern1, const char *pattern2);
	const strvec &alias_list(const char *key) const;
	inline stralloc &pool() const { return (stralloc &)pstr_alloc_; }
	inline svmap &aliases_map() { return aliases_map_; }
protected:
	//internals
	int remove_ignored(const u8 *in, int ilen, u8 *out, int *olen);
	int read_next_with_normalize(const u8 *in, int ilen, u8 *out, int *olen);
public:
	static inline void *operator new(std::size_t, void *buf) { return buf; }
	static inline void operator delete(void *p, void *buf) {}
	static inline void operator delete( void *p ) {}  
};
}
}

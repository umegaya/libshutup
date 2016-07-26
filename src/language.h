#pragma once
#include <functional>
#include <string>
#include "matcher.h"
#include "allocator.h"
#include "checker.h"
//declare per language setting of "improper word"
namespace shutup {
namespace language {
class WordChecker : public IWordChecker {
public:
	typedef std::function<int(const u8 *in, int ilen, u8 *out, int *olen)> normalizer;
	typedef std::basic_string<char, std::char_traits<char>, allocator<char>> str;
	typedef allocator<str> stralloc;
	typedef std::vector<str, stralloc> strvec;
	typedef allocator<std::pair<str, strvec>> pairalloc;
	typedef std::map<str, strvec, std::less<str>, pairalloc> svmap;
protected:
	stralloc pstr_alloc_;
	pairalloc pair_alloc_;
	svmap aliases_;
	normalizer remove_ignored_;
	char *ignore_glyphs_;
public:
	WordChecker(IMempool *p) : pstr_alloc_(p), pair_alloc_(p), aliases_(pair_alloc_), ignore_glyphs_(nullptr) {
		remove_ignored_ = std::bind(&WordChecker::remove_ignored, this, 
			std::placeholders::_1, std::placeholders::_2, 
			std::placeholders::_3, std::placeholders::_4);
	}
	virtual ~WordChecker() {}
	//from IMatcher
	virtual int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs);
	//from IWordChecker
	virtual int init();
	virtual int normalize(const u8 *in, int ilen, u8 *out, int olen);
	virtual void add_synonym(const char *pattern, Checker &c);
	virtual void add_alias(const char *target, const char *alias);
	virtual void ignore_glyphs(const char *glyphs);
	//WordChecker own
	virtual bool ignored(const char *g);
	virtual normalizer *normalizers(int *n_norm);
	//helpers
	void set_alias(const char *pattern, strvec &vec);
	void link_alias(const char *pattern1, const char *pattern2);
	void add_ignore_glyphs(const char *glyphs, bool reset = false);
	int remove_ignored(const u8 *in, int ilen, u8 *out, int *olen);
	int read_next_with_normalize(const u8 *in, int ilen, u8 *out, int *olen);
	const strvec &alias_list(const char *key) const;
	inline void set_ignore_glyphs(const char *glyphs) { add_ignore_glyphs(glyphs, true); }
	inline IMempool &pool() { return pstr_alloc_.pool(); }
public:
	static inline void *operator new(std::size_t, void *buf) { return buf; }
	static inline void operator delete(void *p, void *buf) {}
	static inline void operator delete( void *p ) {}  
#if defined(DEBUG) || defined(TEST)
	inline svmap &aliases() { return aliases_; }
#endif
};
}
}

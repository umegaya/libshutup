#pragma once
#include "matcher.h"
#include "allocator.h"
#include "checker.h"
//declare per language setting of "improper word"
namespace shutup {
namespace language {
class WordChecker : public IWordChecker {
public:
	typedef int (*normalizer)(const u8 *in, int ilen, u8 *out, int *olen);
	typedef allocator<const char *> stralloc;
	typedef std::vector<const char *, stralloc> strvec;
	typedef allocator<std::pair<const char*, strvec>> pairalloc;
	typedef tpl::psmap<strvec, pairalloc> svmap;
protected:
	stralloc pstr_alloc_;
	pairalloc pair_alloc_;
	svmap aliases_;
	char *ignore_gryphs_;
public:
	WordChecker(IMempool *p) : pstr_alloc_(p), pair_alloc_(p), aliases_(pair_alloc_) {}
	virtual ~WordChecker() {}
	//from IMatcher
	virtual int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs);
	//from IWordChecker
	virtual int init();
	virtual int normalize(const u8 *in, int ilen, u8 *out, int olen);
	virtual void add_synonym(const char *pattern, Checker &c);
	//WordChecker own
	virtual bool ignored(const char *g);
	virtual normalizer *normalizers(int *n_norm);
	//helpers
	void set_alias(const char *pattern, strvec &vec);
	void link_alias(const char *pattern1, const char *pattern2);
	void add_ignore_gryphs(const char *gryphs, bool reset = false);
	inline void set_ignore_gryphs(const char *gryphs) { add_ignore_gryphs(gryphs, true); }
	inline IMempool &pool() { return pstr_alloc_.pool(); }
public:
	static inline void *operator new(std::size_t, void *buf) { return buf; }
	static inline void operator delete(void *p, void *buf) {}
	static inline void operator delete( void *p ) {}  
};
}
}

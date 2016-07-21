#pragma once

#include "lib.h"
#include "patricia.h"

namespace shutup {
class IWordChecker : public IMatcher {
public:
	virtual int init() = 0;
	virtual int normalize(const u8 *in, int ilen, u8 *out, int olen) = 0;
	virtual void add_synonym(const char *pattern, class Checker &c) = 0;
	virtual bool ignored(const char *gryph) = 0;
};

class Checker {
public:
	static const int MAX_FILTER_STRING = 1024 * 1024;//1M
	class Mempool : public IMempool {
		shutup_allocator alloc_;
	public:
		Mempool(shutup_allocator *a) {
			shutup_allocator def = {
				std::malloc,
				std::free,
				std::realloc,
			};
			alloc_ = ((a != nullptr) ? *a : def);
		}
		void *malloc(size_t sz) { return alloc_.malloc(sz); }
		void free(void *p) { alloc_.free(p); }
		void *realloc(void *p, size_t sz) { return alloc_.realloc(p, sz); }
	};
protected:
	Mempool pool_;
	IWordChecker *checker_;
	Patricia trie_;
	int masking(const u8 *in, int ilen, u8 *out, int olen, const char *mask, int mlen);
public:
	inline Checker(const char *lang, shutup_allocator *a) : 
		pool_(a), checker_(by(lang, pool_)), trie_(checker_, &pool_) {}
	inline ~Checker() {}
	void add(const char *s);
	inline void remove(const char *s) { trie_.remove(s); }
	const char *filter(const char *in, char *out, int *olen, const char *mask = "?");
	bool should_filter(const char *s, char *out, int *olen);
public:
	IWordChecker *by(const char *lang, Mempool &m);
};
}

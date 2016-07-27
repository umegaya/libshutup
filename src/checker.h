#pragma once

#include "lib.h"
#include "patricia.h"

namespace shutup {
class IWordChecker : public IMatcher {
public:
	virtual int init() = 0;
	virtual int normalize(const u8 *in, int ilen, u8 *out, int olen) = 0;
	virtual void add_synonym(const char *pattern, class Checker &c) = 0;
	virtual void add_alias(const char *target, const char *alias) = 0;
	virtual void ignore_glyphs(const char *glyphs) = 0;
	virtual bool ignored(const char *glyph) = 0;
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
	inline bool valid() const { return checker_ != nullptr; }
	void add(const char *s);
	inline void add_word(const char *s) { TRACE("add_word: %s\n", s); trie_.add(s); }
	inline void add_alias(const char *target, const char *alias) { checker_->add_alias(target, alias); }
	inline void ignore_glyphs(const char *glyphs) { checker_->ignore_glyphs(glyphs); }
	inline void remove(const char *s) { trie_.remove(s); }
	const char *filter(const char *in, int ilen, char *out, int *olen, const char *mask = nullptr);
	bool should_filter(const char *in, int ilen, char *out, int *olen);
public:
	static IWordChecker *by(const char *lang, Mempool &m);
};
}

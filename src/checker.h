#pragma once

#include "shutup.h"
#include "patricia.h"

namespace shutup {
namespace language {
class WordChecker;
}
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
	language::WordChecker *checker_;
	Patricia trie_;
	int masking(const u8 *in, int ilen, u8 *out, int olen, const char *mask, int mlen);
public:
	Checker(const char *lang, shutup_allocator *a);
	Checker(shutup_allocator *a, std::function<language::WordChecker*(Mempool&)> factory);
	~Checker();
	inline bool valid() const { return checker_ != nullptr; }
	void add(const char *s, void *ctx);
	void add_alias(const char *target, const char *alias);
	void ignore_glyphs(const char *glyphs);
	inline void add_word(const char *s, void *ctx) { trie_.add(s, ctx); }
	inline void remove(const char *s) { trie_.remove(s); }
	const char *filter(const char *in, int ilen, char *out, int *olen, const char *mask = nullptr);
	bool should_filter(const char *in, int ilen, char *out, int *olen, void **pctx = nullptr);
public:
	static language::WordChecker *by(const char *lang, Mempool &m);
	template <class C> static C *create(Mempool &m) {
		C *c = new(m.malloc(sizeof(C))) C(m);
		c->init();
		return c;
	}
};
}

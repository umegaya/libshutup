#pragma once

#include "shutup.h"
#include "patricia.h"

namespace shutup {
namespace language {
class WordChecker;
}
class Checker {
public:
	typedef shutup_allocator Allocator;
	typedef std::function<bool(const char *,int,int,int,void *)> ContextChecker; 
	static const int MAX_FILTER_STRING = 1024 * 1024;//1M
	class Mempool : public IMempool {
		Allocator alloc_;
	public:
		Mempool(Allocator *a) {
			Allocator def = {
				std::malloc,
				std::free,
				std::realloc,
			};
			alloc_ = ((a != nullptr) ? *a : def);
		}
		void *malloc(size_t sz) { return alloc_.malloc(sz); }
		void free(void *p) { alloc_.free(p); }
		void *realloc(void *p, size_t sz) { return alloc_.realloc(p, sz); }
		Allocator &allocator() { return alloc_; }
	};
protected:
	Mempool pool_;
	language::WordChecker *checker_;
	Patricia trie_;
	int masking(const u8 *in, int ilen, u8 *out, int olen, const char *mask, int mlen);
public:
	Checker(const char *lang, Allocator *a);
	Checker(Allocator *a, std::function<language::WordChecker*(Mempool&)> factory);
	~Checker();
	inline bool valid() const { return checker_ != nullptr; }
	void add(const char *s, void *ctx = nullptr);
	void add_alias(const char *target, const char *alias);
	void ignore_glyphs(const char *glyphs);
	inline void add_word(const char *s, void *ctx) { trie_.add(s, ctx); }
	inline void remove(const char *s) { trie_.remove(s); }
    static bool truer(const char *in, int ilen, int start, int count, void *ctx);
	const char *filter(const char *in, int ilen, char *out, int *olen, const char *mask = nullptr, ContextChecker checker = truer);
	bool should_filter(const char *in, int ilen, int *start, int *count, void **pctx = nullptr, ContextChecker checker = truer);
public:
	static language::WordChecker *by(const char *lang, Mempool &m);
	template <class C> static C *new_word_checker(Mempool &m) {
		C *c = new(m.malloc(sizeof(C))) C(&m);
		c->init();
		return c;
	}
	template <class C> static Checker *create(Allocator *a) {
		return new(a->malloc(sizeof(Checker))) Checker(a, new_word_checker<C>); 
	}
	static Checker *create(const char *lang, Allocator *a) {
		return new(a->malloc(sizeof(Checker))) Checker(lang, a); 
	}
	static void destroy(Checker *c) {
		auto f = c->pool_.allocator().free;
		c->~Checker();
		f(c);
	}
	static inline void *operator new(std::size_t sz) { return std::malloc(sz); }
	static inline void *operator new(std::size_t, void *buf) { return buf; }
	static inline void operator delete(void *p, void *buf) {}
	static inline void operator delete( void *p ) { std::free(p); }
#if defined(DEBUG)
    void dump() { trie_.dump(); }
#endif
};
}

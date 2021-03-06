#include "checker.h"
#include "language/jp.h"

namespace shutup {
Checker::Checker(const char *lang, Allocator *a) : 
	pool_(a), checker_(by(lang, pool_)), trie_(checker_, &pool_) {}
Checker::Checker(Allocator *a, std::function<language::WordChecker*(Mempool&)> factory) : 
	pool_(a), checker_(factory(pool_)), trie_(checker_, &pool_) {}
Checker::~Checker() { 
	if (checker_ != nullptr) {
		checker_->fin();
		delete checker_; //to call destrcutor. operator delete overridden not to free memory
		pool_.free(checker_); //freeing memory actually done in here.
		checker_ = nullptr;
	}
}
void Checker::add(const char *s, void *ctx) { 
	int sz = (int)strnlen(s, MAX_FILTER_STRING);
	if (sz <= 0) { return; }
	u8 buf[sz * utf8::MAX_BYTE_PER_GRYPH];
	int rlen = checker_->normalize(reinterpret_cast<const u8*>(s), sz, buf, sz * utf8::MAX_BYTE_PER_GRYPH);
	buf[rlen] = 0;
	checker_->add_synonym(reinterpret_cast<const char *>(buf), *this, ctx);
	add_word(reinterpret_cast<const char *>(buf), ctx);
}
void Checker::add_alias(const char *target, const char *alias) { 
	checker_->add_alias(target, alias); 
}
void Checker::ignore_glyphs(const char *glyphs) { 
	checker_->ignore_glyphs(glyphs); 
}
bool Checker::truer(const char *in, int ilen, int start, int count, void *ctx) { return true; }
int Checker::masking(const u8 *in, int ilen, u8 *out, int olen, const char *mask, int mlen) {
	int iofs = 0, oofs = 0;
	while (ilen > iofs && olen > oofs) {
		int tmp = utf8::peek(in + iofs, ilen - iofs);
		if (tmp == 0) {
			break;
		}
		char glyph[tmp + 1];
		std::memcpy(glyph, in + iofs, tmp); glyph[tmp] = 0;
		//TRACE("masking: glyph: %s\n", glyph);
		if (!checker_->ignored(glyph)) {
			std::memcpy(out + oofs, mask, mlen);
			oofs += mlen;
			iofs += tmp;
		} else {
			std::memcpy(out + oofs, glyph, tmp);
			oofs += tmp;
			iofs += tmp;
		}
	}
	out[oofs] = 0;
	return oofs;
}
const char *Checker::filter(const char *in, int ilen, char *out, int *olen, const char *mask, ContextChecker checker) {
	if (mask == nullptr) { mask = "?"; }
	int iofs = 0;
	int msz = (int)strnlen(mask, MAX_FILTER_STRING);
	int oofs = 0, tmp;
	void *ctx;
	const u8 *iptr = reinterpret_cast<const u8 *>(in);
	u8 *optr = reinterpret_cast<u8 *>(out);
	while (iofs < ilen) {
		if ((ctx = trie_.get(iptr + iofs, ilen - iofs, &tmp)) != nullptr && checker(in, ilen, iofs, tmp, ctx)) {
			oofs += masking(iptr + iofs, tmp, optr + oofs, *olen - oofs, mask, msz);
			iofs += tmp;
		} else {
			tmp = utf8::copy(iptr + iofs, ilen - iofs, optr + oofs, *olen - oofs, 1);
			if (tmp < 0) {
				return nullptr; //buf short
			}
			oofs += tmp;
			iofs += tmp;
		}
	}
	*olen = oofs;
	out[*olen] = 0;
	return out;
}
bool Checker::should_filter(const char *in, int ilen, int *start, int *count, void **pctx, ContextChecker checker) {
	int iofs = 0, tmp;
	void *ctx;
	const u8 *iptr = reinterpret_cast<const u8 *>(in);
	while (iofs < ilen) {
		if ((ctx = trie_.get(iptr + iofs, ilen - iofs, count)) != nullptr && checker(in, ilen, iofs, *count, ctx)) {
			*start = iofs;
			if (pctx != nullptr) { *pctx = ctx; }
			return true;
		} else {
			tmp = utf8::peek(iptr + iofs, ilen - iofs);
			iofs += tmp;
		}
	}
	return false;
}

language::WordChecker *Checker::by(const char *lang, Mempool &p) {
	language::WordChecker *w;
	if (std::memcmp(lang, "jp", 2) == 0) {
		w = new(p.malloc(sizeof(language::JP))) language::JP(&p);
	} else {
		TRACE("invalid language %s\n", lang);
		return nullptr;
	}
	w->init();
	return w;
}
}

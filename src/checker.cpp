#include "checker.h"
#include "language/jp.h"

namespace shutup {

void Checker::add(const char *s) { 
	int sz = strnlen(s, MAX_FILTER_STRING);
	u8 buf[sz * utf8::MAX_BYTE_PER_GRYPH];
	int rlen = checker_->normalize(reinterpret_cast<const u8*>(s), sz, buf, sz * utf8::MAX_BYTE_PER_GRYPH);
	buf[rlen] = 0;
	checker_->add_synonym(reinterpret_cast<const char *>(buf), *this);
}
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
const char *Checker::filter(const char *in, int ilen, char *out, int *olen, const char *mask) {
	if (mask == nullptr) { mask = "?"; }
	int iofs = 0;
	int msz = strnlen(mask, MAX_FILTER_STRING);
	int oofs = 0, tmp;
	const u8 *iptr = reinterpret_cast<const u8 *>(in);
	u8 *optr = reinterpret_cast<u8 *>(out);
	while (iofs < ilen) {
		if (trie_.get(iptr + iofs, ilen - iofs, &tmp) != nullptr) {
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
bool Checker::should_filter(const char *in, int ilen, char *out, int *olen) {
	int iofs = 0, tmp;
	const u8 *iptr = reinterpret_cast<const u8 *>(in);
	while (iofs < ilen) {
		if (trie_.get(iptr + iofs, ilen - iofs, &tmp) != nullptr) {
			int n_copy = std::min(tmp, *olen - 1);
			std::strncpy(out, in + iofs, n_copy);
			*olen = n_copy;
			return true;
		} else {
			tmp = utf8::peek(iptr + iofs, ilen - iofs);
			iofs += tmp;
		}
	}
	return false;
}

IWordChecker *Checker::by(const char *lang, Mempool &p) {
	IWordChecker *w;
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

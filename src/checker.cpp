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
		char gryph[tmp + 1];
		std::memcpy(gryph, in + iofs, tmp); gryph[tmp] = 0;
		//TRACE("masking: gryph: %s\n", gryph);
		if (!checker_->ignored(gryph)) {
			std::memcpy(out + oofs, mask, mlen);
			oofs += mlen;
			iofs += tmp;
		} else {
			std::memcpy(out + oofs, gryph, tmp);
			oofs += tmp;
			iofs += tmp;
		}
	}
	out[oofs] = 0;
	return oofs;
}
const char *Checker::filter(const char *in, char *out, int *olen, const char *mask) {
	int rlen = strnlen(in, MAX_FILTER_STRING), rofs = 0;
	int msz = strnlen(mask, MAX_FILTER_STRING);
	int wofs = 0, tmp;
	const u8 *iptr = reinterpret_cast<const u8 *>(in);
	u8 *optr = reinterpret_cast<u8 *>(out);
	while (rofs < rlen) {
		if (trie_.get(iptr + rofs, rlen - rofs, &tmp) != nullptr) {
			wofs += masking(iptr + rofs, tmp, optr + wofs, *olen - wofs, mask, msz);
			rofs += tmp;
		} else {
			tmp = utf8::copy(iptr + rofs, rlen - rofs, optr + wofs, *olen - wofs, 1);
			if (tmp < 0) {
				return nullptr; //buf short
			}
			wofs += tmp;
			rofs += tmp;
		}
	}
	*olen = wofs;
	out[*olen] = 0;
	return out;
}
bool Checker::should_filter(const char *in, char *out, int *olen) {
	//TRACE("should_filter: :%s\n", in);
	int rlen = strnlen(in, MAX_FILTER_STRING), rofs = 0;
	int tmp;
	const u8 *iptr = reinterpret_cast<const u8 *>(in);
	while (rofs < rlen) {
		if (trie_.get(iptr + rofs, rlen - rofs, &tmp) != nullptr) {
			int n_copy = std::min(tmp, *olen - 1);
			std::strncpy(out, in + rofs, n_copy);
			*olen = n_copy;
			return true;
		} else {
			tmp = utf8::peek(iptr + rofs, rlen - rofs);
			rofs += tmp;
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

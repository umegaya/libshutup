#include "checker.h"
#include "language/jp.h"

namespace shutup {

static int UTF8_MAX_BYTE_PER_GRYPH = 6;
void Checker::add(const char *s) { 
	int sz = strnlen(s, MAX_FILTER_STRING);
	u8 buf[sz * UTF8_MAX_BYTE_PER_GRYPH];
	int rlen = checker_->normalize(reinterpret_cast<const u8*>(s), sz, buf, sz * UTF8_MAX_BYTE_PER_GRYPH);
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
		if (checker_->ignored(gryph)) {
			std::memcpy(out + oofs, mask, mlen);
			oofs += mlen;
			iofs += mlen;
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
	int sz = strnlen(in, MAX_FILTER_STRING);
	int msz = strnlen(mask, MAX_FILTER_STRING);
	u8 buf[*olen];
	int rofs = 0, rlen = checker_->normalize(reinterpret_cast<const u8*>(in), sz, buf, *olen);
	if (rlen < 0) {
		//overflow?
		return nullptr;
	}
	int wofs = 0, tmp;
	u8 *optr = reinterpret_cast<u8 *>(out);
	while (rofs < rlen) {
		if (trie_.get(buf + rofs, rlen - rofs, &tmp) != nullptr) {
			wofs += masking(buf + rofs, rlen - rofs, optr + wofs, *olen - wofs, mask, msz);
			rofs += tmp;
		} else {
			tmp = utf8::copy(buf + rofs, rlen - rofs, optr + wofs, *olen - wofs, 1);
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
	int sz = strnlen(in, MAX_FILTER_STRING);
	u8 buf[sz * UTF8_MAX_BYTE_PER_GRYPH];
	int rofs = 0, rlen = checker_->normalize(reinterpret_cast<const u8*>(in), sz, buf, sz * 6);
	if (rlen < 0) {
		//overflow?
		return true;
	}
	int tmp;
	while (rofs < rlen) {
		if (trie_.get(buf + rofs, rlen - rofs, &tmp) != nullptr) {
			int n_copy = std::min(tmp, *olen - 1);
			std::strncpy(out, reinterpret_cast<char *>(buf + rofs), n_copy);
			*olen = n_copy;
			return true;
		} else {
			tmp = utf8::peek(buf + rofs, rlen - rofs);
			rofs += tmp;
		}
	}
	return false;
}

IWordChecker *Checker::by(const char *lang, Mempool &p) {
	if (std::memcmp(lang, "jp", 2) == 0) {
		return new(p.malloc(sizeof(language::JP))) language::JP(&p);
	} else {
		TRACE("invalid language %s\n", lang);
		return nullptr;
	}
}
}

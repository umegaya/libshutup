#include "checker.h"
#include "language.h"

#if defined(DEBUG) || defined(TEST)
#include <cstdio>
#endif

namespace shutup {
namespace language {
//aliasやignore stringの登録などを行う.
int WordChecker::init() {
	return 0;
}
//独自のメモリ割当の後処理を行う.
void WordChecker::fin() {
	if (ignore_glyphs_ != nullptr) { 
		pool().free(ignore_glyphs_); 
	} 
}
//aliasは文字単位での組み合わせを全てチェックしてしまうので、そこまでチェックしたくない場合、
//ここで単語単位で同じ意味のものを登録する.
void WordChecker::add_synonym(const char *pattern, Checker &c) {
	c.add_word(pattern);
}
//文字列のマッチを行う.
int WordChecker::match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) {
#if defined(TEST) || defined(DEBUG)
	char buf1[256], buf2[256];
	std::memcpy(buf1, in, ilen); buf1[ilen] = 0;
	std::memcpy(buf2, pattern, plen); buf2[plen] = 0;
	//TRACE("match: %s[%d] %s[%d]\n", buf1, ilen, buf2, plen);
#endif
	int n_read = 0, n_pread = 0;
	u8 out[utf8::MAX_BYTE_PER_GRYPH];
	u8 pout[utf8::MAX_BYTE_PER_GRYPH];
NEXT:
	while (n_read < ilen && n_pread < plen) {
		int wtmp = utf8::MAX_BYTE_PER_GRYPH;
		int rtmp = read_next_with_normalize(in + n_read, ilen - n_read, out, &wtmp);
		//TRACE("read_next_with_normalize: %d %d\n", rtmp, wtmp);
		if (rtmp < 0) {
			return rtmp; //error
		} else if (rtmp == 0) {
			break; //finish
		} else if (wtmp == 0) {
			//glyph just skipped. 
			n_read += rtmp;
			goto NEXT;
		} else {
			out[wtmp] = 0;
			int prtmp = utf8::peek(pattern + n_pread, plen - n_pread);
			std::memcpy(pout, pattern + n_pread, prtmp);
			pout[prtmp] = 0;
			auto al = alias_list(reinterpret_cast<const char *>(pout));
			//TRACE("check with alias: pick %s %s %lu\n", out, pout, al.size());
			if (al.size() > 0) {
				for (auto &a : al) {
					if (wtmp == a.length() && std::memcmp(out, a.c_str(), wtmp) == 0) {
						n_read += rtmp;
						n_pread += prtmp;
						goto NEXT;
					}
				}
			} else if (wtmp == prtmp && std::memcmp(out, pout, wtmp) == 0) {
				n_read += rtmp;
				n_pread += prtmp;
				goto NEXT;
			}
			break;
		}
	}
	*ofs = n_read;
	return n_pread;
}
//次の一文字の正規化を行う.
int WordChecker::read_next_with_normalize(const u8 *in, int ilen, u8 *out, int *olen) {
	int tmp, otmp;
	for (auto &norm : normalizers_) {
		otmp = *olen;
		tmp = norm(in, ilen, out, &otmp);
		if (tmp != 0) { 
			*olen = otmp;
			return tmp; 
		}
	}
	//nothing convert current character. just copy one utf8 character.
	tmp = utf8::copy(in, ilen, out, *olen, 1);
	if (tmp < 0) {
		return tmp;
	}
	*olen = tmp;
	return tmp;
}
//文字種を限定するために誤検出の心配がないような文字種の変換を既に行っておく.
int WordChecker::normalize(const u8 *in, int ilen, u8 *out, int olen) {
	return util::convert(in, ilen, out, olen, std::bind(&WordChecker::read_next_with_normalize, this, 
			std::placeholders::_1, std::placeholders::_2, 
			std::placeholders::_3, std::placeholders::_4));
}
void WordChecker::set_alias(const char *pattern, strvec &vec) { 
	auto i = aliases_map_.find(pattern);
	if (i == aliases_map_.end()) {
		vec.push_back(pattern); 
		aliases_map_[pattern] = std::move(vec); 
	} else {
		strvec &v = (*i).second;
		std::copy(vec.begin(), vec.end(), std::back_inserter(v));
	}
}
void WordChecker::link_alias(const char *pattern1, const char *pattern2) {
	if (std::strcmp(pattern1, pattern2) != 0) {
		strvec v1(pstr_alloc_); v1.push_back(pattern1);
		strvec v2(pstr_alloc_); v2.push_back(pattern2);
		set_alias(pattern1, v2);
		set_alias(pattern2, v1);
	}
}
const WordChecker::strvec &WordChecker::alias_list(const char *key) const {
	auto i = aliases_map_.find(key);
	if (i == aliases_map_.end()) {
		static strvec empty_list;
		return empty_list;
	} else {
		return (*i).second;
	}
}
//エイリアスを追加する.
void WordChecker::add_alias(const char *target, const char *alias) {
	strvec v{alias};
	set_alias(target, v);
}
//無視する文字列を追加する
void WordChecker::ignore_glyphs(const char *glyphs, bool reset) { 
	if (reset && ignore_glyphs_ != nullptr) {
		pool().free(ignore_glyphs_);
		ignore_glyphs_ = nullptr; 
	}
	if (ignore_glyphs_ == nullptr) {
		size_t sz = std::strlen(glyphs);
		ignore_glyphs_ = reinterpret_cast<char *>(pool().malloc(sz + 1));
		std::strncpy(ignore_glyphs_, glyphs, sz + 1);
	} else {
		size_t sz = std::strlen(glyphs), osz = std::strlen(ignore_glyphs_);
		ignore_glyphs_ = reinterpret_cast<char *>(pool().realloc(ignore_glyphs_, osz + sz + 1));
		std::strncpy(ignore_glyphs_ + osz, glyphs, sz + 1);	
	}
}
//マッチングで無視される文字列かどうかを判定する.
bool WordChecker::ignored(const char *g) { 
	return std::strstr(ignore_glyphs_, g) != nullptr; 
}
int WordChecker::remove_ignored(const u8 *in, int ilen, u8 *out, int *olen) {
	u8 buff[utf8::MAX_BYTE_PER_GRYPH + 1];
	int r = utf8::copy(in, ilen, buff, sizeof(buff), 1);
	if (r <= 0) {
		*olen = 0;
		return 0;
	}
	buff[r] = 0;
	if (ignored(reinterpret_cast<const char *>(buff))) {
		*olen = 0;
		return r;
	} else {
		*olen = 0;
		return 0;
	}
}
}
}

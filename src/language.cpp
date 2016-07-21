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
//aliasは文字単位での組み合わせを全てチェックしてしまうので、そこまでチェックしたくない場合、
//ここで単語単位で同じ意味のものを登録する.
void WordChecker::add_synonym(const char *pattern, Checker &c) {
	c.add(pattern);
}
//normalizeで使うnormalizerを定義する.
WordChecker::normalizer *WordChecker::normalizers(int *n_norm) {
	*n_norm = 0;
	return nullptr;
}
//マッチングで無視される文字列かどうかを判定する.
bool WordChecker::ignored(const char *g) { 
	return std::strstr(ignore_gryphs_, g) != nullptr; 
}
//文字列のマッチを行う.
int WordChecker::match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) {
	int i = 0;
	while (i < std::min(ilen, plen)) {
		int tmp = utf8::peek(in + i, ilen - i);
		if (tmp <= 0 || plen < (i + tmp)) {
			break;
		}
		if (memcmp(in + i, pattern + i, tmp) != 0) {
			break;
		}
		i += tmp;
	}
	*ofs = i;
	return i;
}
//文字種を限定するために誤検出の心配がないような文字種の変換を既に行っておく.
int WordChecker::normalize(const u8 *in, int ilen, u8 *out, int olen) {
	int n_read = 0, n_write = 0, wtmp, rtmp, n_norm;
	normalizer *norms = normalizers(&n_norm);
NEXT:
	while (olen > n_write && ilen > n_read) {
		for (int i = 0; i < n_norm; i++) {
			auto norm = norms[i];
			wtmp = olen - n_write;
			rtmp = norm(in + n_read, ilen - n_read, out + wtmp, &wtmp);
			if (rtmp < 0) { 
				return rtmp; 
			} else if (rtmp > 0) {
				n_read += rtmp;
				n_write += wtmp;
				goto NEXT;
			}
		}
		//nothing convert current character. just copy one utf8 character.
		int tmp = utf8::copy(in + n_read, ilen - n_read, out + n_write, olen - n_write, 1);
		if (tmp < 0) {
			return tmp;
		}
		n_read += tmp;
		n_write += tmp;
	}
	return n_write;
}
void WordChecker::set_alias(const char *pattern, strvec &vec) { 
	auto i = aliases_.find(pattern);
	if (i == aliases_.end()) {
		vec.push_back(pattern); aliases_[pattern] = std::move(vec); 
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
void WordChecker::add_ignore_gryphs(const char *gryphs, bool reset) { 
	if (reset) {
		pool().free(ignore_gryphs_);
		ignore_gryphs_ = nullptr; 
	}
	if (ignore_gryphs_ == nullptr) {
		size_t sz = std::strlen(gryphs);
		ignore_gryphs_ = reinterpret_cast<char *>(pool().malloc(sz + 1));
		std::strncpy(ignore_gryphs_, gryphs, sz);
	} else {
		size_t sz = std::strlen(gryphs), osz = std::strlen(ignore_gryphs_);
		ignore_gryphs_ = reinterpret_cast<char *>(pool().realloc(ignore_gryphs_, osz + sz + 1));
		std::strncpy(ignore_gryphs_ + osz, gryphs, sz);	
	}
}
}
}

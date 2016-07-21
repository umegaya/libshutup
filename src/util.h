#pragma once

#include <map>
#include <cstring>

#include "types.h"

// macro!!!
#if defined(TEST) || defined(DEBUG)
#define TRACE(...) std::printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

namespace shutup {
//
//	template utils
//
namespace tpl {
//map which can handle const char *key correctly
class pstring_compare {
public:
	static const int MAX_KEY_LEN = 1024;
	bool operator()(const char *lhs, const char *rhs) const {
		size_t ll = strnlen(lhs, MAX_KEY_LEN), rl = strnlen(rhs, MAX_KEY_LEN);
		int r = std::memcmp(lhs, rhs, std::min(ll, rl));
		if (r == 0) {
			return ll < rl;
		} else {
			return r < 0;
		}
	}
};
template <class T, class A> class psmap : public std::map<const char*, T, pstring_compare, A> {
public:
	typedef std::map<const char*, T, pstring_compare, A> super;
	psmap(const typename super::allocator_type& a) : super(a) {}
	T *get(const char *key) {
		auto i = super::find(key);
		return (i != super::end()) ? &(*i) : nullptr;
	}
};
}
//
//	utf8 utils
//
class utf8 {
public:
	//string sets
	static const char *hiras;
	static const char *katas;
	static const char *alphabets;
	static const char *upper_alphabets;
	static const char *wide_lower_alphabets;
	static const char *wide_upper_alphabets;
	static const char *numbers;
	static const char *wide_numbers;
	static const char *half_katas;
	//methods
	static inline int peek(const u8 *in, int ilen) {
		int mask = 0x80, count = 0, bit = in[0];
		while(bit != 0x00) {
			if ((bit & mask) == 0) {
				break;
			}
			count++;
			mask >>= 1;
		}
		return count <= 0 ? 1 : count;
	}
	static inline int copy(const u8 *in, int ilen, u8 *out, int olen, int n) {
		int n_read = 0, n_write = 0;
		while (n > 0 && ilen > n_read && olen > n_write) {
			int tmp = peek(in + n_read, ilen - n_read);
			if ((n_write + tmp) > olen) {
				return -1;
			} else {
				std::memcpy(out + n_write, in + n_read, tmp);
				n_write += tmp;
				n_read += tmp;
				n--;
			}
		}
		return n_write;
	}
	static bool is_kana_string(const char *str);
	static int to_hebon_roman(const u8 *in, int ilen, u8 *out, int *olen);
	static int to_japan_roman(const u8 *in, int ilen, u8 *out, int *olen);
	static int widen_kata(const u8 *in, int ilen, u8 *out, int *olen);
	static int shrunk_alnum(const u8 *in, int ilen, u8 *out, int *olen);
};
}
#pragma once

#include <map>
#include <cstring>

#include "types.h"

// macro!!!
#if defined(TEST) || defined(DEBUG)
#include <cstdio>
#define TRACE(...) std::printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

namespace shutup {
//
//  tpl utils
//
namespace util {
static inline int convert(const u8 *in, int ilen, u8 *out, int olen, std::function<int(const u8*, int, u8*, int*)> conv) {
	int n_read = 0, n_write = 0, wtmp, rtmp;
	while (olen > n_write && ilen > n_read) {
		wtmp = olen - n_write;
		//TRACE("read_next_with_normalize: %d %d %d %d\n", ilen, olen, n_read, n_write);
		rtmp = conv(in + n_read, ilen - n_read, out + n_write, &wtmp);
		if (rtmp < 0) {
			return rtmp;
		} else {
			n_read += rtmp;
			n_write += wtmp;
		}
	}
	return n_write;
}
}

//
//	utf8 utils
//
class utf8 {
public:
	static const int MAX_BYTE_PER_GRYPH = 6;
	//string sets	
	static const char *hiras;
	static const char *katas;
	static const char *wide_katas;
	static const char *wide_psign_katas;
	static const char *wide_sonant_katas;
	static const char *alphabets;
	static const char *upper_alphabets;
	static const char *wide_lower_alphabets;
	static const char *wide_upper_alphabets;
	static const char *numbers;
	static const char *wide_numbers;
	static const char *half_katas;
	static const char *vowels[];
	static const char *hebon_consonants[];
	static const char *japan_consonants[];
	static char work[8];
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
	static int to_roman(const u8 *in, int ilen, u8 *out, int *olen, bool assimilated, 
			const char *consonants[], const char *(*consonant_exception)(const char *, int, int, bool));
	static const char *hebon_consonant_exception(const char *normal, int vowel_index, int consonant_index, bool assimilated);
	static const char *japan_consonant_exception(const char *normal, int vowel_index, int consonant_index, bool assimilated);
	static int widen_kata(const u8 *in, int ilen, u8 *out, int *olen);
	static int norm_space_and_lf(const u8 *in, int ilen, u8 *out, int *olen);
	static int shrunk_alnum(const u8 *in, int ilen, u8 *out, int *olen);
};
}

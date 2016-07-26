#include <cstring>

#include "util.h"

namespace shutup {
//
//	utf8
//
const char *utf8::alphabets = 
"abcdefghijklmnopqrstuvwxyz";

const char *utf8::upper_alphabets = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char *utf8::wide_lower_alphabets = 
"ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ";

const char *utf8::wide_upper_alphabets = 
"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ";

const char *utf8::numbers = 
"0123456789";

const char *utf8::wide_numbers = 
"０１２３４５６７８９";

char utf8::work[8];

int utf8::shrunk_alnum(const u8 *in, int ilen, u8 *out, int *olen) {
	if (*olen <= 0) {
		return -1;
	}
	struct {
		const char *check;
		const char *replace;
		int cl, rl;
	} tests[] = {
		{wide_lower_alphabets, alphabets, 3, 1},
		{wide_upper_alphabets, alphabets, 3, 1},
		{upper_alphabets, alphabets, 1, 1},
		{wide_numbers, numbers, 3, 1},
	};
	for (int i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
		auto t = tests[i];
		u8 buff[utf8::MAX_BYTE_PER_GRYPH + 1];
		int r = copy(in, ilen, buff, sizeof(buff), 1);
		buff[r] = 0;
		const char *p = std::strstr(t.check, reinterpret_cast<char *>(buff));
		if (p != nullptr) {
			size_t ofs = (int)((p - t.check) / t.cl);
			std::memcpy(out, t.replace + ofs, t.rl);
			*olen = t.rl;
			return r;
		}
	}
	*olen = 0;
	return 0;
}

int utf8::norm_space_and_lf(const u8 *in, int ilen, u8 *out, int *olen) {
	//TODO: remove linefeed and convert tab and wide space to space
	return 0;
}

//
// utf8::jp
//
#include "language/utiljp.h"

}


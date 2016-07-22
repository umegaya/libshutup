#include <cstring>

#include "util.h"

namespace shutup {
//
//	utf8::jp
//
//以下は同じ文字のオフセット位置と５０音表内での位置をちゃんと揃えること.
const char *utf8::hiras = 
"あいうえお"
"かきくけこ"
"さしすせそ"
"たちつてと"
"なにぬねの"
"はひふへほ"
"まみむめも"
"や　ゆ　よ"
"らりるれろ"
"わゐ　ゑを"
"ぁぃぅぇぉ"
"ゃ　ゅ　ょ"
"がぎぐげご"
"ざじずぜぞ"
"だぢづでど"
"ばびぶべぼ"
"ぱぴぷぺぽ"
"　　ゔ　　"
"んっー";

const char *utf8::katas = 
"アイウエオ"
"カキクケコ"
"サシスセソ"
"タチツテト"
"ナニヌネノ"
"ハヒフヘホ"
"マミムメモ"
"ヤ　ユ　ヨ"
"ラリルレロ"
"ワヰ　ヱヲ"
"ァィゥェォ"
"ャ　ュ　ョ"
"ガギグゲゴ"
"ザジズゼゾ"
"ダヂヅデド"
"バビブベボ"
"パピプペポ"
"　　ヴ　　"
"ンッー";

const char *utf8::half_katas = 
"ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ";

const char *utf8::wide_katas = 
"ヲァィゥェォャュョッーアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワン";

const char *utf8::wide_psign_katas = 
"パピプペポ";

const char *utf8::wide_sonant_katas = 
"ガギグゲゴザジズゼゾダヂヅデド　　　　　バビブベボ";

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

bool utf8::is_kana_string(const char *str) {
	size_t len = std::strlen(str);
	int idx = 0;
	while (len > idx) {
		int tmp = peek(reinterpret_cast<const u8 *>(str + idx), len - idx);
		if (tmp == 0) {
			break;
		}
		char gryph[tmp + 1];
		std::memcpy(gryph, str + idx, tmp); gryph[tmp] = 0;
		if (nullptr == std::strstr(utf8::hiras, gryph) && nullptr == std::strstr(utf8::katas, gryph)) {
			return false;
		}
		idx += tmp;
	}
	return true;
}

int utf8::to_hebon_roman(const u8 *in, int ilen, u8 *out, int *olen) {
	return 0;
}
int utf8::to_japan_roman(const u8 *in, int ilen, u8 *out, int *olen) {
	return 0;
}
int norm_space_and_lf(const u8 *in, int ilen, u8 *out, int *olen) {
	//TODO: remove linefeed and convert tab and wide space to space
	return 0;
}
int utf8::widen_kata(const u8 *in, int ilen, u8 *out, int *olen) {
	if (*olen < 3) {
		return -1; //buf short
	}
	u8 buff[utf8::MAX_BYTE_PER_GRYPH + 1];
	int r = copy(in, ilen, buff, sizeof(buff), 1);
	if (r <= 0) {
		*olen = 0;
		return 0;
	}
	buff[r] = 0;
	const char *p = std::strstr(half_katas, reinterpret_cast<char *>(buff));
	if (p != nullptr) {
		if (std::memcmp(p, "ﾞ", r) == 0 || std::memcmp(p, "ﾟ", r) == 0) {
			*olen = 0;
			return r;
		} else {
			size_t ofs = (p - half_katas), idx = ofs / 3;
			//ウ、カ〜ト、ハ〜ホ.
			//TRACE("ofs = %zu(%s)%d %d\n", ofs, buff, ilen, r);
			if (idx == 13 || (idx >= 16 && idx <= 31) || (idx >= 36 && idx <= 41)) {
				u8 buff2[utf8::MAX_BYTE_PER_GRYPH + 1];
				if (ilen > r) {
					int r2 = copy(in + r, ilen - r, buff2, sizeof(buff2), 1);
					buff2[r2] = 0;
					if (std::memcmp(buff2, "ﾞ", r2) == 0) {
						if (idx == 13) {
							std::memcpy(out, "ヴ", 3);
							*olen = 3;
							return r + r2;
						}
						std::memcpy(out, wide_sonant_katas + ofs - 16*3, 3);
						*olen = 3;
						return r + r2;
					} else if (std::memcmp(buff2, "ﾟ", r2) == 0) {
						if (idx >= 36 && idx <= 41) {
							std::memcpy(out, wide_psign_katas + ofs - 36*3, 3);
							*olen = 3;
							return r + r2;
						}
					}
				}
			}
			std::memcpy(out, wide_katas + ofs, 3);
			*olen = 3;
			return r;
		}
	} else {
		//TODO: [ト゛] => [ド] 
	}
	*olen = 0;
	return 0;
}
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
}


#include "util.h"

namespace shutup {
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
"ンッー";

const char *utf8::half_katas = 
"ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ";

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
	while (true) {
		int tmp = peek(reinterpret_cast<const u8 *>(str + idx), len - idx);
		if (tmp == 0) {
			break;
		}
		char gryph[tmp + 1];
		std::memcpy(gryph, str + idx, tmp); gryph[tmp] = 0;
		if (nullptr == std::strstr(utf8::hiras, gryph) || nullptr == std::strstr(utf8::katas, gryph)) {
			return false;
		}
	}
	return true;
}

int utf8::to_hebon_roman(const u8 *in, int ilen, u8 *out, int *olen) {
	return 0;
}
int utf8::to_japan_roman(const u8 *in, int ilen, u8 *out, int *olen) {
	return 0;
}
int utf8::widen_kata(const u8 *in, int ilen, u8 *out, int *olen) {
	return 0;
}
int utf8::shrunk_alnum(const u8 *in, int ilen, u8 *out, int *olen) {
	return 0;
}
}


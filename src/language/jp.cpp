#include <cstdint>
#include <cstring>
#include <cstdlib> 

#include "language/jp.h"

namespace shutup {
namespace language {
//チェックの時、無視する文字達.
static const char *ignore_glyphs = 
"-+!\"#$%%&()*/,:;<=>?@[\\]^_{|}~ "
"ｰ" //half kata hyphen
"、。，．・：；？！゛゜´｀¨＾￣＿ヽヾゝゞ〃仝々〆〇ー‐／＼～∥｜…‥"
"‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴"
"♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓"
"〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭"
"♪†‡¶◯〝〟≒≡∫∮√⊥∠∵∩∪　";

int JP::init() {
	//区切り文字として無視される文字を指定する.
	set_ignore_gryphs(ignore_glyphs);
	//ひらがな、カタカナをそれぞれのaliasとする.
	size_t hlen = std::strlen(utf8::hiras), klen = std::strlen(utf8::katas);
	int hidx = 0, kidx = 0;
	while (kidx < klen && hidx < hlen) {
		int htmp = utf8::peek(reinterpret_cast<const u8 *>(utf8::hiras + hidx), hlen - hidx);
		int ktmp = utf8::peek(reinterpret_cast<const u8 *>(utf8::katas + kidx), klen - kidx);
		if (htmp == 0 || ktmp == 0) {
			break;
		}
		char hira[htmp + 1], kata[ktmp + 1];
		std::memcpy(hira, utf8::hiras + hidx, htmp); hira[htmp] = 0;
		std::memcpy(kata, utf8::katas + kidx, ktmp); kata[ktmp] = 0;
		link_alias(hira, kata);
		hidx += htmp;
		kidx += ktmp;
	}
	//TODO: ソとンみたいなのとか、ひらがなの大文字小文字とか.
	return 0;
}
//normalizeで使うnormalizerを定義する.
WordChecker::normalizer *JP::normalizers(int *n_norm) {
	static normalizer norms[3] = {
		nullptr,
		utf8::widen_kata,		//try widen kata
		utf8::shrunk_alnum,	//try shrunk alphabet (half byte lower)
	};
	norms[0] = remove_ignored_;
	*n_norm = (int)(sizeof(norms)/sizeof(norms[0]));
	return norms;
}
//aliasは文字単位での組み合わせを全てチェックしてしまうので、そこまでチェックしたくない場合、
//ここで単語単位で同じ意味のものを登録する.
void JP::add_synonym(const char *pattern, Checker &c) {
	if (utf8::is_kana_string(pattern)) {
		//ローマ字変換の登録.
		c.add_word(to_hebon_roman(pattern));
		c.add_word(to_japan_roman(pattern));
	}
	c.add_word(pattern);
}
const char *JP::to_hebon_roman(const char *str) {
	u8 buff[256];
	int blen = 256;
	utf8::to_hebon_roman(reinterpret_cast<const u8*>(str), std::strlen(str), buff, &blen);
	return str;
}
const char *JP::to_japan_roman(const char *str) {
	u8 buff[256];
	int blen = 256;
	utf8::to_japan_roman(reinterpret_cast<const u8*>(str), std::strlen(str), buff, &blen);
	return str;
}
}
}

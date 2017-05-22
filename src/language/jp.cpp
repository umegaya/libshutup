#include <cstdint>
#include <cstring>
#include <cstdlib> 

#include "util.h"
#include "checker.h"
#include "jp.h"

namespace shutup {
namespace language {
//チェックの時、無視する文字達.
static const char *ignore_list = 
".-\"#$%%&()*/,:;<=>?@[\\]^_{|}~ "
"ｰ" //half kata hyphen
"、。，．・：；？！゛゜´｀¨＾￣＿ヽヾゝゞ〃仝々〆〇ー‐／＼～∥｜…‥"
"‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴"
"♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓"
"〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭"
"♪†‡¶◯〝〟≒≡∫∮√⊥∠∵∩∪　";

int JP::init() {
	//区切り文字として無視される文字を指定する.
	ignore_glyphs(ignore_list);
	//ひらがな、カタカナをそれぞれのaliasとする.
	size_t hlen = std::strlen(utf8::jp::hiras), klen = std::strlen(utf8::jp::katas);
	int hidx = 0, kidx = 0;
	while (kidx < klen && hidx < hlen) {
		int htmp = utf8::peek(reinterpret_cast<const u8 *>(utf8::jp::hiras + hidx), (int)hlen - hidx);
		int ktmp = utf8::peek(reinterpret_cast<const u8 *>(utf8::jp::katas + kidx), (int)klen - kidx);
		if (htmp == 0 || ktmp == 0) {
			break;
		}
		char hira[htmp + 1], kata[ktmp + 1];
		std::memcpy(hira, utf8::jp::hiras + hidx, htmp); hira[htmp] = 0;
		std::memcpy(kata, utf8::jp::katas + kidx, ktmp); kata[ktmp] = 0;
		link_alias(hira, kata);
		hidx += htmp;
		kidx += ktmp;
	}
	add_alias("ン", "ソ");
	//normalizerの追加
	normalizers_.push_back(utf8::jp::widen_kata);
	normalizers_.push_back(utf8::shrunk_alnum);
	return 0;
}
//aliasは文字単位での組み合わせを全てチェックしてしまうので、そこまでチェックしたくない場合、
//ここで単語単位で同じ意味のものを登録する.
void JP::add_synonym(const char *pattern, Checker &c, void *ctx) {
	if (utf8::jp::is_kana_string(pattern)) {
		//ローマ字変換の登録.まずヘボン式.
		int ilen = (int)std::strlen(pattern), olen = ilen;
		const u8 *in = reinterpret_cast<const u8*>(pattern);
		u8 out[ilen]; 
		int r = util::convert(in, ilen, out, olen, std::bind(&utf8::jp::to_hebon_roman, 
				std::placeholders::_1, std::placeholders::_2, 
				std::placeholders::_3, std::placeholders::_4));
		out[r] = 0;
		c.add_word(reinterpret_cast<const char*>(out), ctx);
		//日本式.
		r = util::convert(in, ilen, out, olen, std::bind(&utf8::jp::to_japan_roman, 
				std::placeholders::_1, std::placeholders::_2, 
				std::placeholders::_3, std::placeholders::_4));
		out[r] = 0;
		c.add_word(reinterpret_cast<const char*>(out), ctx);
	}
}
}
}

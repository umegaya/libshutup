#include <cstdio>
#include <vector>
#include "checker.h"
#include "language.h"
#include "util.h"


namespace test {
namespace language {
struct testcase {
	static bool alias_tested_;
	struct normalize {
		const char *text_;
		const char *expect_;
	};
	//test explanation
	const char *message_;
	const char *lang_;
	//test
	std::vector<normalize> normalizes_;
	const char *test() {
		shutup::Checker::Mempool *p = new shutup::Checker::Mempool(nullptr);
		shutup::language::WordChecker *wc = shutup::Checker::by(lang_, *p);
		wc->init();
		if (!alias_tested_) {
			for (auto &c : wc->aliases_map()) {
				const char *k = std::strstr(shutup::utf8::jp::katas, c.first.c_str());

				if (k != nullptr) {
					const char *v = std::strstr(shutup::utf8::jp::hiras, c.second[0].c_str());
					if ((k - shutup::utf8::jp::katas) != (v - shutup::utf8::jp::hiras)) {
						return "wrong kata-kana alias setting";
					}
				} else {
					k = std::strstr(shutup::utf8::jp::hiras, c.first.c_str());
					if (k == nullptr) {
						TRACE("k not found: [%s]\n", c.first.c_str());
						return "alias key exists which is not hira-kana or kata-kana";
					}
					const char *v = std::strstr(shutup::utf8::jp::katas, c.second[0].c_str());
					if ((v - shutup::utf8::jp::katas) != (k - shutup::utf8::jp::hiras)) {
						return "wrong hira-kana alias setting";
					}
				}
			}
			alias_tested_ = true;
		}
		for (auto &n : normalizes_) {
			int rlen = std::strlen(n.text_);
			int olen = rlen * shutup::utf8::MAX_BYTE_PER_GRYPH;
			shutup::u8 out[olen];
			int wlen = wc->normalize(reinterpret_cast<const shutup::u8*>(n.text_), rlen, out, olen);
			if (wlen < 0) {
				TRACE("norm: fail %d\n", wlen);
				return "fail to normalize";
			}
			out[wlen] = 0;
			if (std::strcmp(n.expect_, reinterpret_cast<const char*>(out)) != 0) {
				TRACE("norm:[%s]=>[%s]\n", n.expect_, out);
				return "normalize result does not match expected";
			}
		}
		return nullptr;
	}
};
bool testcase::alias_tested_ = false;
}
}

extern const char *language_test() {
	std::vector<test::language::testcase> cases{
		{
			.message_ = "jp test",
			.lang_ = "jp",
			.normalizes_ = {
				{"ｱ", "ア"},
				{"ｦ", "ヲ"},
				{"ﾝ", "ン"},
				{"ｰ", ""},
				{"ﾞ", ""},
				{"ﾟ", ""},
				{"ｶﾞ", "ガ"},
				{"ｺﾞ", "ゴ"},
				{"ﾄﾞ", "ド"},
				{"ﾊﾞ", "バ"},
				{"ﾎﾞ", "ボ"},
				{"ﾊﾟ", "パ"},
				{"ﾎﾟ", "ポ"},
				{"ｳﾞ", "ヴ"},
				{"ｱﾟ", "ア"},
				{"ｶﾟ", "カ"},
				{"ｱﾞ", "ア"},
				{"a", "a"},
				{"Ｚ", "z"},
				{"ｂ", "b"},
				{"C", "c"},
				{"1", "1"},
				{"１", "1"},
				{"ｱイｳエｵ", "アイウエオ"},
				{"１2３4５", "12345"},
				{"ﾊﾟﾋﾌﾟﾍﾎﾟ", "パヒプヘポ"},
				{"あいうえお", "あいうえお"},
				{"ｶﾞ２ｳﾞ4", "ガ2ヴ4"},
				{"あ\"い#う$え%%お\\", "あいうえお"},
				{"ﾊﾟ（ﾋ）ﾌﾞ〔ﾍ〕ﾎﾞ", "パヒブヘボ"},
			}
		},
	};
	for (auto &c : cases) {
		std::printf("language_test %s\n", c.message_);
		auto result = c.test();
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

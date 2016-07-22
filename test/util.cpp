#include <cstdio>
#include <vector>
#include "util.h"


namespace test {
namespace util {
struct testcase {
	struct normalize {
		const char *text_;
		const char *expect_;
		int rlen_, wlen_;
	};
	struct check {
		const char *text_;
		bool expect_;
	};
	//test explanation
	const char *message_;
	//normalize test
	int (*normalizer_)(const shutup::u8 *in, int ilen, shutup::u8 *out, int *olen);
	std::vector<normalize> normalizes_;
	//check test
	bool (*checker_)(const char *str);
	std::vector<check> checks_;
	const char *test() {
		if (normalizer_ != nullptr) {
			for (auto &n : normalizes_) {
				int ilen = std::strlen(n.text_), olen = ilen * shutup::utf8::MAX_BYTE_PER_GRYPH, r;
				shutup::u8 out[olen];
				//TRACE("norm: [%s]=>[%s]\n", n.text_, n.expect_);
				if ((r = normalizer_(reinterpret_cast<const shutup::u8 *>(n.text_), ilen, out, &olen)) < 0) {
					return "normalize fails";
				} else {
					out[olen] = 0;
					if (std::strcmp(n.expect_, reinterpret_cast<const char *>(out)) != 0 || r != n.rlen_ || olen != n.wlen_) {
						TRACE("result: [%s]=>[%s] %d %d %d %d\n", n.expect_, reinterpret_cast<const char *>(out), r, n.rlen_, olen, n.wlen_);
						return "normalize result does not match expectation";
					}
				}
			}
		}
		if (checker_ != nullptr) {
			for (auto &c : checks_) {
				//TRACE("test %s\n", c.text_);
				if (c.expect_ != checker_(c.text_)) {
					return "check result does not match expectation";
				}
			}
		}
		return nullptr;
	}
};
}
}

extern const char *util_test() {
	std::vector<test::util::testcase> cases{
		{
			.message_ = "is_kana_string test",
			.checks_ = {
				{"abc", false},
				{"ＡＢＣ",false},
				{"ｱｲｳ", false},
				{"あいう", true},
				{"アイウエオ", true},
				{"あいうｱｲｳ", false},
				{"あｱいう", false},
				{"ｱｲｳあいう", false},
				{"アイヴ",true},
				{"あア", true},
				{"123", false},
				{"＠", false},
				{"\t@", false},
			},
			.checker_ = shutup::utf8::is_kana_string,
		},
		{
			.message_ = "widen_kata test",
			.normalizes_ = {
				{"a", "", 0, 0},
				{"Ａ", "", 0, 0},
				{"A", "", 0, 0},
				{"1", "", 0, 0},
				{"ｱ", "ア", 3, 3},
				{"ｦ", "ヲ", 3, 3},
				{"ﾝ", "ン", 3, 3},
				{"ｰ", "ー", 3, 3},
				{"ﾞ", "", 3, 0},
				{"ﾟ", "", 3, 0},
				{"ｶﾞ", "ガ", 6, 3},
				{"ｺﾞ", "ゴ", 6, 3},
				{"ﾄﾞ", "ド", 6, 3},
				{"ﾊﾞ", "バ", 6, 3},
				{"ﾎﾞ", "ボ", 6, 3},
				{"ﾊﾟ", "パ", 6, 3},
				{"ﾎﾟ", "ポ", 6, 3},
				{"ｳﾞ", "ヴ", 6, 3},
				{"ｱﾟ", "ア", 3, 3},
				{"ｶﾟ", "カ", 3, 3},
				{"ｱﾞ", "ア", 3, 3},
			},
			.normalizer_ = shutup::utf8::widen_kata,
		},
		{
			.message_ = "shrunk_alnum test",
			.normalizes_ = {
				{"a", "", 0, 0},
				{"Ｚ", "z", 3, 1},
				{"ｂ", "b", 3, 1},
				{"C", "c", 1, 1},
				{"ｱ", "", 0, 0},
				{"ｶﾞ", "", 0, 0},
				{"1", "", 0, 0},
				{"１", "1", 3, 1},
			},
			.normalizer_ = shutup::utf8::shrunk_alnum,
		},
	};
	for (auto &c : cases) {
		std::printf("util_test %s\n", c.message_);
		auto result = c.test();
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

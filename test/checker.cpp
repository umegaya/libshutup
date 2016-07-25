#include <cstdio>
#include <vector>
#include "checker.h"
#include "language.h"
#include "util.h"


namespace test {
namespace checker {
struct testcase {
	struct input {
		const char *text_;
		bool filtered_;
		const char *matched_;
		const char *expect_;
	};
	//test explanation
	const char *message_;
	const char *lang_;
	//test
	std::vector<const char*> filters_;
	const char *mask_;
	std::vector<input> inputs_;
	const char *test() {
		shutup::Checker c(lang_, nullptr);
		int count = 0;
		for (auto w : filters_) {
		TRACE("add filter word %s\n", w);
			c.add(w);
			count++;
		}
		TRACE("start input test\n");
		for (auto &i : inputs_) {
			int ilen = std::strlen(i.text_);
			int olen = ilen * shutup::utf8::MAX_BYTE_PER_GRYPH;
			char buff[olen];
			//TRACE("===== %s: should_filter test\n", i.text_);
			if (i.filtered_ != c.should_filter(i.text_, buff, &olen)) {
				TRACE("input:[%s]\n", i.text_);
				return "text should be filtered but actually not";
			}
			//TRACE("===== %s: should_filter test ok\n", i.text_);
			buff[olen] = 0;
			//TRACE("===== %s: should_filter word test\n", i.text_);
			if (i.filtered_ && std::strcmp(i.matched_, buff) != 0) {
				TRACE("filtered:[%s] [%s]\n", i.matched_, buff);
				return "filtered but match part does not match expected";
			}
			//TRACE("===== %s: filter test\n", i.text_);
			olen = ilen * shutup::utf8::MAX_BYTE_PER_GRYPH;
			const char *r = mask_ == nullptr ? c.filter(i.text_, buff, &olen) : c.filter(i.text_, buff, &olen, mask_);
			if (std::strcmp(i.expect_, r) != 0) {
				TRACE("filter:[%s] => [%s]\n", i.expect_, r);
				return "filter result does not match expected";
			}
		}
		return nullptr;
	}
};
}
}

extern const char *checker_test() {
	std::vector<test::checker::testcase> cases{
		{
			.message_ = "jp test",
			.lang_ = "jp",
			.filters_ = {
				"badword",
				"バッドワード",
			},
			.inputs_ = {
				{"ＢａdＷorDです", true, "ＢａdＷorD", "???????です"},
				{"ＢａdＷorOです", false, "", "ＢａdＷorOです"},
				{"これってＢ|ａ「d」Ｗ(or)Dなの", true, "Ｂ|ａ「d」Ｗ(or)D", "これって?|?「?」?(\?\?)?なの"},
				{"_ﾊﾞｯドわーﾄﾞ_", true, "ﾊﾞｯドわーﾄﾞ", "_?????ー??_"},
				{"これはﾊﾞ：ｯ；ﾄﾞ_？_わｰドですか？", true, "ﾊﾞ：ｯ；ﾄﾞ_？_わｰド", "これは??：?；??_？_?ｰ?ですか？"},
				{"これはﾊﾞ：ｯ；ﾄﾞ_？_わタですか？", false, "", "これはﾊﾞ：ｯ；ﾄﾞ_？_わタですか？"},
			},
		},
	};
	for (auto &c : cases) {
		std::printf("checker_test %s\n", c.message_);
		auto result = c.test();
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

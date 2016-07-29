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
		void *ctx_;
	};
	struct filter {
		const char *text_;
		void *ctx_;
	};
	//test explanation
	const char *message_;
	const char *lang_;
	//test
	std::vector<filter> filters_;
	const char *mask_;
	std::vector<input> inputs_;
	const char *test() {
		shutup::Checker c(lang_, nullptr);
		int count = 0;
		for (auto &f : filters_) {
		//TRACE("add filter word %s\n", w);
			c.add(f.text_, f.ctx_);
			count++;
		}
		for (auto &i : inputs_) {
			int ilen = std::strlen(i.text_);
			int start, count;
			void *ctx;
			if (i.filtered_ != c.should_filter(i.text_, std::strlen(i.text_), &start, &count, &ctx)) {
				TRACE("input:[%s]\n", i.text_);
				return "text should be filtered but actually not";
			}
			//TRACE("test[%s]s:%d,c:%d\n", i.text_, start, count);
			if (i.filtered_) {
				char buff[count + 1];
				std::memcpy(buff, i.text_ + start, count); buff[count] = 0;
				if (std::strcmp(i.matched_, buff) != 0 || ctx != i.ctx_) {
					TRACE("filtered:[%s] [%s]\n", i.matched_, buff);
					return "filtered but match part does not match expected";
				}
			}
			int olen = ilen * shutup::utf8::MAX_BYTE_PER_GRYPH;
			char buff[olen];
			const char *r = mask_ == nullptr ? 
				c.filter(i.text_, std::strlen(i.text_), buff, &olen) : 
				c.filter(i.text_, std::strlen(i.text_), buff, &olen, mask_);
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

static void *p(int id) {
	return reinterpret_cast<void *>(id);
}

extern const char *checker_test() {
	std::vector<test::checker::testcase> cases{
		{
			.message_ = "jp test",
			.lang_ = "jp",
			.filters_ = {
				{"badword", p(1)},
				{"", p(2)},
				{"馬津怒輪亜怒",p(3)},
				{"バッドワード",p(4)},
				{"ワンド",p(5)},
				{"ソビエト",p(6)},
				{"ぢょく",p(7)}
			},
			.inputs_ = {
				{"ＢａdＷorDです", true, "ＢａdＷorD", "???????です", p(1)},
				{"ＢａdＷorOです", false, "", "ＢａdＷorOです"},
				{"これってＢ|ａ「d」Ｗ(or)Dなの", true, "Ｂ|ａ「d」Ｗ(or)D", "これって?|?「?」?(\?\?)?なの", p(1)},
				{"_ﾊﾞｯドわーﾄﾞ_", true, "_ﾊﾞｯドわーﾄﾞ", "_?????ー??_", p(4)},
				{"これはﾊﾞ：ｯ；ﾄﾞ_？_わｰドですか？", true, "ﾊﾞ：ｯ；ﾄﾞ_？_わｰド", "これは??：?；??_？_?ｰ?ですか？", p(4)},
				{"これはﾊﾞ：ｯ；ﾄﾞ_？_わタですか？", false, "", "これはﾊﾞ：ｯ；ﾄﾞ_？_わタですか？"},
				{"これはbaddo-wa-doですか?", true, "baddo-wa-do", "これは\?\?\?\?\?-\?\?-\?\?ですか?", p(4)},
				{"これはbaddo-wa-diですか?", false, "", "これはbaddo-wa-diですか?"},
				{
					"これは b a d w o r d でしょうか？それともﾊﾞｯﾄﾞﾜｰﾄﾞでしょうか？あるいはbaddowa-doですか？", 
					true, 
					" b a d w o r d", 
					"これは \? \? \? \? \? \? \? でしょうか？それとも\?\?\?\?\?\?ｰ\?\?でしょうか？あるいは\?\?\?\?\?\?\?-\?\?ですか？",
					p(1),
				},
				{"OK：「ンビエト」、NG：「ワソド」", true, "：「ワソド", "OK：「ンビエト」、NG：「???」", p(5)},
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

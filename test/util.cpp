#include <cstdio>
#include <vector>
#include "util.h"


namespace test {
namespace util {
struct testcase {
	const char *message_;
	const char *text_;
	const char *test() {
		return nullptr;
	}
};
}
}

extern const char *util_test() {
	std::vector<test::util::testcase> cases{
		{
			.message_ = "is_kana_string test",
			.text_ = "text",
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

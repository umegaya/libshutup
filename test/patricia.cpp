#include <cstdio>
#include "patricia.h"

struct traverse_testcase {
	const char *message_;
	std::vector<const char *> inserts_;
	std::vector<const char *> results_;
	//test routine
	bool test() {
		shutup::Patricia p;
		for (auto i : inserts_) {
			p.add(i);
		}
		auto results = results_;
		int c = 0;
		if (!p.traverse([&c, &results] (shutup::Patricia::NodeData *n, int depth) -> bool {
			auto s = results[c++];
			return std::strlen(s) == n->len() && memcmp(s, n->bytes(), n->len()) == 0;
		})) {
			p.dump();
			return false;
		};
		if (c != results.size()) {
			p.dump();
			return false;
		}
		p.dump();
		return true;		
	}
};

extern const char *patricia_test() {
	std::vector<traverse_testcase> cases{
		{
			.message_="simple tree",
			.inserts_={"hogi", "hoge"},
			.results_={
				"hog",
					"e",
					"i"
			}
		}
	};
	for (auto &c : cases) {
		std::printf("patricia_test %s\n", c.message_);
		if (!c.test()) {
			return c.message_;
		}
	}
	return nullptr;
}

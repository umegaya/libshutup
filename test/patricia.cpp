#include <cstdio>
#include "patricia.h"

struct traverse_testcase {
	const char *message_;
	std::vector<const char *> inserts_;
	std::vector<const char *> results_;
	//test routine
	void dump_tree(shutup::Patricia &p) {
		p.traverse([] (shutup::Patricia::ExportNode *n) -> bool {
			char buffer[256];
			std::memcpy(buffer, n->bytes(), n->len());
			buffer[n->len()] = 0;
			std::printf("key:%s\n", buffer);
			return true;
		});
	}
	bool test() {
		shutup::Patricia p;
		for (auto i : inserts_) {
			p.add(i);
		}
		auto results = results_;
		int c = 0;
		if (!p.traverse([&c, &results] (shutup::Patricia::ExportNode *n) -> bool {
			auto s = results[c++];
			return std::strlen(s) == n->len() && memcmp(s, n->bytes(), n->len()) == 0;
		})) {
			dump_tree(p);		
			return false;
		};
		dump_tree(p);
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

#include <cstdio>
#include "patricia.h"

struct testcase {
	struct operation {
		const char *type;
		const char *data;
	};
	struct expect {
		const char *data;
		int depth;
	};
	struct search {
		std::vector<const char*> found;
		std::vector<const char*> not_found;
	};
	const char *message_;
	std::vector<operation> operations_;
	std::vector<expect> expects_;
	search search_;
	shutup::IMatcher *matcher_;
	//test routine
	const char *test() {
		shutup::Patricia p(matcher_);
		for (auto &o : operations_) {
			if (std::strcmp(o.type, "+") == 0) { 
				p.add(o.data); 
			} else if (std::strcmp(o.type, "-") == 0) {
				p.remove(o.data);
			} else {
				return "invalid operation";
			}
		}
		auto expects = expects_;
		int c = 0;
		if (!p.traverse([&c, &expects] (shutup::Patricia::NodeData *n, int depth) -> bool {
			if (expects.size() <= c) {
				return false;
			}
			auto e = expects[c++];
			return e.depth == depth && 
				std::strlen(e.data) == n->len() && 
				(n->len() == 0 || memcmp(e.data, n->bytes(), n->len()) == 0);
		})) {
			p.dump();
			return "traverse result does not match expects (content)";
		};
		if (c != expects.size()) {
			p.dump();
			return "traverse result does not match expects (length)";
		}
		int ofs;
		for (auto s : search_.found) {
			bool found = (p.contains(s, &ofs) && ofs == std::strlen(s));
			if (!found) {
				p.dump();
				return "entry not found that is ought to be";
			}
		}
		for (auto s : search_.not_found) {
			bool found = (p.contains(s, &ofs) && ofs == std::strlen(s));
			if (found) {
				p.dump();
				return "entry found that is not ought to be";
			}
		}
		p.dump();
		return nullptr;		
	}
};

extern const char *patricia_test() {
	std::vector<testcase> cases{
		{
			.message_ = "empty",
			.operations_ = {},
			.expects_ = {},
			.search_= { 
				.not_found={""} 
			},
		},
		{
			.message_ = "simple insert",
			.operations_ = {{"+", "goge"}, {"+", "hoge"}},
			.expects_ = {
				{"goge", 1},
				{"hoge", 1},
			},
			.search_ = {
				.found = {"goge", "hoge"},
			},
		},
		{
			.message_ = "insert cause split",
			.operations_ = {{"+", "hogi"}, {"+", "hoge"}},
			.expects_ = {
				{"hog", 1},
					{"e", 2},
					{"i", 2},
			},
			.search_ = {
				.found = {"hoge", "hogi"}, 
				.not_found = {"hog", "e", "i"},
			},
		},
		{
			.message_ = "insert + remove",
			.operations_ = {{"+", "abcdef"}, {"-", "abcdef"}},
			.expects_ = {},
			.search_ = {
				.not_found = {"abcdef"},
			},
		},
		{
			.message_ = "insert + remove not exists",
			.operations_ = {{"+", "abcdef"}, {"-", "abcdee"}},
			.expects_ = {
				{"abcdef", 1}
			},
			.search_ = {
				.found = {"abcdef"},
				.not_found = {"abcdee"},
			},
		},
		{
			.message_ = "insert + remove not exists (no terminate)",
			.operations_ = {{"+", "hogi"}, {"+", "hoge"}, {"-", "hog"}},
			.expects_ = {
				{"hog", 1},
					{"e", 2},
					{"i", 2},
			},
			.search_ = {
				.found = {"hoge", "hogi"}, 
				.not_found = {"hog", "e", "i"},
			},
		},
		{
			.message_ = "insert + remove cause merge",
			.operations_ = {{"+", "hogi"}, {"+", "hoge"}, {"-", "hoge"}},
			.expects_ = {
				{"hogi", 1},
			},
			.search_ = {
				.found = {"hogi"}, 
				.not_found = {"hoge", "hog", "e", "i"},
			},
		},
		{
			.message_ = "insert which includes previous element",
			.operations_ = {{"+", "abc"}, {"+", "abd"}, {"+", "abde"}},
			.expects_ = {
				{"ab", 1},
					{"c", 2},
					{"d", 2},
						{"", 3},
						{"e", 3},
			},
			.search_ = {
				.found = {"abc", "abd", "abde"}, 
			},
		},
		{
			.message_ = "insert which is included by previous element",
			.operations_ = {{"+", "abc"}, {"+", "abde"}, {"+", "abd"}},
			.expects_ = {
				{"ab", 1},
					{"c", 2},
					{"d", 2},
						{"", 3},
						{"e", 3},
			},
			.search_ = {
				.found = {"abc", "abd", "abde"}, 
			},
		},
		/*
		{
			.message_ = "utf8 insert",
			.operations_ = {{"+", "あたま"}, {"+", "あたれ"}, {"+", "あたれば"}},
			.expects_ = {
				{"あた", 1},
					{"ま", 2},
					{"れ", 2},
						{"", 3},
						{"ば", 3},
			},
			.search_ = {
				.found = {"あたま", "あたれ", "あたれば"}, 
			},
			.matcher_ = new shutup::UTF8Matcher(),
		},
		{
			.message_ = "utf8 insert + remove cause merge",
			.operations_ = {{"+", "あたま"}, {"+", "あたる"}, {"-", "あたる"}},
			.expects_ = {
				{"あたま", 1},
			},
			.search_ = {
				.found = {"あたま"}, 
				.not_found = {"あたる"},
			},
			.matcher_ = new shutup::UTF8Matcher(),
		},
		*/
	};
	for (auto &c : cases) {
		std::printf("patricia_test %s\n", c.message_);
		auto result = c.test();
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

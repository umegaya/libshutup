#include <cstdio>
#include "patricia.h"

namespace test {
namespace patricia {
class IgnoreSpaceMatcher : public shutup::UTF8Matcher {
	static bool isspace(shutup::u8 p) {
		//std::printf("isspace: %02x %02x %02x\n", p, static_cast<shutup::u8>(' '), static_cast<shutup::u8>('\t'));
		return p == static_cast<shutup::u8>(' ') || p == static_cast<shutup::u8>('\t');
	}
	int match(const shutup::u8 *in, int ilen, const shutup::u8 *pattern, int plen, int *ofs) {
		int i = 0, p = 0;
		while (i < std::min(ilen, plen)) {
			int tmp = shutup::utf8::peek(in + i, ilen - i);
			//std::printf("peek: %d %d %d\n", tmp, i, plen);
			if (tmp <= 0) {
				break;
			}
			//std::printf("isspace: %d %s\n", tmp, isspace(in[i]) ? "space" : "non-space");
			if (tmp == 1 && isspace(in[i])) {
				//std::printf("space and tab are ignored\n");
				i += tmp;
			} else if (memcmp(in + i, pattern + p, tmp) == 0) {
				i += tmp;
				p += tmp;
			} else {
				break;
			}
		}
		*ofs = i;
		return p;
	}
};

struct testcase {
	struct operation {
		const char *type;
		const char *data;
		void *param;
	};
	struct expect {
		const char *data;
		int depth;
		void *param;
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
			//std::printf("---- %s%s\n", o.type, o.data);
			if (std::strcmp(o.type, "+") == 0) { 
				p.add(o.data, o.param); 
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
				e.param == n->param() && 
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
			//std::printf("found -- %s\n", s);
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
		//p.dump();
		return nullptr;		
	}
};
}
}

static void *p(int id) {
	return reinterpret_cast<void *>(id);
}

extern const char *patricia_test() {
	std::vector<test::patricia::testcase> cases{
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
			.operations_ = {{"+", "goge", p(1)}, {"+", "hoge", p(2)}},
			.expects_ = {
				{"goge", 1, p(1)},
				{"hoge", 1, p(2)},
			},
			.search_ = {
				.found = {"goge", "hoge"},
			},
		},
		{
			.message_ = "insert cause split",
			.operations_ = {{"+", "hogi", p(1)}, {"+", "hoge", p(2)}},
			.expects_ = {
				{"hog", 1},
					{"e", 2, p(2)},
					{"i", 2, p(1)},
			},
			.search_ = {
				.found = {"hoge", "hogi"}, 
				.not_found = {"hog", "e", "i"},
			},
		},
		{
			.message_ = "insert + remove",
			.operations_ = {{"+", "abcdef", p(1)}, {"-", "abcdef"}},
			.expects_ = {},
			.search_ = {
				.not_found = {"abcdef"},
			},
		},
		{
			.message_ = "insert + remove not exists",
			.operations_ = {{"+", "abcdef", p(1)}, {"-", "abcdee"}},
			.expects_ = {
				{"abcdef", 1, p(1)}
			},
			.search_ = {
				.found = {"abcdef"},
				.not_found = {"abcdee"},
			},
		},
		{
			.message_ = "insert + remove not exists (no terminate)",
			.operations_ = {{"+", "hogi", p(1)}, {"+", "hoge", p(2)}, {"-", "hog"}},
			.expects_ = {
				{"hog", 1},
					{"e", 2, p(2)},
					{"i", 2, p(1)},
			},
			.search_ = {
				.found = {"hoge", "hogi"}, 
				.not_found = {"hog", "e", "i"},
			},
		},
		{
			.message_ = "insert + remove cause merge",
			.operations_ = {{"+", "hogi", p(1)}, {"+", "hoge", p(2)}, {"-", "hoge"}},
			.expects_ = {
				{"hogi", 1, p(1)},
			},
			.search_ = {
				.found = {"hogi"}, 	
				.not_found = {"hoge", "hog", "e", "i"},
			},
		},
		{
			.message_ = "insert which includes previous element",
			.operations_ = {{"+", "abc", p(1)}, {"+", "abd", p(2)}, {"+", "abde", p(3)}},
			.expects_ = {
				{"ab", 1},
					{"c", 2, p(1)},
					{"d", 2, p(2)},
						{"e", 3, p(3)},
			},
			.search_ = {
				.found = {"abc", "abd", "abde"}, 
			},
		},
		{
			.message_ = "insert which is included by previous element",
			.operations_ = {{"+", "abc", p(1)}, {"+", "abde", p(2)}, {"+", "abd", p(3)}},
			.expects_ = {
				{"ab", 1},
					{"c", 2, p(1)},
					{"d", 2, p(3)},
						{"e", 3, p(2)},
			},
			.search_ = {
				.found = {"abc", "abd", "abde"}, 
			},
		},
		{
			.message_ = "remove causes merge node which has children",
			.operations_ = {{"+", "ab", p(1)}, {"+", "ac", p(2)}, {"+", "abc", p(3)}, {"+", "abd", p(4)}, {"-", "ac"}},
			.expects_ = {
				{"ab", 1, p(1)},
					{"c", 2, p(3)},
					{"d", 2, p(4)},
			},
			.search_ = {
				.found = {"ab", "abc", "abd"}, 
			},
		},
		{
			.message_ = "remove causes split node which has children",
			.operations_ = {{"+", "ab", p(1)}, {"+", "abc", p(2)}, {"+", "abd", p(3)}, {"+", "ac", p(4)}, {"-", "ac"}},
			.expects_ = {
				{"ab", 1, p(1)},
					{"c", 2, p(2)},
					{"d", 2, p(3)},
			},
			.search_ = {
				.found = {"ab", "abc", "abd"}, 
			},
		},
		{
			.message_ = "remove last node in children",
			.operations_ = {{"+", "abc", p(1)}, {"+", "ab", p(2)}, {"+", "a", p(3)}, {"-", "abc"}},
			.expects_ = {
				{"a", 1, p(3)},
					{"b", 2, p(2)},
			},
			.search_ = {
				.found = {"a", "ab"}, 
				.not_found = {"abc"},
			},			
		},
		{
			.message_ = "insert cause make intermediate terminate node (hog)",
			.operations_ = {{"+", "hoge", p(1)}, {"+", "hogi", p(2)}, {"+", "hog", p(3)}},
			.expects_ = {
				{"hog", 1, p(3)},
					{"e", 2, p(1)},
					{"i", 2, p(2)},
			},
			.search_ = {
				.found = {"hog", "hoge", "hogi"}, 
			},
		},
		{
			.message_ = "insert + remove cause remove intermediate terminate node (hog)",
			.operations_ = {{"+", "hoge", p(1)}, {"+", "hogi", p(2)}, {"+", "hog", p(3)}, {"-", "hog"}},
			.expects_ = {
				{"hog", 1},
					{"e", 2, p(1)},
					{"i", 2, p(2)},
			},
			.search_ = {
				.found = {"hoge", "hogi"}, 
			},
		},
		{
			.message_ = "insert + remove cause merge its children",
			.operations_ = {{"+", "ab", p(1)}, {"+", "abc", p(2)}, {"+", "abcd", p(3)}, {"-", "abc"}},
			.expects_ = {
				{"ab", 1, p(1)},
					{"cd", 2, p(3)},
			},
			.search_ = {
				.found = {"ab", "abcd"},
				.not_found = {"abc"}, 
			},
		},
		{
			.message_ = "utf8 insert",
			.operations_ = {{"+", "あたま", p(1)}, {"+", "あたれ", p(2)}, {"+", "あたれば", p(3)}},
			.expects_ = {
				{"あた", 1},
					{"ま", 2, p(1)},
					{"れ", 2, p(2)},
						{"ば", 3, p(3)},
			},
			.search_ = {
				.found = {"あたま", "あたれ", "あたれば"}, 
			},
			.matcher_ = new shutup::UTF8Matcher(),
		},
		{
			.message_ = "utf8 insert + remove cause merge",
			.operations_ = {{"+", "あたま", p(1)}, {"+", "あたる", p(2)}, {"-", "あたる", p(3)}},
			.expects_ = {
				{"あたま", 1, p(1)},
			},
			.search_ = {
				.found = {"あたま"}, 
				.not_found = {"あたる"},
			},
			.matcher_ = new shutup::UTF8Matcher(),
		},
		{
			.message_ = "ignore space contains",
			.operations_ = {{"+", "あたま", p(1)}, {"+", "あたる", p(2)}},
			.expects_ = {
				{"あた", 1},
					{"ま", 2, p(1)},
					{"る", 2, p(2)},
			},
			.search_ = {
				.found = {"あたま", "あたる", "あ た ま", "あ	た	る", "あた	る"}, 
				.not_found = {"あ た 	れ", "あ	たれ"},
			},
			.matcher_ = new test::patricia::IgnoreSpaceMatcher(),
		},
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

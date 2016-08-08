#include "checker.h"
#include "language.h"

namespace test {
namespace alloc {
//test custom allocator
class MyAllocatorContainer {
	static std::map<unsigned long long, size_t> s_alloc_map_;
	static shutup::Checker::Allocator s_allocator_;
public:
	static void *malloc(size_t sz) {
		void *p = std::malloc(sz);
		unsigned long long key = (unsigned long long)p;
		s_alloc_map_[key] = sz;
		//TRACE("malloc: %lu, returns %p\n", sz, p);
		return p;
	}
	static void free(void *p) {
		unsigned long long key = (unsigned long long)p;
		auto it = s_alloc_map_.find(key);
		size_t sz = 0;
		if (it != s_alloc_map_.end()) {
			sz = (*it).second;
			s_alloc_map_.erase(it);
			//TRACE("free: %lu, %p\n", sz, p);
		} else {
			//TRACE("error: %p not found\n", p);
		}
		std::free(p);
	}
	static void *realloc(void *p, size_t sz) {
		//TRACE("realloc: %lu, returns %p\n", sz, p);
		void *q = std::realloc(p, sz);
		unsigned long long key = (unsigned long long)p;
		auto it = s_alloc_map_.find(key);
		if (it != s_alloc_map_.end()) {
			s_alloc_map_.erase(it);
		}
		unsigned long long key2 = (unsigned long long)q;
		s_alloc_map_[key2] = sz;
		return q;		
	}
	static shutup::Checker::Allocator *allocator() {
		s_allocator_.malloc = MyAllocatorContainer::malloc;
		s_allocator_.free = MyAllocatorContainer::free;
		s_allocator_.realloc = MyAllocatorContainer::realloc;
		return &s_allocator_;
	}
	static bool check() {
		return s_alloc_map_.size() == 0;
	}
	static void dump() {
		for (auto p : s_alloc_map_) {
			std::printf("alloc:%p:%lu\n", (void *)p.first, p.second);
		}
	}
};
std::map<unsigned long long, size_t> MyAllocatorContainer::s_alloc_map_;
shutup::Checker::Allocator MyAllocatorContainer::s_allocator_;

//test custom checker
typedef shutup::language::WordChecker SuperChecker;
class MyChecker : public SuperChecker {
	char *ptr_;
public:
	MyChecker(shutup::IMempool *m) : SuperChecker(m), ptr_(nullptr) {}
	int init() {
		ptr_ = new(pool().malloc(11)) char[11];
		for (int i = 0; i < 10; i++) {
			ptr_[i] = '0' + i;
		}
		ptr_[10] = 0;
		ignore_glyphs(ptr_);
		return 0;
	}
	void fin() {
		if (ptr_ != nullptr) {
			pool().free(ptr_);
			ptr_ = nullptr;
		}
		SuperChecker::fin();
	}
};

//
class CheckerUser {
	struct filter {
		const char *word;
		void *ptr;
	};
	static std::vector<filter> s_filters;
protected:
	shutup::Checker c_;
public:
	CheckerUser(const char *lang) : c_(lang, MyAllocatorContainer::allocator()) {}
	CheckerUser(std::function<shutup::language::WordChecker*(shutup::Checker::Mempool&)> f) : c_(MyAllocatorContainer::allocator(), f) {}
	static const char *use_checker(shutup::Checker &c) {
		for (auto &e : s_filters) {
			c.add(e.word, e.ptr);
		}
		int start, count;
		void *ctx;
		for (auto &e : s_filters) {
			if (std::strlen(e.word) <= 0) { continue; }
			if (!c.should_filter(e.word, std::strlen(e.word), &start, &count, &ctx) || ctx != e.ptr) {
				TRACE("w:%s\n", e.word);
				return "filter error";
			}
		}
		return nullptr;
	}
	static void *ptr(int id) {
		return reinterpret_cast<void *>(id);
	}
	const char *use() {
		return use_checker(c_);
	}
};
std::vector<CheckerUser::filter> CheckerUser::s_filters = {
	{"badword", CheckerUser::ptr(1)},
	{"", CheckerUser::ptr(2)},
	{"馬津怒輪亜怒", CheckerUser::ptr(3)},
	{"バッドワード", CheckerUser::ptr(4)},
	{"ワンド", CheckerUser::ptr(5)},
	{"ソビエト", CheckerUser::ptr(6)},
	{"ぢょく", CheckerUser::ptr(7)}
};
class CheckerUserNormal : public CheckerUser {
public:
	CheckerUserNormal() : CheckerUser("jp") {}		
};
class CheckerUserCustom : public CheckerUser {
public:
	CheckerUserCustom() : CheckerUser(&shutup::Checker::new_word_checker<MyChecker>) {}
};

//test cases
struct testcase {
	//test explanation
	const char *message_;
	const char *(*proc_)();
	const char *test() {
		const char *msg = proc_();
		if (msg != nullptr) {
			return msg;
		}
		if (!MyAllocatorContainer::check()) {
			MyAllocatorContainer::dump();
			return "memory leak";
		}
		return nullptr;
	}
};
static const char *on_stack() {
	shutup::Checker c("jp", MyAllocatorContainer::allocator());
	return CheckerUser::use_checker(c);
}
static const char *in_class() {
	CheckerUserNormal u;
	return u.use();
}
static const char *on_stack_custom() {
	shutup::Checker c(MyAllocatorContainer::allocator(), &shutup::Checker::new_word_checker<MyChecker>);
	return CheckerUser::use_checker(c);
}
static const char *in_class_custom() {
	CheckerUserCustom u;
	return u.use();
}
static const char *all() {
	shutup::Checker *pc = shutup::Checker::create("jp", MyAllocatorContainer::allocator());
	const char *msg = CheckerUser::use_checker(*pc);
	shutup::Checker::destroy(pc);
	return msg;
}
static const char *all_custom() {
	shutup::Checker *pc = shutup::Checker::create<MyChecker>(MyAllocatorContainer::allocator());
	const char *msg = CheckerUser::use_checker(*pc);
	shutup::Checker::destroy(pc);
	return msg;
}
}
}

extern const char *alloc_test() {
	std::vector<test::alloc::testcase> cases{
		{
			.message_ = "use allocator internally, custom checker, on stack",
			.proc_ = test::alloc::on_stack_custom,
		},
		{
			.message_ = "use allocator internally, custom checker, as class member",
			.proc_ = test::alloc::in_class_custom,
		},
		{
			.message_ = "use allocator internally, on stack",
			.proc_ = test::alloc::on_stack,
		},
		{
			.message_ = "use allocator internally, as class member",
			.proc_ = test::alloc::in_class,
		},
		{
			.message_ = "use allocator all",
			.proc_ = test::alloc::all,
		},
		{
			.message_ = "use allocator all, custom checker",
			.proc_ = test::alloc::all_custom,
		},
	};
	for (auto &c : cases) {
		std::printf("alloc_test %s\n", c.message_);
		auto result = c.test();
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

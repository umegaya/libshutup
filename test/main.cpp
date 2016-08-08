#include <cstdio>
#include <cstring>

extern const char *patricia_test();
extern const char *util_test();
extern const char *language_test();
extern const char *checker_test();
extern const char *alloc_test();

int main(int argc, char *argv[]) {
	struct {
		const char *name;
		const char *(*proc)();
	} tests[] = {
		{"patricia", patricia_test},
		{"util", util_test},
		{"language", language_test},
		{"checker", checker_test},
		{"alloc", alloc_test},
	};
	for (int i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
		if (argc <= 1 || std::strcmp(argv[1], tests[i].name) == 0) {
			const char *err = tests[i].proc();
			if (err != nullptr) {
				std::printf("test error: %s\n", err);
				return 1;
			}
		}
	}
	std::printf("test success\n");
	return 0;
}

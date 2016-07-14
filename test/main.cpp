#include <cstdio>
#include <cstring>

extern const char *patricia_test();

int main(int argc, char *argv[]) {
	struct {
		const char *name;
		const char *(*proc)();
	} tests[] = {
		"patricia", patricia_test,
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

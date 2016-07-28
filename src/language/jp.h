#pragma once

#include "language.h"

namespace shutup {
namespace language {
class JP : public WordChecker {
public:
	JP(IMempool *p) : WordChecker(p) {}
	virtual ~JP() {}
	int init();
	void add_synonym(const char *pattern, Checker &c, void *ctx);
	normalizer *normalizers(int *n_norm);
};
}
}

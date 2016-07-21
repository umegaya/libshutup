#pragma once

#include "language.h"

namespace shutup {
namespace language {
class JP : public WordChecker {
public:
	JP(IMempool *p) : WordChecker(p) {}
	virtual ~JP() {}
	int init();
	void add_synonym(const char *pattern, Checker &c);
	normalizer *normalizers(int *n_norm);
	const char *to_hebon_roman(const char *str);
	const char *to_japan_roman(const char *str);
};
}
}

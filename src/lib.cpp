#include "lib.h"

extern "C" {
	extern shutup *shutup_new(const char *matcher, shutup_allocator *a) {
		return NULL;
	}
	extern shutup *shutup_setup_alias(shutup *s, const char ***alias_groups) {
		return s;
	}
	extern shutup *shutup_setup_mask(shutup *s, const char *mask) {
		return s;
	}
	extern shutup *shutup_setup_word(shutup *s, const char *word) {
		return s;
	}
	extern const char *shutup_should_block(shutup *s, const char *text, const shutup_block_config *cfg) {
		return NULL;
	}
	const char *shutup_block(shutup *s, const char *text, const shutup_block_config *cfg) {
		return text;
	}
}

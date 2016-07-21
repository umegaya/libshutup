#include "lib.h"

extern "C" {
	extern shutter *shutup_new(const char *matcher, shutup_allocator *a) {
		return NULL;
	}
	extern shutter *shutup_setup_alias(shutter *s, const char ***alias_groups) {
		return s;
	}
	extern shutter *shutup_setup_mask(shutter *s, const char *mask) {
		return s;
	}
	extern shutter *shutup_setup_word(shutter *s, const char *word) {
		return s;
	}
	extern const char *shutup_should_block(shutter *s, const char *text, const shutup_block_config *cfg) {
		return NULL;
	}
	const char *shutup_block(shutter *s, const char *text, const shutup_block_config *cfg) {
		return text;
	}
}

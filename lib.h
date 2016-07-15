#pragma once

#include <stdio.h>

extern "C" {
	typedef void *shutup;
	typedef struct shutup_allocator {
		void *(*malloc)(size_t);
		void (*free)(void *);
		void *(*realloc)(void *, size_t);
	};
	typedef struct shutup_block_config {
		bool check_alias;
		bool full_combination;
	};
	extern shutup *shutup_new(const char *matcher, shutup_allocator *a);
	extern shutup *shutup_setup_alias(shutup *s, const char ***alias_groups);
	extern shutup *shutup_setup_mask(shutup *s, const char *mask);
	extern shutup *shutup_setup_word(shutup *s, const char *word);
	extern const char *shutup_should_block(shutup *s, const char *text, const shutup_block_config *cfg);
	const char *shutup_block(shutup *s, const char *text, const shutup_block_config *cfg);
}

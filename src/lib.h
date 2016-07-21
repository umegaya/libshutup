#pragma once

#include <stdio.h>

extern "C" {
	typedef void *shutter;
	typedef struct _shutup_allocator {
		void *(*malloc)(size_t);
		void (*free)(void *);
		void *(*realloc)(void *, size_t);
	} shutup_allocator;
	typedef struct _shutup_block_config {
		bool check_alias;
		bool full_combination;
	} shutup_block_config;
	extern shutter *shutup_new(const char *lang, shutup_allocator *a);
	extern shutter *shutup_setup_alias(shutter *s, const char ***alias_groups);
	extern shutter *shutup_setup_mask(shutter *s, const char *mask);
	extern shutter *shutup_setup_word(shutter *s, const char *word);
	extern const char *shutup_should_block(shutter *s, const char *text, const shutup_block_config *cfg);
	const char *shutup_block(shutter *s, const char *text, const shutup_block_config *cfg);
}

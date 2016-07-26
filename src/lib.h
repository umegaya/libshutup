#pragma once

#include <stdio.h>

extern "C" {
	typedef void *shutter;
	typedef struct _shutup_allocator {
		void *(*malloc)(size_t);
		void (*free)(void *);
		void *(*realloc)(void *, size_t);
	} shutup_allocator;
	extern shutter shutup_new(const char *lang, shutup_allocator *a);
	extern void shutup_set_alias(shutter s, const char *target, const char *alias);
	extern void shutup_ignore_glyphs(shutter s, const char *glyphs);
	extern void shutup_add_word(shutter s, const char *word);
	extern const char *shutup_should_filter(shutter s, const char *text, char *out, int *olen);
	extern const char *shutup_filter(shutter s, const char *text, char *out, int *olen, const char *mask);
}

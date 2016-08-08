#pragma once

#include <stdio.h>

extern "C" {
	typedef void *shutter;
	typedef struct _shutup_allocator {
		void *(*malloc)(size_t);
		void (*free)(void *);
		void *(*realloc)(void *, size_t);
	} shutup_allocator;
	typedef bool (*shutup_context_checker)(const char *in, int ilen, int start, int count, void *ctx);
	extern shutter shutup_new(const char *lang, shutup_allocator *a);
	extern void shutup_delete(shutter s);
	extern void shutup_set_alias(shutter s, const char *target, const char *alias);
	extern void shutup_ignore_glyphs(shutter s, const char *glyphs);
	extern void shutup_add_word(shutter s, const char *word, void *ctx);
	extern void *shutup_should_filter(shutter s, const char *in, int ilen, int *start, int *count, 
		shutup_context_checker checker);
	extern const char *shutup_filter(shutter s, const char *in, int ilen, char *out, int *olen, const char *mask, 
		shutup_context_checker checker);
	//for debugging
	typedef void (*shutup_logger)(const char *);
	extern void shutup_set_logger(shutup_logger logger);
	extern void shutup_log(const char *fmt, ...);
}

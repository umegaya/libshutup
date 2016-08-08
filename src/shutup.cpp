#include "shutup.h"
#include "checker.h"

extern "C" {
shutter shutup_new(const char *lang, shutup_allocator *a) {
	return a == nullptr ? 
		reinterpret_cast<void *>(new shutup::Checker(lang, a)) : 
		shutup::Checker::create(lang, a);
}
void shutup_delete(shutter s) {
	shutup::Checker::destroy(reinterpret_cast<shutup::Checker *>(s));
}
void shutup_set_alias(shutter s, const char *target, const char *alias) {
	shutup::Checker *c = reinterpret_cast<shutup::Checker *>(s);
	if (c->valid()) {
		c->add_alias(target, alias);
	}
}
void shutup_ignore_glyphs(shutter s, const char *glyphs) {
	shutup::Checker *c = reinterpret_cast<shutup::Checker *>(s);
	if (c->valid()) {
		c->ignore_glyphs(glyphs);
	}
}
void shutup_add_word(shutter s, const char *word, void *ctx) {
	shutup::Checker *c = reinterpret_cast<shutup::Checker *>(s);
	if (c->valid()) {
		c->add(word, ctx);
	}
}
void *shutup_should_filter(shutter s, const char *in, int ilen, int *start, int *count, 
	shutup_context_checker checker) {
	void *p;
	shutup::Checker *c = reinterpret_cast<shutup::Checker *>(s);
	if (!c->valid()) {
		*start = -1;
		return nullptr;
	}
	if (c->should_filter(in, ilen, start, count, &p, checker)) {
		//shutup_log("should filter: length: %d [%s]\n", *olen, out);
		return p;
	}
	return nullptr;
}
static char *s_buff = nullptr;
static int s_buff_size = 0;
static char *allocate(int *size) {
	if (s_buff_size == 0) {
		s_buff_size = 1;
	}
	while (s_buff_size < *size) {
		s_buff_size *= 2;
	}
	s_buff = reinterpret_cast<char *>(std::realloc(s_buff, s_buff_size));
	*size = s_buff_size;
	return s_buff;
}
const char *shutup_filter(shutter s, const char *in, int ilen, char *out, int *olen, const char *mask, 
	shutup_context_checker checker) {
	int tmp;
	shutup::Checker *c = reinterpret_cast<shutup::Checker *>(s);
	if (!c->valid()) {
		*olen = -1;
		return nullptr;
	}
	if (out == nullptr) {
		olen = &tmp;
		tmp = ilen;
		out = allocate(olen);
		if (out == nullptr) {
			return nullptr;
		}
	}
	if (c->filter(in, ilen, out, olen, mask, checker) != nullptr) {
		out[*olen] = 0;
		return reinterpret_cast<const char *>(out);
	}
	return nullptr;
}
static shutup_logger s_logger = nullptr;
void shutup_set_logger(shutup_logger logger) {
	s_logger = logger;
}
void shutup_log(const char *fmt, ...) {
	if (s_logger == nullptr) {
		return;
	}
	char buff[1024];
	va_list v;
    va_start(v, fmt);
    vsnprintf(buff, sizeof(buff), fmt, v);
    s_logger(buff);
}
}

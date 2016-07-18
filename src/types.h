#pragma once

namespace shutup {
	typedef signed char s8;
	typedef unsigned char u8;
	typedef signed short s16;
	typedef unsigned short u16;
	typedef signed int s32;
	typedef unsigned int u32;
#ifdef __LP64__
    typedef signed   long s64;
    typedef unsigned long u64;
#else
	typedef signed long long s64;
	typedef unsigned long long u64;
#endif
}

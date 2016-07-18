#pragma once

#if defined(DEBUG)
#include <cstdio>
#endif

namespace shutup {
class IMatcher {
public:
	virtual ~IMatcher() {}
	//返り値：in, ilenのスライスがpattern, plenのスライスの*どこまで*マッチしたかを返す.
	//*ofsには、マッチした場合にin, ilenはどこまで読まれたか、を返す.
	//例えば、in, ilenに含まれる特定のbyteを無視するなどしても良いため、別に返すことになっている.
	//eg) |を無視する場合、 a|a|aはaaaとマッチする。この場合、返り値は3, *ofs = 5となる.
	virtual int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) = 0;
};
class ByteMatcher : public IMatcher {
public:
	int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) {
		int i = 0;
		while (i < std::min(ilen, plen)) {
			if (in[i] != pattern[i]) { break; }
			i++;
		}
		*ofs = i;
		return i;
	}
};
class UTF8Matcher : public IMatcher {
public:
	static inline int utf8peek(const u8 *in, int ilen) {
		int mask = 0x80, count = 0, bit = in[0];
		while(bit != 0x00) {
			if ((bit & mask) == 0) {
				break;
			}
			count++;
			mask >>= 1;
		}
		/*#if defined(DEBUG)
		char buffer[256];
		std::strncpy(buffer, reinterpret_cast<const char *>(in), ilen + 1);
		std::printf("utf8peek: %p %s: %d\n", in, buffer, count);
		#endif*/
		return count <= 0 ? 1 : count;
	}
	int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) {
		int i = 0;
		while (i < std::min(ilen, plen)) {
			int tmp = utf8peek(in + i, ilen - i);
			if (tmp <= 0 || plen < (i + tmp)) {
				break;
			}
			if (memcmp(in + i, pattern + i, tmp) != 0) {
				break;
			}
			i += tmp;
		}
		*ofs = i;
		return i;
	}
};
}

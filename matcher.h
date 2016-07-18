#pragma once

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
	int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) {
		*ofs = plen;
		return plen <= ilen && memcmp(in, pattern, plen) == 0;
	}
};
}

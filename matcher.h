#pragma once

namespace shutup {
class imatcher {
public:
	virtual ~imatcher() {}
	virtual bool match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) = 0;
};
class byte_matcher : public imatcher {
	bool match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs) {
		*ofs = plen;
		return plen <= ilen && memcmp(in, pattern, plen) == 0;
	}
};
}


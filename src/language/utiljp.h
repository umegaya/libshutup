//以下は同じ文字のオフセット位置と５０音表内での位置をちゃんと揃えること.
const char *utf8::jp::hiras = 
"あいうえお"
"かきくけこ"
"さしすせそ"
"たちつてと"
"なにぬねの"
"はひふへほ"
"まみむめも"
"や　ゆ　よ"
"らりるれろ"
"わゐ　ゑを"
"がぎぐげご"
"ざじずぜぞ"
"だぢづでど"
"ばびぶべぼ"
"ぱぴぷぺぽ"
"　　ゔ　　"
"ぁぃぅぇぉ"
"ゃ　ゅ　ょ"
"んーっ";

const char *utf8::jp::katas = 
"アイウエオ"
"カキクケコ"
"サシスセソ"
"タチツテト"
"ナニヌネノ"
"ハヒフヘホ"
"マミムメモ"
"ヤ　ユ　ヨ"
"ラリルレロ"
"ワヰ　ヱヲ"
"ガギグゲゴ"
"ザジズゼゾ"
"ダヂヅデド"
"バビブベボ"
"パピプペポ"
"　　ヴ　　"
"ァィゥェォ"
"ャ　ュ　ョ"
"ンーッ";

const char *utf8::jp::half_katas = 
"ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ";

const char *utf8::jp::wide_katas = 
"ヲァィゥェォャュョッーアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワン";

const char *utf8::jp::wide_psign_katas = 
"パピプペポ";

const char *utf8::jp::wide_sonant_katas = 
"ガギグゲゴザジズゼゾダヂヅデド　　　　　バビブベボ";

enum Vowel {
	A,
	I,
	U,
	E,
	O,
};

enum Consonant {
	__A,
	K,
	S,
	T,
	N,
	H,
	M,
	Y,
	R,
	W,
	G,
	Z,
	D,
	B,
	P,
	V,

	SMALL_VOWELS,
	SMALL_Y,
	NN,

	Y_I_CONTRACTED_SOUND_START,
	KY,
	SY,
	TY,
	NY,
	HY,
	MY,
	__YI,
	RY,
	__WI,
	__VI,

	GY,
	J,
	DY,
	BY,
	PY,

	Y_U_CONTRACTED_SOUND_START,
	__KU,
	__SU,
	__TU,
	__NU,
	FY,
	__MU,
	__YU,
	__RU,
	__WU,
	VY,

	__GU,
	__JU,
	__DU,
	__BU,
	__PU,

	Y_E_CONTRACTED_SOUND_START,
	__KJ,
	__SJ,
	TJ,
	__NJ,
	__FJ,
	__MJ,
	__YJ,
	__RJ,
	__WJ,
	__VJ,

	__GJ,
	__JJ,
	DJ,
	__BJ,
	__PJ,	

	V_U_CONTRACTED_SOUND_START,
	KW,
	SW,
	TS,
	NW,
	F,
	MW,
	__YW,
	RW,
	__WW,
	V_,

	GW,
	ZW,
	DZ,
	BW,
	PW,

	V_O_CONTRACTED_SOUND_START,
	__KO,
	__SO,
	TW,
	__NO,
	HW,
	__MO,
	__YO,
	__RO,
	__WO,
	__VO,

	__GO,
	__ZO,
	DW,
	__BO,
	__PO,
};

const char *utf8::jp::vowels[] =
{"a", "i", "u", "e", "o"};

#define EMP nullptr
const char *utf8::jp::hebon_consonants[] =
{
	EMP, "k",  "s",  "t",  "n",  "h",  "m",  "y", "r",  "w", "g",  "z",  "d",  "b",  "p",  "v",  "x", "xy", EMP,
	EMP, "ky", "sh", "ch", "ny", "hy", "my", EMP, "ry", EMP, "gy", "j",  "dy", "by", "py", EMP,  //Y_I_
	EMP, EMP,  EMP,  EMP,  EMP,  "fy", EMP,  EMP, EMP,  EMP, EMP,  EMP,  EMP,  EMP,  EMP,  "vy", //Y_U_
	EMP, EMP,  EMP,  "tj", EMP,  EMP,  EMP,  EMP, EMP,  EMP, EMP,  EMP,  "dj", EMP,  EMP,  EMP,  //Y_E_
	EMP, "kw", "sw", "ts", "nw", "f",  "mw", EMP, "rw", EMP, "gw", "zw", "dz", "bw", "pw", "v",  //V_U_
	EMP, EMP,  EMP,  "tw", EMP,  "hw", EMP,  EMP, EMP,  EMP, EMP,  EMP,  "dw", EMP,  EMP,  EMP,  //V_O_
};

const char *utf8::jp::japan_consonants[] =
{
	EMP, "k",  "s",  "t",  "n",  "h",  "m",  "y", "r",  "w", "g",  "z",  "d",  "b",  "p",  "v",  "x", "xy", EMP,
	EMP, "ky", "sh", "ty", "ny", "hy", "my", EMP, "ry", EMP, "gy", "j",  "dy", "by", "py", EMP,  //Y_I_
	EMP, EMP,  EMP,  EMP,  EMP,  "fy", EMP,  EMP, EMP,  EMP, EMP,  EMP,  EMP,  EMP,  EMP,  "vy", //Y_U_
	EMP, EMP,  EMP,  "tj", EMP,  EMP,  EMP,  EMP, EMP,  EMP, EMP,  EMP,  "dj", EMP,  EMP,  EMP,  //Y_E_
	EMP, "kw", "sw", "ts", "nw", "f",  "mw", EMP, "rw", EMP, "gw", "zw", "dz", "bw", "pw", "v",  //V_U_
	EMP, EMP,  EMP,  "tw", EMP,  "hw", EMP,  EMP, EMP,  EMP, EMP,  EMP,  "dw", EMP,  EMP,  EMP,  //V_O_
};

bool utf8::jp::is_kana_string(const char *str) {
	size_t len = std::strlen(str);
	int idx = 0;
	while (len > idx) {
		int tmp = peek(reinterpret_cast<const u8 *>(str + idx), len - idx);
		if (tmp == 0) {
			break;
		}
		char glyph[tmp + 1];
		std::memcpy(glyph, str + idx, tmp); glyph[tmp] = 0;
		if (nullptr == std::strstr(utf8::jp::hiras, glyph) && nullptr == std::strstr(utf8::jp::katas, glyph)) {
			return false;
		}
		idx += tmp;
	}
	return true;
}

static inline int kana_index(const u8 *in, int ilen, int *olen) {
	u8 buff[utf8::MAX_BYTE_PER_GRYPH + 1];
	int r = utf8::copy(in, ilen, buff, sizeof(buff), 1);
	if (r <= 0) {
		*olen = 0;
		return -1;
	}
	buff[r] = 0;
	const char *p = std::strstr(utf8::jp::katas, reinterpret_cast<const char *>(buff));
	if (p != nullptr) {
		*olen = r;
		return (p - utf8::jp::katas) / 3;
	} else {
		p = std::strstr(utf8::jp::hiras, reinterpret_cast<const char *>(buff));
		if (p != nullptr) {
			//TRACE("kana_index: match hira: %s %lu\n", p, p - utf8::jp::hiras);
			*olen = r;
			return (p - utf8::jp::hiras) / 3;
		}
	}
	//TRACE("kana_index error no match: %s\n", in);
	return -1; //no hira or kata kana.
}

int utf8::jp::to_roman(const u8 *in, int ilen, u8 *out, int *olen, bool assimilated, bool strict_mode,
	const char *consonants[], const char *(*consonant_exception)(const char *, int, int, bool)) {
	int tmp, idx = kana_index(in, ilen, &tmp);
	if (idx < 0) {
		TRACE("to_roman kana_index error: %s\n", in);
		*olen = 0;
		return 0;
	}
	int vowel_index = idx % 5, consonant_index = idx / 5;
	//TRACE("to_roman: %s %d %d\n", in, vowel_index, consonant_index);
	if (consonant_index == 0) {
		if (*olen < 1) {
			TRACE("to_roman: buf short1\n");
			return -1;
		}
		std::memcpy(out, vowels[vowel_index], 1);
		*olen = 1;
		return tmp;
	} else if (vowel_index == Vowel::U && consonant_index == Consonant::NN) {
		//小さな「つ」
		if (ilen > tmp) {
			//次の読み出しを行う.
			return tmp + to_roman(in + tmp, ilen - tmp, out, olen, true, strict_mode, consonants, consonant_exception);
		} else {
			//もうバッファがないのでから文字列を返す.
			*olen = 0;
			return tmp;
		}
	} else if (vowel_index == Vowel::I && consonant_index == Consonant::NN) {
		//長音記号.無視される.
		*olen = 0;
		return tmp;
	} else if (vowel_index == Vowel::A && consonant_index == Consonant::NN) {
		//「ん」
		if (*olen < 1) {
			TRACE("to_roman: buf short2\n");
			return -1;
		}
		int ntmp, nidx = kana_index(in + tmp, ilen - tmp, &ntmp);
		if (nidx >= 0) {
			int next_consonant_index = nidx / 5;
			//b,p,mが次の子音の時のみ、m
			if (next_consonant_index == Consonant::B || next_consonant_index == Consonant::P || next_consonant_index == Consonant::M) {
				std::memcpy(out, "m", 1);
				*olen = 1;
				return tmp;
			} else if (strict_mode && (next_consonant_index == Consonant::__A || next_consonant_index == Consonant::Y)) {
				//撥音の後に母音やヤ行音が来てナ行音と区別できなくなった場合は、間に「’」（アポストロフィ）を挿入する。らしいが,
				//禁止語句の入力として考えた場合、'を入力するユーザーなんているわけないのでstrict_modeの時だけ.
				if (*olen < 2) {
					TRACE("to_roman: buf short2\n");
					return -1;
				}
				std::memcpy(out, "n'", 2);
				*olen = 2;
				return tmp;
			}
		}
		std::memcpy(out, "n", 1);
		*olen = 1;
		return tmp;
	} else {
		//TRACE("check combine with next char %d %d %s\n", vowel_index, consonant_index, assimilated ? "asim" : "no-asim");
		//次の１文字とくっつくケースか調べる. (99式ベース:http://green.adam.ne.jp/roomazi/kyuukyuusiki.html)
		//今の文字のvowelとconsonantによって、[ぁぃぅぇぉゃゅょ]のどの文字と結合するか決まる.（上記のhebon_consonantsのEMPではない場所が該当する)
		int ntmp, nidx = kana_index(in + tmp, ilen - tmp, &ntmp);
		//TRACE("next kana_index result %d %d\n", nidx, ntmp);
		if (nidx >= 0) {
			int next_vowel_index = nidx % 5, next_consonant_index = nidx / 5, consonant_offset = 0;
			switch (vowel_index) {
			case Vowel::A:
				break;
			case Vowel::I:
				//Y_I_CONTRACTED_SOUND_START: ゃぃゅぇょに結合する.
				if (next_consonant_index == Consonant::SMALL_Y || 
					(next_consonant_index == Consonant::SMALL_VOWELS && (next_vowel_index % 2) == 1)) {
					consonant_offset = Consonant::Y_I_CONTRACTED_SOUND_START;
				}
				break;
			case Vowel::U:
				//V_U_CONTRACTED_SOUND_START: ぁぃぅぇぉに結合する.
				if (next_consonant_index == Consonant::SMALL_VOWELS) {
					//ゔはぅには結合しない.
					if (consonant_index == Consonant::V && vowel_index == Vowel::U && next_vowel_index == Vowel::U) {
					} else {
						consonant_offset = Consonant::V_U_CONTRACTED_SOUND_START;
					}
				} else if ((consonant_index == Consonant::V || consonant_index == Consonant::H) && 
					next_consonant_index == Consonant::SMALL_Y) {
					//V行とH行ならゃゅょにも結合する.
					consonant_offset = Consonant::Y_U_CONTRACTED_SOUND_START;
				}
				break;
			case Vowel::E:
				//Y_E_CONTRACTED_SOUND_START: ゃぃゅぇょに結合する.
				if (next_consonant_index == Consonant::SMALL_Y || 
					(next_consonant_index == Consonant::SMALL_VOWELS && (next_vowel_index % 2) == 1)) {
					consonant_offset = Consonant::Y_E_CONTRACTED_SOUND_START;
				}
				break;
			case Vowel::O:
				//V_O_CONTRACTED_SOUND_START: ぁぃぅぇぉに結合する.
				if (next_consonant_index == Consonant::SMALL_VOWELS) {
					consonant_offset = Consonant::V_O_CONTRACTED_SOUND_START;
				}
				break;
			}
			//結合可能なconsonantが存在する.
			if (consonant_offset > 0 && consonants[consonant_index + consonant_offset] != nullptr) {
				consonant_index += consonant_offset;
				vowel_index = next_vowel_index;
				tmp = tmp + ntmp;
			}
		} else {
			//読み出すバッファがもうない.
		}
		const char *c = consonant_exception(consonants[consonant_index], vowel_index, consonant_index, assimilated);
		int clen = std::strlen(c);
		int total_len = 1 + clen;
		//TRACE("store last result %d %d %s %d %d %d\n", vowel_index, consonant_index, c, *olen, clen, total_len);
		if (*olen < total_len) {
			TRACE("to_roman: buf short3\n");
			return -1;
		}
		if (clen > 0) {
			std::memcpy(out, c, clen);
		}
		std::memcpy(out + clen, vowels[vowel_index], 1);
		*olen = total_len;
		//長音記号の無視.
		nidx = kana_index(in + tmp, ilen - tmp, &ntmp);
		if (nidx >= 0) {
			int next_vowel_index = nidx % 5, next_consonant_index = nidx / 5;
			if (next_vowel_index == Vowel::I && next_consonant_index == Consonant::NN) {
				tmp += ntmp;
			}
		}
		return tmp;
	}
	return 0;
}

const char *utf8::jp::hebon_consonant_exception(const char *normal, int vowel_index, int consonant_index, bool assimilated) {
	char *buff = utf8::work;
	int idx = assimilated ? 1 : 0;
	buff[idx] = 0;
	switch (vowel_index) {
	case Vowel::I:
		switch (consonant_index) {
		case Consonant::S:
			std::strcpy(buff + idx, "sh");
			break;
		case Consonant::T:
			std::strcpy(buff + idx, "ch");
			break;
		case Consonant::Z: case Consonant::D:
			std::strcpy(buff + idx, "j");
			break;
		}
		break;
	case Vowel::U:
		switch (consonant_index) {
		case Consonant::T:
			std::strcpy(buff + idx, "ts");
			break;
		case Consonant::D:
			std::strcpy(buff + idx, "z");
			break;
		}
	case Vowel::O:
		switch (consonant_index) {
		case Consonant::W:
			return buff; //empty consonant.
		}
	}
	if (buff[idx] == 0) {
		std::strcpy(buff + idx, normal);		
	}
	if (assimilated) {
		if (std::memcmp(buff + 1, "ch", 2) == 0) {
			buff[0] = 't';
		} else {
			buff[0] = buff[1];
		}
		return buff;
	} else {
		return buff;
	}
}

const char *utf8::jp::japan_consonant_exception(const char *normal, int vowel_index, int consonant_index, bool assimilated) {
	char *buff = utf8::work;
	int idx = assimilated ? 1 : 0;
	std::strcpy(buff + idx, normal);		
	if (assimilated) {
		buff[0] = buff[1];
		return buff;
	} else {
		return buff;
	}
}

int utf8::jp::to_hebon_roman(const u8 *in, int ilen, u8 *out, int *olen) {
	return to_roman(in, ilen, out, olen, false, false, hebon_consonants, hebon_consonant_exception);
}
int utf8::jp::to_japan_roman(const u8 *in, int ilen, u8 *out, int *olen) {
	return to_roman(in, ilen, out, olen, false, false, japan_consonants, japan_consonant_exception);
}
int utf8::jp::widen_kata(const u8 *in, int ilen, u8 *out, int *olen) {
	if (*olen < 3) {
		return -1; //buf short
	}
	u8 buff[utf8::MAX_BYTE_PER_GRYPH + 1];
	int r = copy(in, ilen, buff, sizeof(buff), 1);
	if (r <= 0) {
		*olen = 0;
		return 0;
	}
	buff[r] = 0;
	const char *p = std::strstr(half_katas, reinterpret_cast<char *>(buff));
	if (p != nullptr) {
		if (std::memcmp(p, "ﾞ", r) == 0 || std::memcmp(p, "ﾟ", r) == 0) {
			*olen = 0;
			return r;
		} else {
			size_t ofs = (p - half_katas), idx = ofs / 3;
			//ウ、カ〜ト、ハ〜ホ.
			//TRACE("ofs = %zu(%s)%d %d\n", ofs, buff, ilen, r);
			if (idx == 13 || (idx >= 16 && idx <= 31) || (idx >= 36 && idx <= 41)) {
				u8 buff2[utf8::MAX_BYTE_PER_GRYPH + 1];
				if (ilen > r) {
					int r2 = copy(in + r, ilen - r, buff2, sizeof(buff2), 1);
					buff2[r2] = 0;
					if (std::memcmp(buff2, "ﾞ", r2) == 0) {
						if (idx == 13) {
							std::memcpy(out, "ヴ", 3);
							*olen = 3;
							return r + r2;
						}
						std::memcpy(out, wide_sonant_katas + ofs - 16*3, 3);
						*olen = 3;
						return r + r2;
					} else if (std::memcmp(buff2, "ﾟ", r2) == 0) {
						if (idx >= 36 && idx <= 41) {
							std::memcpy(out, wide_psign_katas + ofs - 36*3, 3);
							*olen = 3;
							return r + r2;
						}
					}
				}
			}
			std::memcpy(out, wide_katas + ofs, 3);
			*olen = 3;
			return r;
		}
	} else {
		//TODO: [ト゛] => [ド] 
	}
	*olen = 0;
	return 0;
}

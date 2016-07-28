# libshutup
c++ library for blocking improper words in sentence, which has large orthographical variants like Japanese.
currently english readme is under construction (wait for translation from Japanese)
日本語のReadmeは[こちら](https://github.com/umegaya/libshutup/README.jp.md)

## highlights
- matching dictinary word with large orthographical variants, for example, "badword" blocks following in default JP mode.
  - "BaDwOrD"
  - "b　a(d)w-o-r-d"
- more over japanese word "バッドワード" blocks following
  - "ﾊﾞｯﾄﾞﾜｰﾄﾞ" (half byte kana entry)
  - "ばっどわーど" (hira-kana)
  - "baddo wa-do" (roman entry)
- memory/search efficiency with extended implementation of Patricia tree.
- all memory can be allocatable via custom allocator (C++ version)
- customizable blocking settings consist of three layer
  - ignore glyph
  - alias
  - synonym

## supported platform
- iOS
- Android
- OSX (bundle)

## installation
```
git clone https://github.com/umegaya/libshutup
cd libshutup
make ios
make android
make bundle # osx bundle
make lib # osx static library (may work in linux)
```

## usage
``` C++
#include "checker.h"

int main(int argc, char *argv[]) {
	shutup::Checker c("jp", nullptr/* or specify pointer to shutup_allocator */);
	c.add("アイウエオ");
	c.add("abc");
	assert(c.should_filter("a-B-c"));
	assert(c.should_filter("あいうえお"));
	assert(!c.should_filter("bbc"))
	assert(!c.should_filter("あいうえこ"));
}
```


# libshutup (v0.1.0)
c++ library for blocking improper words in sentence, which has large orthographical variants like Japanese.
日本語のReadmeは[こちら](https://github.com/umegaya/libshutup/README.jp.md)

## Highlights
- searching registered word pattern in dictionary from input, with large orthographical variants.
  - for example, in jp mode, "badword" blocks following:
    - "BａDworD" (ignore half/full width, upper/lower case difference)
    - "b　a(d)w-o-r-d" (ignore symbolic characters inserted between word pattern)
  - also, Japanese word "バッドワード" blocks following (in jp mode)
    - "ﾊﾞｯﾄﾞﾜｰﾄﾞ" (ignore half/full width difference)
    - "ばっどわーど" (ignore kata-kana/hira-kana difference)
    - "ﾊﾞｯドわーﾄﾞ" (ignore both difference)
    - "baddo wa-do" (matches *entire word* Romanization: Hepburn style, Japan style)
- highly customizable, memory and execution efficient matching by using extended patricia tree implementation.
- custom allocation: all memory can be allocated with your custom allocator
- good abstraction to easily build your own word checker: 
  - 4 abstraction layer which has different scope of application
    - normalize (and ignore, special case of normalize)
    - alias
    - synonym
    - context

## Platform
- iOS
- Android
- OSX (bundle or static lib)

## Installation
```
git clone https://github.com/umegaya/libshutup
cd libshutup
make ios
make android
make bundle # osx bundle
make lib # osx static library (may work in linux)
```

## Basic usage

- C++
  ``` C++
  #include "checker.h"

  int main(int argc, char *argv[]) {
    //create checker object on stack.
    shutup::Checker c(
      "jp", /* using built-in check routine for Japanese */
      nullptr/* using standard malloc/realloc/free for memory allocation */
    );

    //create checker object on stack, giving custom allocator
    shutup::Checker::Allocator a = { my_malloc, my_free, my_realloc };
    shutup::Checker c2 = shutup::Checker("jp", &a);

    //register word
    c.add("アイウエオ");
    word_context *pctx = new word_context(OnlyForName);
    c.add("abc", pctx); //if you want to give context related with registered word

    //word check
    int start, count;
    void *ctx;
    assert(c.should_filter("a-B-c", 5, &start, &count, &ctx) && ctx == pctx); //returns given context for the word
    assert(c.should_filter("あいうえお", 15, &start, &count));
    assert(!c.should_filter("bbc", 3, &start, &count));
    assert(!c.should_filter("あいうえこ", 15, &start, &count));
  }
  ```

- C
  ``` C
  #include "shutup.h"

  int main(int argc, char *argv[]) {
    //create checker object
    shutter s = shutup_new("jp", NULL);

    //create checker object with custom allocator
    shutup_allocator a = { my_malloc, my_free, my_realloc };
    shutter s2 = shutup_new("jp", &a);

    //register word 
    shutup_add_word("アイウエオ", NULL);
    word_context *pctx = malloc(sizeof(word_context));
    pctx->kind = OnlyForName;
    shutup_add_word("abc", pctx);//if you want to give context related with registered word

    //check
    int start, count;
    assert(shutup_should_filter(s, "a-B-c", 5, &start, &count) == pctx); //returns given context for the word
    assert(shutup_should_filter(s, "あいうえお", 15, &start, &count));
    assert(!shutup_should_filter(s, "bbc", 3, &start, &count));
    assert(!shutup_should_filter(s, "あいうえこ", 15, &start, &count));
  }
  ```

## Under the food of word checker: 4 abstraction layers
- libshutup has 4 abstraction layer which absorbs orthographical variants of words. and user can combinate layers to realize various requirement of checking existence of improper word.

### normalize
- specifying set of character which can be always equate each other, and convert character in input to the one of such a group. 
- this group called "normalize group".
- in JP mode, following normalize groups are configured.
  - groups of same alphabet's variant (half/full width, upper/lower case) => convert to lower,half width alphabet
  - groups of same kata-kana's variant (half/full width) => convert to full width kata-kana

### ignore
- ignore is special case of normalization. but treated as independent category because it is commonly used for efficient improper word matching. 
- for example, suppose here is improper word [badword], and spacing version of it [b a d w o r d]. human is still able to recognize the meaning of second one, so we need to remove these kind of character, before match with word dictionary.
- we have built in functionality to specify normalize group which equates with empty string, and call it "ignored"
- in JP mode, following characters are ignored.
  ```
  -+!\"#$%%&()*/,:;<=>?@[\\]^_{|}~ 
  ｰ
  、。，．・：；？！゛゜´｀¨＾￣＿ヽヾゝゞ〃仝々〆〇ー‐／＼～∥｜…‥
  ‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴
  ♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓
  〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭
  ♪†‡¶◯〝〟≒≡∫∮√⊥∠∵∩∪　  
  ```

### alias
- equation by normalizing applies all string, which may cause frequent false-positive detection. for example, Japanese kata-kana ソ(so) and ン(n), these character should *_not_* be always equate with each other, but equation of ン(n) with ソ(so) is frequently used to describe some kind of Japanese improper word (cannot write down in holy github :<)
- also in this example, equating ン(n) with ソ(so) is important but reverse relationship is not. normalize cannot correspond to such a asymmetci relationship. 
- alias is exactly for solving this kind of problem. alias is list of string which is relate with specified character. when matching character in improper word with input character, libshutup matches not only original character, but also lists which is related with the original. and if input matches alias, libshutup regard it as matching original one.
- in JP mode, following aliases are configured.
  - kata-kana and corresponding hira-kana, and vise versa (caution: alias is one-way link)
  - ン(n) => ソ(so) (ン(n) is not alias of ソ(so))

### synonym
- alias and normalize can cover large part of orthographical variants, but not enough for common use case in Japanese.
- Romanization is a way to describe Japanese with alphabet, and some people try to describe improper word with its Romanization expression. so we want to block Romanization expression of Japanese word which described in hira-kana or kata-kana. but: 
  - normalize equate e.g.[か] with [ka], which seems to cause unpredictable side effect.
  - configuring [ka] as alias of [か], causes matching with mixed string of hira-kana, kata-kana and its Romanization expression, that also causes unpredictable side effect.
- all we actually want to do is, blocking Romanization form of improper word which only contains hira-kana and kata-kana. synonym achieve this by configuring "alias per word" 
- in JP mode, if improper word entry only contains hira-kana and kata-kana, Romanization form (Hepburn, Japan style) of the word also added as improper word.

### context
- in above 3 layer, improper word always matches if pattern appeared in input. but some application wants to match an improper word in limited situation. for example, only matches when input used as user name or input starts with improper word. 
- corresponding to such an application specific requirement in library side, is not good idea because it increases complexity of internal implementation or code size. libshutup solves this problem by enabling you to attach application specific information called "context", and to specify callback which receive matching position and related context when match occurs. 
- please refer bindings/Unity/Shutup.cs for detail, which implements forward/backward/exact matching of improper words.

## Implement original word checker
- by inheriting shutup::language::WordChecker, you can implement original word checker.
- please refer src/language/jp.cpp as example.
- basic example
  ``` C++
  class MyWordChecker : public shutup::language::WordChecker {
    //implementations...
  }

  //create checker which uses custom checker "MyWordChecker" on stack
  shutup::Checker c(nullptr, &shutup::Checker::new_word_checker<MyWordChecker>);

  //same example as above, but using custom allocator.
  shutup::Allocator a = { my_malloc, my_free, my_realloc };
  shutup::Checker *c = shutup::Checker::create<MyWordChecker>(&a);
  ```

### Overrid-able implementation
- basic implementation
  - *_int init()_*
    - called when WordChecker is initialized
    - mainly configure alias, ignore, normalize settings, described in "Under the food of word checker: 4 abstraction layers"
      - for alias, ignore, call shutup::language::WordChecker::link_alias, set_alias, ignore_glyphs.
      - for normalize, call normalizer_.push_back to add built in normalizers and customs.
  - *_void fin()_*
    - free resources if you allocate them in init()
    - for calling link_alias, set_alias, ignore_glyph, and normalizer_.push_back, no need to write clean up code.
  - *_void add_synonym(const char *pattern, Checker &c)_*
    - configure synonym. 
    - pattern is given improper word entry. if there is synonym of it, please add it by using c.add_word call.
- low level implementation
  - *_int normalize(const u8 *in, int ilen, u8 *out, int olen)_*
    - you can replace entire normalize implementation by overriding this function.
    - in, ilen is received strings. if there is a character which can be normalized to other string, write converted result to out, olen.
    - returns bytes count written in out.
  - *_int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs)_*
    - you can replace entire matching routine which checks how many bytes count matches between (pattern, plen) and (in, ilen), when traversing Patricia trie.
    - write bytes count read for (in, ilen) to *ofs, and returns how many bytes count read for (pattern, plen). 

## Using custom memory allocator
- you can use your custom memory allocation routine by passing function which has same interface as malloc/free/realloc.
- code is slightly different whether allocating shutup::Checker instance by custom allocator, or not.

### allocate shutup::Checker instance without using custom allocator
``` C++
//declare custom allocator
shutup::Allocator a = { my_malloc, my_free, my_realloc };

//initialize built in checker (jp mode)
shutup::Checker c1("jp", &a);
shutup::Checker *pc1 = new shutup::Checker("jp", &a);

//initialize customized checker
shutup::Checker c2(&a, &shutup::Checker::new_word_checker<MyWordChecker>);
shutup::Checker *pc2 = new shutup::Checker(&a, &shutup::Checker::new_word_checker<MyWordChecker>);

//free memory
delete pc1;
delete pc2;
//c1, c2 autometically free memory when it goes out of scope
```

### allocate shutup::Checker instance using custom allocator
``` C++
//declare custom allocator
shutup::Allocator a = { my_malloc, my_free, my_realloc };

//initialize built in checker (jp mode)
shutup::Checker *pc1 = new shutup::Checker::create("jp", &a);

//initialize customized checker
shutup::Checker *pc2 = new shutup::Checker::create<MyWordChecker>(&a);

//free memory
shutup::Checker::destroy(pc1);
shutup::Checker::destroy(pc2);
```


# libshutup (v0.1.0)
libshutupは日本語のように、同じ意味の言葉を色々なやり方で記述できる言語における禁止語句のブロックを効率的に行うためにデザインされたC++で書かれたライブラリです.
for English readme. go to [here](https://github.com/umegaya/libshutup/README.md)

## ハイライト
- 辞書に登録した単語を、表記の揺れを考慮しつつマッチングします, 例えば日本語(JP)モードにおいて、 "badword" は以下をブロックします。
  - "BａDworD" (大文字小文字、全角半角文字の違いを無視)
  - "b　a(d)w-o-r-d" (間に挟まった記号を無視)
- それに加えて、日本語 "バッドワード" は以下のようなパターンもブロックします。
  - "ﾊﾞｯﾄﾞﾜｰﾄﾞ" (半角カナ)
  - "ばっどわーど" (ひらがな)
  - "ﾊﾞｯドわーﾄﾞ" (文字種の混合)
  - "baddo wa-do" (ローマ字表記：ヘボン式、日本式)
- 拡張されたPatricia木により、高いカスタマイズ性を持ちながら、メモリおよび実行効率の良いマッチングができます。
- ゲームなどのメモリ管理が厳しいアプリケーション向けに、C++バージョンのライブラリはすべてのメモリをアロケーター経由で割り当てることができます。
- 適用範囲のことなる4つのレイヤーの組み合わせによって多様な方法で禁止語句のマッチングが可能です。これにより他の言語についても有効なブロック処理を比較的簡単に構築できます。
  - normalize (specialなケースとしてのignore)
  - alias
  - synonym
  - context

## プラットフォーム
- iOS
- Android
- OSX (bundle or static lib)

## ビルド
```
git clone https://github.com/umegaya/libshutup
cd libshutup
make ios
make android
make bundle # osx bundle
make lib # osx static library (may work in linux)
```

## 基本的な使い方

- C++
  ``` C++
  #include "checker.h"

  int main(int argc, char *argv[]) {
    //スタック上、あるいはクラスのメンバ変数としてチェッカーオブジェクトを作成する場合
  	shutup::Checker c(
  		"jp", /* 組み込みの日本語向け禁止文字チェックルーチンを利用する。 */
  		nullptr/* 標準のmalloc,free,reallocを使う */
  	);

    //アロケーターを与える場合
    shutup::Checker::Allocator a = { my_malloc, my_free, my_realloc };
    shutup::Checker c2 = shutup::Checker("jp", &a);

    //辞書の登録
  	c.add("アイウエオ");
    word_context *pctx = new word_context(OnlyForName);
  	c.add("abc", pctx); //語句に紐付いたコンテクストを与える場合.

    //チェック
    int start, count;
    void *ctx;
  	assert(c.should_filter("a-B-c", 5, &start, &count, &ctx) && ctx == pctx);
  	assert(c.should_filter("あいうえお", 15, &start, &count));
  	assert(!c.should_filter("bbc", 3, &start, &count));
  	assert(!c.should_filter("あいうえこ", 15, &start, &count));
  }
  ```

- C
  ``` C
  #include "shutup.h"

  int main(int argc, char *argv[]) {
    //標準のメモリ割当ルーチンを利用してチェッカーオブジェクトを作成する.
    shutter s = shutup_new("jp", NULL);

    //アロケーターを与える場合
    shutup_allocator a = { my_malloc, my_free, my_realloc };
    shutter s2 = shutup_new("jp", &a);

    //辞書の登録
    shutup_add_word("アイウエオ", NULL);
    word_context *pctx = malloc(sizeof(word_context));
    pctx->kind = OnlyForName;
    shutup_add_word("abc", pctx);//語句に紐付いたコンテクストを与える場合.

    //チェック
    int start, count;
    assert(shutup_should_filter(s, "a-B-c", 5, &start, &count) == pctx);
    assert(shutup_should_filter(s, "あいうえお", 15, &start, &count));
    assert(!shutup_should_filter(s, "bbc", 3, &start, &count));
    assert(!shutup_should_filter(s, "あいうえこ", 15, &start, &count));
  }
  ```

## 禁止語句マッチングの仕組み
- libshutupは文字列の同一性を判定するための幾つかの抽象的な機能を用意し、その組み合わせによって任意の禁止語句のブロックの仕様を実現するようにしています。
- 以下にその概念を説明します。

### normalize: 正規化
- いかなる場合でも同一視できる文字のグループを指定し、入力に含まれている文字をそれと同一視される文字グループに含まれる要素の１つに変換する機能です。
- JPモードにおいては、以下のような正規化のグループが指定されています。
  - 全角/半角, 大文字/小文字の同じ種類のアルファベットおよび数字のグループ => 半角小文字および数字に変換
  - カタカナ、半角カナのグループ => 全角カナに変換 

### ignore: 無視
- normalizeの特殊な例で、効率的な禁止語句マッチのためには必ず必要になってくるため独立したカテゴリーとして説明します。
- 例えば[badword]に対して[b a d w o r d]のように空白を含んでも人間は意味を解釈することができてしまうため、こういった文字として認識されない文字は取り除いてからマッチングを行わないといくらでもフィルターを抜けられてしまいます。
- そのため、空文字列に同一視される文字のグループを組み込みで設定できるようにしており、これをignoreと呼んでいます。
- JPモードにおいては以下のように設定されています。
  ```
  "-+!\"#$%%&()*/,:;<=>?@[\\]^_{|}~ "
  "ｰ" //half kata hyphen
  "、。，．・：；？！゛゜´｀¨＾￣＿ヽヾゝゞ〃仝々〆〇ー‐／＼～∥｜…‥"
  "‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴"
  "♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓"
  "〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭"
  "♪†‡¶◯〝〟≒≡∫∮√⊥∠∵∩∪　"
  ```

### alias: 別名
- normalizeの同一視は文字列すべてに適用されてしまうので、予測できない誤マッチングが多くなりがちだと考えられます。例えば、ソ(そ)とン(ん)などのように、あらゆる場所で一致させると問題がある一方で、一部の単語を記述する際には頻繁に置き換えて使われる（神聖なgithubには書けませんが）文字もあります。
- またこの例ではン=>ソという同一視は重要ですが、逆はそれほど重要ではありません。normalizeではこういった非対称性に対応できません。
- こういった問題に対応するためにaliasが存在します。aliasは文字に対応づけられた文字列のリストで、辞書に含まれる文字にマッチングする際に辞書の文字そのものだけでなく、入力がその文字のaliasのどれかにマッチしていないかも調べます。そして、aliasにマッチした際にももともとの辞書にあった文字にマッチしたものとして処理を続けます。
- JPモードにおいては、以下のようなエイリアスが設定されています。
  - カタカナとそれに対応するひらがな、またその逆(aliasは一方向リンクであることに注意してください)
  - ン => ソ (逆はaliasではありません)

### synonym: 類義語
- aliasとnormalizeでかなりのパターンの表記の揺れをカバーできると思いますが、この２つだけではやはり細かなブロックができないケースがあります。
- 日本語をアルファベットで表記する方法としてローマ字表記がありますが、日本語をひらがな、カタカナで追加した際に自動的にローマ字表記をブロックすることを考えます。
  - 例えばすべての「か」を「ka」と同一視する(normalize)のは予測できないマッチングが増えそうです
  - 単語のマッチ時に現れた「か」を「ka」と同一視する(alias)としても、ローマ字とひらがなが混じった文字列がマッチしてしまいます
- 本来は単語がひらがな、カタカナからなる場合にのみ、全体をローマ字に変換したものも一緒にブロックするようにしたいわけです。synonymはいわば単語単位でのaliasを設定することによりそのような要求にこたえます。
- JPモードにおいては、入力文字列がひらがな、カタカナのみからなる場合にそれをローマ字（ヘボン式、日本式）に変換した文字列も同様に禁止対象として追加しています。

### context: コンテクスト
- 上記３つはどのような状況においても、指定された語句が出現する限りマッチします。しかし例えばアプリケーションによっては、特定の語句は特定のシチューエーションで入力された時のみ禁止語句としたい、とか、出現箇所が先頭あるいは末尾の時のみマッチしたい、といったマッチする条件が単純なパターンの存在ではないケースがあります。こう言った要望に１つ１つライブラリ側で応えていくことは、実装の複雑化やコードサイズの肥大化をもたらし、あまり良い考え方とは言えません。そこでlibshutupはcontextと呼ばれるアプリーケーション定義の情報を禁止したい語句に紐付け、さらに禁止語句への単純なマッチが発生した際にコンテクストとマッチ位置を受け取るコールバックを設定できるようにすることでこう言った要望に対応できるようにしています。
- 詳しくはbindings/Unity/Shutup.csで前方一致、後方一致、完全一致などを実装していますのでそれをご覧ください。


## 独自のチェック機構の実装
- shutup::language::WordCheckerを継承して実装を変更することで独自のチェックを追加できます。
- 具体例はjp.cppを参照してください。
- コードの例
``` C++
class MyWordChecker : public shutup::language::WordChecker {
	//implementations...
}

//MyWordCheckerを使うcheckerをスタック上に生成する.
shutup::Checker c(nullptr, &shutup::Checker::new_word_checker<MyWordChecker>);

//MyWordCheckerを使い、さらにアロケーターを使ってpointerを割り当てる.
shutup::Allocator a = { my_malloc, my_free, my_realloc };
shutup::Checker *c = shutup::Checker::create<MyWordChecker>(&a);
```

### 変更可能な実装
- 基本的な実装
  - int init()
    - WordCheckerが初期化される際に呼び出されます.
    - 主に禁止語句マッチングの仕組みで説明した、alias, ignore, normalizeの設定を行います。
      - normalize以外は、設定用のAPI, shutup::language::WordChecker::link_alias, set_alias, ignore_glyphsを呼び出して設定してください.
      - normalizeはnormalizer_.push_backを呼び出して設定します。
  - void fin()
    - initで何か割り当てたリソースがあればそれを解放します。
    - link_alias, set_alias, ignore_glyph, およびnormalizer_.push_backの呼び出しについては何も後処理を行う必要はありません。
  - void add_synonym(const char *pattern, Checker &c)
    - 禁止語句マッチングの仕組みで説明した、synonymを行います。
    - patternが外部から禁止語句として設定された時に呼び出されます。patternと一緒に登録したい禁止語句がある場合にはc.add_wordを利用して設定を行ってください。
- ローレベルの実装
  - int normalize(const u8 *in, int ilen, u8 *out, int olen)
    - これをoverrideすることで、normalizeの処理をそのまま置き換えることができます。
    - in, ilenで受け取った文字列で、他の文字と同一視できるものがあれば、変換し、その結果をout, olenに書き込みます。
    - 最終的にoutに書き込まれたバイト数を返してください。
  - int match(const u8 *in, int ilen, const u8 *pattern, int plen, int *ofs)
    - これをoverrideすることで、patricia木を実際にトラバースする時に、ノードに含まれる文字列(pattern, plen)と入力(in, ilen)が一致するかのチェックをそのまま置き換えることができます。
    - ofsには入力の何バイト目までマッチしたかを返し、戻り値としてpatternの何バイト目までマッチしたかを返します。

## 独自のメモリアロケーターの使用
- 標準のメモリ割当関数 malloc, free, realloc と同じインターフェイスの関数を与えることでメモリの割当をカスタマイズすることができます。
- shutup::Checker自体をアロケーターで割り当てるかどうかで若干初期化と終了のコードが異なります。

### shutup::Checkerのインスタンス自体をアロケーターを使って割り当てない場合
```
//組み込みチェッカーの初期化
shutup::Checker c1("jp", nullptr);
shutup::Checker *pc1 = new shutup::Checker("jp", nullptr);

//カスタマイズされたチェッカーの初期化
shutup::Checker c2(nullptr, &shutup::Checker::new_word_checker<MyWordChecker>);
shutup::Checker *pc2 = new shutup::Checker(nullptr, &shutup::Checker::new_word_checker<MyWordChecker>);

//解放
delete pc1;
delete pc2;
//c1, c2の形式はスコープを外れる、ないし親クラスの開放時に自動的に解放される
```

### shutup::Checkerのインスタンス自体をアロケーターを使って割り当てる場合
```
//アロケーターの宣言
shutup::Allocator a = { my_malloc, my_free, my_realloc };

//組み込みチェッカーの初期化
shutup::Checker *pc1 = new shutup::Checker::create("jp", &a);

//カスタマイズされたチェッカーの初期化
shutup::Checker *pc2 = new shutup::Checker::create<MyWordChecker>(&a);

//解放
shutup::Checker::destroy(pc1);
shutup::Checker::destroy(pc2);
```

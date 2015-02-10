// C++11によるゲームプログラミング
#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "Holder.hpp"
#include "NamedObject.hpp"
#include "Task.hpp"

using namespace std;

#if defined(USING_OLD_STYLE)

// C++ for game
/*
  ゲーププログラミングの開発環境が変化してきました。
  アセンブラ
  C
  C++
  C#
  Java
  JavaScript

  メインはC++
  コーディングの容易さではC#やJava
  C++11で大幅な機能アップ

  ゲームプログラミングにC++11のフィーチャーを取り入れることで効率アップ

  コーディングの効率化
  バグの出にくい構造
  パフォーマンスアップ

  Chap.1 moveの活用
  1-1 右辺値参照のおさらい
  1-2 moveコンストラクタとmove operator = を実装
  1-3 std::swapの違い
  1-4 VisualStudioはmoveコンストラクタを自動生成してくれない
  1-5 落とし穴
  1-5-1 move後のクラスオブジェクトへのアクセス
  1-5-2 継承クラスのmoveコンストラクタ
  Chap.2 ラムダ式の活用
  2-1 コールバックの活用
  2-1 キャプチャの注意点
  Chap.3 コピーコンストラクタの廃止
  3-1 コピーコンストラクタをmoveコンストラクタに置き換える
  3-2 shared_ptrをunique_ptrに置き換える
  Chap.4 応用編 描画ループの処理
  4-1 1/60秒の間隔で呼ばれるメインループ
  4-2 Threadもco-routineも使わないマルチタスク的処理
  4-3 タスクマネージャの紹介

 */
// CHAPTER-1 moveの活用
//
// moveのおさらい
// 右辺値参照？

string a = "1";
string b = "2";
string c = a + b;
//         ~~~~~
//         右辺値
/*
右辺値とは、名前のない一時的に生成されるオブジェクト
この場合、string型で値が"12"の一時オブジェクトが生成される。
わかりやすく置き換えると、
*/
string a = "1";
string b = "2";
string tmp = a; /// copy 
tmp += b;       // resize and copy
string c = tmp; // copy

このような動作になる。
tmpはfuncを呼び出した後は不要になる、一時オブジェクトだ。
さて、最後の"c = tmp"で行われるコピーが無駄な動作ということは明白だ。
tmpをcにエイリアスしてしまえはば、解決する。ところが、

string& c= a + b;

これはエラーになる。"a + b"は右辺値なので、参照として使うことができない。
そこで、右辺値参照という新しい機能が追加された。

string&& c = a + b;

これで、内部的にはc++03で記述すると以下の動作とほぼ等しくなる
string tmp = a;
tmp += b;
string& c = tmp;

では、moveはどこでつかうかというと、
string&& c = a + b;
とするかわりに、
string c = move(a+b);
とすることで、a+bの一時オブジェクトをcに移動することになる。
cは左辺値なので、なんの制約もなく使うことができる。

moveのコストは、一般的にcopyよりも少ない。stringならば、バッファのポインタとサイズを記憶している変数をコピーするだけだ。

なお、上記の例はわかりやすくするために move(a+b)と書いたが、a+bは明らかに右辺値なので、moveは省略可能だ。
moveは、左辺値を右辺値に変換するときに使う。

string a = "1";
string c = move(a); // cにaのインスタンスが移動する。
// これ以降はaにアクセスしてはならない。



1-2 moveコンストラクタとmove operator = を実装











struct Text {
  char* text_ = nullptr;
  size_t length_ = 0;
  Text() = default;
  template <size_t N>
  Text(const (char&)[N] src)
	: text_(new char[N])
	, length_(N)
  {
	copyFrom(src);
  }
  // コピーコンストラクタ
  // 複製を作るのでバッファの確保とコピーが必要
  Text(const Text& src)
	: text_(new char[src.length_])
	, length_(src.length)
  {
	copyFrom(src);
  }
  // swap処理
  // ポインタと長さを入れ替えるだけなので高速
  void swap(Text& src) {
	std::swap(text_, src.text_);
	std::swap(length_, src.length_);
  }
  // 通常の代入 (copy constructor and swap ideom)
  Text& operator = (Text src) { // コピーコンストラクタで複製
	swap(src); // 複製と入れ替え
	return *this;
	// 以前のインスタンスはsrcとして破棄される
  } 

  // 文字列を結合する処理
  Text operator + (const Text& src) const {
	Text val;
	val.text_ = new char[length_ + src.length_];
	std::memcpy(val.text_, text_, length_);
	std::memcpy(val.text_ + length_, src.text_, src.length_);
	val.length = length_ + src.length_;
	return val;
  }
	
  ~Text() {
	delete [] text_;
  }

privte:
  void copyFrom(const char* src) {
	std::memcpy(text_, src, length_);
  }
};

Test addHoge(const Text& src) {
  Text hoge("hoge");
  return src + hoge;
}// c++03の場合、return時に(src+hoge)の一時オブジェクトがコピーされる

Text fuga = addHoge("fuga"); // オペレーター=によるコピーが発生
// c++03の場合、上記の1行で、
// newが4回
// "hoge"のコピーが2回
// "fuga"のコピーが１回
// "fugahoge"のコピーが3回
// 実行される

// C++11で実装されたmoveを使ってクラスを書き換えてみる
struct Text11 {
  char* text_ = nullptr;
  size_t length_ = 0;
  Text11() = default;
  template <size_t N>
  Text11(const (char&)[N] src)
	: text_(new char[N])
	, length_(N)
  {
	copyFrom(src);
  }
  // コピーコンストラクタ
  // 複製を作るのでバッファの確保とコピーが必要
  Text11(const Text11& src)
	: text_(new char[src.length_])
	, length_(src.length)
  {
	copyFrom(src);
  }
  Text11(Text11&& src)
	: text_(std::move(src.text_))     //ポインタなのでstd::moveは必要無い
	, length_(std::move(src.length_))
  {}
  
  // swap処理
  // ポインタと長さを入れ替えるだけなので高速
  void swap(Text11& src) {
	std::swap(text_, src.text_);
	std::swap(length_, src.length_);
  }
  // 通常の代入 (copy constructor and swap ideom)
  Text11& operator = (Text11 src) { // コピーコンストラクタで複製
	swap(src); // 複製と入れ替え
	return *this;
	// 以前のインスタンスはsrcとして破棄される
  }
  // 右辺値の代入
  Text11& operator = (Text11&& src) { // コピーコンストラクタで複製
	delete [] text_;
	text_ = src.text_;
	length_ = src.length_;
	return *this;
  }

  // 文字列を結合する処理
  Text11 operator + (const Text& src) const {
	Text11 val;
	val.text_ = new char[length_ + src.length_];
	std::memcpy(val.text_, text_, length_);
	std::memcpy(val.text_ + length_, src.text_, src.length_);
	val.length = length_ + src.length_;
	return val; // C++11ではreturn文はmoveされる
  }
	
  ~Text() {
	delete [] text_;
  }

privte:
  void copyFrom(const char* src) {
	std::memcpy(text_, src, length_);
  }
};

Test11 addHoge(const Text& src) {
  Text hoge("hoge");
  return src + hoge; // Test11にmoveコンストラクタがあるのでmoveされる
} 

Text fuga = std::move(addHoge("fuga"));
// c++11の場合、上記の1行で、
// newが3回
// "hoge"のコピーが1回
// "fuga"のコピーが1回
// "fugahoge"のコピーが0回
// 実行される


  
// 数字を文字列に変換する
// c++03
void toString(int v, string& s) {
  s = std::to_string(v);
}

// 悪い例
const char* toString(int v) {
  static int cnt = 0;
  static char buffer[16][10];
  int index = cnt++ % 16;
  sprintf(buffer[index], "%d", v);
  return buffer[index];
}

// c++11
string toString(int v) {
  return std::to_string(v); // 一時オブジェクトがreturnによりmoveされる
}

// moveされたオブジェクトへのアクセスに注意
// 継承クラスのmoveコンストラクタ
struct Base {
  vector<int> a;
  Base(const Base&);
  Base(Base&&);
};
struct Sub : Base {
  vector<int> b;
  Sub(const Sub& s)
	: Base(s)
	, b(s.b)
  {}
  Sub(Sub&& s)
	: Base(std::move(s)) // sはmoveされたので出涸らし状態. bはダークサイドに
	, b(std::move(s.b))  // <- ここで死にます
  {}
};
struct Sub : Base {
  vector<int> b;
  Sub(Sub&& s)
	: Base(std::move(static_cast<Base&&>(s))) // キャストすれば、bは生きている
	, b(std::move(s.b))  // OK
  {}
};
  

// ラムダ式の活用

// コールバックを簡単に書ける
vector<Hoge> hoges;
std::sort(begin(hoges), end(hoges), [](const Hoge& a, const Hoge& b) { return a < b; });

// 変数をキャプチャできる
float effect[];
std::sort(begin(hoges), end(hoges), [effect](const Hoge& a, const Hoge& b) { return effect[a] < effect[b]; });

// クラスメソッドのコールバックが簡単
foo([this]{ return myFunc_(); });
// 以前だと(boostで簡略化した場合)
foo(boost::bind(&MyClass::myFunc, _1));

// 上記のものは、コーディングが楽になるだけで、実行時のコードには影響がない

// ラムダ式による遅延評価

int add(int v1, int v2) {
  return v1 + v2;
}

int value_a = 1;
int value_b = 2;

void func() {
  int ret = add(value_a, value_b); // ret = 3
}

int add(std::function<int()> fa, std::function<int()> fb) {
  return fa() + fb();
}

void func() {
  int ret = add([]{return value_a},[]{return value_b;}); // ret = 3
}
// この例だと、結果は変わらないが、aとｂの変数を評価するタイミングが異なる
// 最初の例は、関数が呼ばれた時のaとbの値、次の例は、関数から帰る時のaとbの値が使われる

// value_aやvalue_bを変化させてしまう処理
void foo() {
  value_a = 0, value_b = 0;
}

int add(std::function<int()> fa, std::function<int()> fb) {
  foo(); // 時間のかかる処理か、aやbを変化させる処理
  return fa() + fb(); // aとbの評価はここで行われる
}

int a = 1;
int b = 2;
int ret = add2([&a]{return a},[&b]{return b;}); // ret = 0

// ラムダ式を使う時の注意点
// std::shared_ptrをそのままラムダ式にキャプチャさせると、リファレンスカウントがインクリメントされる

void foo(std::function<void>()> cb) {
  cb();
}


std::shared_ptr<Hoge> hoge;
void func() {
  foo([hoge]{ std::cout << hoge->name() << "done" << std::endl; });
  std::cout << hoge.use_count() << std::endl;
}

output:
done
2

http://melpon.org/wandbox/permlink/qboF5oRmsj6jYFoY



// moveを活用するためのTIPS

// 1)コピーを禁止する
struct Hoge {
  Hoge(const Hoge&) = delete;
  Hoge& operator=(const Hoge&) = delete;
};
// 2) moveコンストラクタを定義
Hoge(Hoge&&) {...}
Hoge& operator = (Hoge&&) {...}

// 3) コピーするための機能を提供
//   自分自身の複製を作る
Hoge clone() const {
  Hoge hoge;
  // 自分のインスタンスを複製
  return hoge;
}

void foo(Hoge& fuga) {
  Hoge h = fuga; // err
  Hoge h = move(fuga); // moveされる。fuga使用不能
  Hoge h = fuga.clone(); // fugaのコピーがmoveされる。
}

// 4) shared_ptrをunique_ptrに置き換える
// コンパイルエラーを修正すると自然にmove対応になる
vector<shared_ptr<Hoge>> hoges;
⬇️
vector<unique_ptr<Hoge>> hoges;

string s = toString(1);
//  string(string&& s)
//  ptr_ = s.ptr_;
//  length_ = s.length_;
//  s.ptr_ = 0;

// ラムダ式の活用

// boostとc++11標準でどちらを使った方が良いか？



// 一般的なゲームのメインループ
void main() {
  while(true) {
	// main loop
	update();
	draw();
	waitForVsync();
  }
}

// 毎フレーム呼ばれる
void update() {
  switch (status) {
  case TitleLogo:
	status = titleLogo(); break;
  case MainMenu:
	status = mainMenu(); break;
  case GameMain:
	status = gameMain(); break;
  case Ending:
	status = ending(); break;
  }
}

// タイトルロゴ画面
void titleLogo() {
  switch (logoState) {
  case 0:
	initializeScreen(); ++logoState; break;
  case 1:
	// 何かキーがおされたらメインメニューに行く
	if (keyInput()) {
	  return MainMenu;
	}
  }
  return TitleLogo;
}




// c++11でラムダ式を使って実装してみる
// 毎フレーム呼ばれる
void update() {
  titleLogo([]{
	  mainMenu([]{
		  gameMain([]{
			  ending();
			});
		});
	});
}

void titleLogo(std::function<void()> callback) {
  initializeScreen([=]{
	  static bool keypressed = false;
	  if (keypressed || keyInput()) {
		keypressed = true;
		callback();
	  }
	});
}

void initializeScreen(std::function<void()> callback) {
  static bool initialized = false;
  if (!initielized)
	initialized = true;
  else 
	callback();
}
#endif

// 遅延実行キューを使うクラス

template <typename T>
struct SmartValue {
  T* ptr_ = nullptr;
  T* value_ = nullptr;
  SmartValue() = default;
  SmartValue(T* ptr) : ptr_(ptr) {}
  SmartValue(T& ref) : ptr_(&ref) {}
  SmartValue(SmartValue&& rhs) : ptr_(rhs.ptr_), value_(rhs.value_) {
	rhs.ptr_ = nullptr;
	rhs.value_ = nullptr;
  }
  SmartValue(T&& rhs) : value_(new T(move(rhs))) {
	cerr << "Tv new instance " << value_->name_ << ":" << value_ << endl;
	cerr << "Tv old instance " << rhs.name_ << endl;
  }

  ~SmartValue() {
	cerr << "Tv destructor " << value_ << endl;
	if (value_) {
	  value_->release();
	  delete value_;
	  value_ = nullptr;
	}
  }
  int which() const { return ptr_ == nullptr ? 1 : 0; }
  T& get() { return ptr_ == nullptr ? *value_ : *ptr_; }
  const T& get() const { return ptr_ == nullptr ? *value_ : *ptr_; }
  SmartValue& operator = (SmartValue&& rhs) {
	ptr_ = rhs.ptr_;
	value_ = rhs.value_;
	rhs.ptr_ = nullptr;
	rhs.value_ = nullptr;
	return *this;
  }
};

template <typename T>
struct SmartValue2 {
  T* ptr_ = nullptr;
  T  value_;

  SmartValue2() = default;
  SmartValue2(T* ptr)  : ptr_(ptr) {}
  SmartValue2(T& ref)  : ptr_(&ref) {}
  SmartValue2(T&& rhs) : value_(move(rhs)) {}
  SmartValue2(SmartValue2&& rhs)
	: ptr_(rhs.ptr_)
	, value_(move(rhs.value_)) {
  }
  ~SmartValue2() {}

  
  int which() const { return ptr_ == nullptr ? 1 : 0; }
  T& get() { return ptr_ == nullptr ? value_ : *ptr_; }
  const T& get() const { return ptr_ == nullptr ? value_ : *ptr_; }
  SmartValue2& operator = (SmartValue2&& rhs) {
	ptr_ = rhs.ptr_;
	value_ = move(rhs.value_);
	rhs.ptr_ = nullptr;
	return *this;
  }
};
template <typename T>
class SmartValue1 {
  uint64_t body_[1 + sizeof(T)/sizeof(uint64_t)];
  T* ptr_ = nullptr;
public:
  SmartValue1() = default;
  SmartValue1(T* ptr)  : ptr_(ptr) {}
  SmartValue1(T& ref)  : ptr_(&ref) {}
  SmartValue1(T&& rhs) { new (body()) T(move(rhs)); }
  SmartValue1(SmartValue1&& rhs)
  {
	operator = (move(rhs));
  }
  SmartValue1& operator = (SmartValue1&& rhs) {
	ptr_ = rhs.ptr_;
	if (rhs.hasBody()) {
	  // 実体を持っていたらmove
	  new (body()) T(move(*rhs.body()));
	}
	else {
	  rhs.ptr_ = nullptr;
	}
	return *this;
  }

  bool hasBody() const { return ptr_ == nullptr; }

  T& get() { return hasBody() ? *body() : *ptr_; }
  const T& get() const { return hasBody() ? *body() : *ptr_; }
  
private:
  T* body() {return reinterpret_cast<T*>(&body_[0]); }
  const T* body() const { return reinterpret_cast<const T*>(&body_[0]); }
};

template <typename T>
struct SmartValue3 {
  mutable boost::variant<T*,T> value_;
  SmartValue3() = default;
  SmartValue3(T* ptr)  : value_(ptr) {}
  SmartValue3(T& ref)  : value_(&ref) {}
  SmartValue3(T&& rhs) : value_(move(rhs)) {}
  SmartValue3(SmartValue3&& rhs)
	: value_(move(rhs.value_)) {
  }
  ~SmartValue3() {}
  
  int which() const { return value_.which(); }
  T& get() { return value_.which() == 0 ? *boost::get<T*>(value_) : boost::get<T>(value_); }
  const T& get() const { return value_.which() == 0 ? *boost::get<T*>(value_) : boost::get<T>(value_); }
  SmartValue3& operator = (SmartValue3&& rhs) {
	value_ = move(rhs.value_);
	return *this;
  }
};

#if 0

template <typename T>
class Holder {
  uint64_t body_[1 + sizeof(T)/sizeof(uint64_t)];
  T* ptr_ = nullptr;
public:
  Holder() = delete;
  Holder(T& ref)  : ptr_(&ref) {}
  Holder(T* ptr)  : ptr_(ptr) {}
  Holder(T&& rhs) { new (body()) T(move(rhs)); }
  Holder(Holder&& t) { operator = (move(t)); }
  Holder(const Holder& h) = delete;
  ~Holder() {
	//cerr << "Holder " << get().name() << " destructed" << endl;
  }

  Holder& operator = (Holder&& rhs) {
	if (rhs.hasBody()) {
	  // 実体を持っていたらmove
	  new (body()) T(move(*rhs.body()));
	}
	else {
	  ptr_ = rhs.ptr_;
	  rhs.ptr_ = nullptr;
	}
	return *this;
  }
  void operator = (const Holder&) = delete;

  // 自分の複製を作る
  Holder clone(const char* msg = "") const {
	if (hasBody()) {
	  // 実体だったら複製を作る
	  return Holder(move(get().clone(msg)));
	}
	else {
	  // 参照だったらポインタをコピー
	  return Holder(ptr_);
	}
  }
  bool hasBody() const { return ptr_ == nullptr; }
  
  T& get() { return hasBody() ? *body() : *ptr_; }
  const T& get() const { return hasBody() ? *body() : *ptr_; }
private:
  T* body() {return reinterpret_cast<T*>(&body_[0]); }
  const T* body() const { return reinterpret_cast<const T*>(&body_[0]); }
};


struct Task : NamedObject<Task> {
  using Super = NamedObject<Task>;
  using TaskHolder = ts::namedobj::Holder<Task>;
  using RawArgs = vector<Task>;
  struct TaskArgs : vector<TaskHolder> {
	using TaskFunc = function<TaskStatus(TaskArgs&)>;
	
	TaskArgs() = default;
	TaskArgs(Task&& t) {  emplace_back(move(t));}
	TaskArgs(Task&& t, Task&&t2) {  emplace_back(move(t)); emplace_back(move(t2));}
	TaskArgs(TaskFunc&& t, TaskFunc&& t2) { emplace_back(move(t)); emplace_back(move(t2));}
  };

  using TaskFunc = TaskArgs::TaskFunc;
  TaskFunc func_;
  TaskArgs args_;
  Task() noexcept {}
  Task(const Task& t) = delete;

  Task(TaskFunc f)                   noexcept : Super(), func_(f) { setup(); }
  Task(TaskFunc f, Task&& t)         noexcept : Super(), func_(f), args_(move(t)) { setup();  }
  Task(TaskFunc f, TaskArgs&& tasks) noexcept : Super(), func_(f), args_(move(tasks)) {	setup(); }

  Task(const string& n)                               noexcept : Super(n, true) {}
  Task(const string& n, TaskFunc f)                   noexcept : Super(n), func_(f) { setup(); }
  Task(const string& n, TaskFunc f, Task&& t)         noexcept : Super(n), func_(f), args_(move(t)) { setup();  }
  Task(const string& n, TaskFunc f, TaskArgs&& tasks) noexcept : Super(n), func_(f), args_(move(tasks))  {	setup();  }
  
  Task(Task&& t) noexcept
  :  Super(move(static_cast<Super&&>(t)))
	, func_(move(t.func_))
	, args_(move(t.args_))
  {
	valid("constructor &&");
  }

  ~Task() noexcept {
	cerr << "Task:" << name() << " destructed" << endl;
	if (!nameReference_ && !name().empty()) {
	  if (name().length() > 5 && name().substr(name().length()-5) != "moved") {
		cerr << "!!! body destructed" << endl;
	  }
	}
  }


  bool empty() const {
	if (nameReference_) return false;
	if (func_) return false;
	return true;
  }

  Task makeReference() const {
	setUniqName();
	valid("makeRef");
	cerr << name() << " is ok" << endl;
	return Task(name_);
  }

  
  void setup() {
	args_.emplace_back(this);
	cerr << "Task::setup: name:" << name_ << " : " << this << endl;
  }
  void setup(RawArgs&& args) {
	for (auto&& a : args) {
	  args_.emplace_back(move(a));
	}
	setup();
  }


  Task clone(const char* msg = "") const {
	cerr << "Task clone:'" << msg << "'" << name_ << endl;
	return makeReference();
  }
  bool valid(const char* msg = "") const {
	if (nameReference_ && !name_.empty()) {
	  string msg2(msg);
	  msg2 += "+ref";
	  auto found = getBody();
	  if (found) found->valid(msg2.c_str());
	  else {
		//cerr << msg2 << " not found" << endl;
	  }
	  return true;
	}
	if (func_ && !args_.empty()) return true;
	if (!func_ && args_.empty()) return true;
	cerr << "valid(" << msg << ") ";
	if (func_) cerr << "<has func>"; else cerr << "<no func>";
	if (args_.empty()) cerr << "<empty args>";
	cerr << "<name:" << name_ << ">";
	cerr << "<nameref:" << nameReference_ << ">";
	cerr << "<" << this << ">";
	cerr << " invalid: " << name_ <<  endl;
	assert(!"invalid Task instance");
	return false;
  }

  void operator = (const Task& t) = delete; 

  void operator = (Task&& t) {
	Super::operator = (move(static_cast<Super&&>(t)));
	func_ = move(t.func_);
	args_ = move(t.args_);
	valid("operator = ");
  }
  TaskStatus go(){
	assert(func_);
	valid("go");
	assert(!args_.empty());
	return func_(args_);
  }

  bool hasFunction() const { return !nameReference_; }
  const string& name() const { return name_; }
  
  // 最後のタスクリストは呼び出したタスク
  Task& returnTask() {
	valid("return Task");
	return args_.at(args_.size()-1).get();
  }
  static TaskHolder& returnTask(TaskArgs& ta) {
	assert(!ta.empty());
	return ta.at(ta.size()-1);
  }
};

#define TASK_VALID(t) {\
	if (!t.nameReference_) {\
	  cerr << "TASK VALID: "; \
	if (t.func_) cerr << "<has func>";\
	if (t.args_.empty()) cerr << "<empty args>";\
	cerr << "<name:" << t.name_ << ">";\
	cerr << "<nameref:" << t.nameReference_ << ">";\
	cerr << "<" << &t << ">";\
	cerr << endl; \
	assert(!t.args_.empty()); \
	}}

#endif

using Task = ts::namedobj::Task;
using TaskArgs = typename Task::TaskArgs;
using TaskHolder = typename Task::TaskHolder;
using TaskStatus = ts::namedobj::TaskStatus;

class TaskQueue {
  deque<TaskHolder> queue_;
  deque<TaskHolder> nextqueue_;
  vector<Task> trash_;
public:
  
  void addTask(Task&& task) {
	cerr << "addTask1:" << task.name() << endl;
	task.valid("addtask");
	nextqueue_.emplace_back(TaskHolder(move(task)));
	cerr << "addTask1 done:" << task.name() << endl;
  }
  void addTask(TaskHolder&& task) {
	task.get().valid("addtask2");
	cerr << "addTask2:" << task.get().name() << endl;
	nextqueue_.emplace_back(move(task));
	cerr << "addTask2 done:" << task.get().name() << endl;
  }
  void addTask(const TaskHolder& task) {
	task.get().valid("addtask3");
	cerr << "addTask3:" << task.get().name() << endl;
	nextqueue_.emplace_back(move(task.clone("add")));
	nextqueue_.back().get().getBody()->valid("addtask33");
	cerr << "addTask3 done" << endl;
  }
  void update() {
	while (!queue_.empty()) {
	  auto task = std::move(queue_.front());
	  queue_.pop_front();
	  cerr << "taskname: " << task.get().name_ << endl;
	  if (task.get().nameReference_) {
		cout << "task'" << task.get().name() << "' is reference" << endl;
		if (auto found = task.get().getBody()) {
		  cerr << "found check" << endl;
		  found->valid("update");
		  cerr << "found is ok :" << found->name() << endl;
		  task = TaskHolder(found);
		  assert(task.get().func_);
		  task.get().valid("found and get");
		}
		else {
		  // error
		  cerr << "task:" << task.get().name() << " not found" << endl;
		  assert(0);
		  continue;
		}
	  }
	  else {
		task.get().valid("queue front");
	  }
	  assert(task.get().hasFunction());
	  cerr << "it task empty?" << endl;
	  assert(!task.get().empty());
	  cerr << "task.get().valid();" << endl;
	  task.get().valid("get");
	  cerr << "update do task()" << endl;
	  auto& taskbody = task.get();
	  assert(taskbody.func_);
	  auto ret = taskbody.go();
	  cerr << "update do task(" << taskbody.name() << ") done" << endl;
	  switch (ret) {
	  case TaskStatus::RemoveTask:
		if (task.hasBody() && !task.get().name().empty()) {
		  trash_.emplace_back(move(task.get()));
		}
		break;
	  case TaskStatus::ContinueTask:
		task.get().valid("continue");
		addTask(std::move(task));
		break;
	  default:
		break;
	  }
	}
	swap(queue_, nextqueue_);
  }

  /*
	Task goMainLoop() {
	Task t([this](TaskArgs&)-> TaskStatus {
	  addTask(taskRoot_);
	  return TaskStatus::RemoveTask;
	  });
	return t;
  }
  */
  Task root_;
  void run(Task&& func) {
	root_ = move(func);
	cerr << "run: " << root_.name() << endl;
	addTask(move(root_));
	cerr << "run: task added " << func.name() << endl;
  }


  void waitPred(TaskHolder& next, function<bool()> pred) {
	string ref = next.get().makeReference().name_;
	Task t([this, ref, pred](TaskArgs&){
		TaskHolder nh(ref);
		cerr << "waitPred" << endl;
		if (pred()) {
		  cerr << "addTask" << endl;
		  nh.get().valid("waitPred");
		  addTask(nh);
		  return TaskStatus::RemoveTask;
		}
		else {
		  return TaskStatus::ContinueTask;
		}
	  });
	t.valid("waitPred2");
	addTask(std::move(t));
  }
};

bool keyWait() {
  cout << "keyWait" << endl;
  return true;
}
void initializeScreen() {
  cout << "initializeScreen" << endl;
}
int selectedMenu() {
  static int n = 0;
  cout << "selectedMenu" << endl;
  return ++n;
}

TaskQueue task_;

void update() {
  task_.update();
}
int main() {
  auto titleLogo = [](TaskArgs& ar){
	assert(ar.size() > 0);
	initializeScreen();
	Task h("hoge");
	cerr << "1" << endl;
	ar.at(0).get().valid("titlelogo");
	task_.waitPred(ar.at(0), [] { return keyWait(); });
	cerr << "2" << endl;
	return TaskStatus::RemoveTask;
  };
  
  auto mainMenu = [](TaskArgs& ar) {
	cerr << "mainMenu" << endl;
	switch (selectedMenu()) {
	default:
	return TaskStatus::ContinueTask;
	case 1:
	  ar.at(0).get().valid("at0");
	  task_.addTask(ar.at(0));
	  return TaskStatus::RemoveTask;
	case 2:
	  ar.at(1).get().valid("at1");
	  task_.addTask(ar.at(1));
	  return TaskStatus::RemoveTask;
	case 3:
	  cerr << "exit" << endl;
	  exit(0);
	}
  };
  
  auto ending = [](TaskArgs& ar) {
	// do ending
	cerr << "ending" << endl;
	ar.at(0).get().valid("ending");
	task_.waitPred(ar.at(0), [] { return keyWait(); });
	return TaskStatus::RemoveTask;
  };
  auto gameMain = [](TaskArgs& ar) {
	// do main
	cerr << "gameMain" << endl;
	ar.at(0).get().valid("gameMain");
	task_.waitPred(ar.at(0), [] { return true; });
	return TaskStatus::RemoveTask;
  };
  auto settingMenu = [](TaskArgs& ar) {
	cerr << "settingMenu" << endl;
	//task_.waitPred(Task::returnTask(ar), [] { return keyWait(); });
	ar.at(0).get().valid("settingMenu");
	task_.waitPred(ar.at(0), [] { return keyWait(); });
	return TaskStatus::RemoveTask;
  };
  //task_.run({"titleLogo", titleLogo, { mainMenu, Task(ending)}});
  /*
  task_.run({"titleLogo", titleLogo,
		{
		  { "ending", ending, {ending, ending }},
			{  "main", mainMenu, {gameMain, ending}}
		}
	});
  */

  task_.run({
	"titleLogo",
	  titleLogo, {
	  "main", mainMenu,{
		{ // game main
		  gameMain, 
			{ending, Task("main")}
		  
		}
	  , { // setting menu
		  settingMenu, {Task("titleLogo")}
		}
	  }
	}
	});

  
  cerr << " task.run finish" << endl;

  //  try {
  uint32_t frame = 0;
  while(true) {
	cerr << "Frame:" << ++frame << endl;
	update();
	//draw();
	//waitForVsync();
  }
  /*
  }
  catch (const exception& ex) {
	cerr << "exception caught: " << ex.what() << endl;
	throw;
  }
  */
}


#if 0




// 状態タイプのボタン
class Button1 {
  bool pressed = false;
  Button1(Signals& s) : sig(s) {}
  void onClick(bool press) {
	pressed = press;
  }
  bool isPressed() const { return pressed; }
};

	callback(press);
  }
};

void hoge() {
  Signal sig;
  Button1 button1(sig);
  sig.connect(hogehoge);

  Button2 button2([](bool press){ hogehoge(); });
}
#endif
#if 0

// 参照先がmoveしたときの問題
vevtor<Hoge> foo() {
  Hoge hoge;
  Hoge* hogeptr = &hoge;
  std::vector<Hoge> hoges;
  hoges.emplace_back(move(hoge));
  // hogeRef を &hoges[0] にしたい
  // moveデストラクタがあると良いかも
  struct Hoge {
	~~Hoge() {
	  // 自分を参照しているオブジェクトに、破棄された事を通知する
	}
	void moveDestructor() {
	  signalTo(this);
	}
  };
  template <T> T&& move2(T&& h) { T hoge = move(h); h.moveDestructor(); return hoge; }
  
  
  return hoges;
}

#endif


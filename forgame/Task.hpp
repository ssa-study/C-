// -*-tab-width:4;c++-*-
//
// タスククラス
//
// Created by TECHNICAL ARTS h.godai 2014
//
// タスククラスTaskは、処理を行う関数と関数に与える引数(TaskArgs)を保持しているクラスです。
// TaskはNamedObjectを継承し、名前付きオブジェクトとして振舞います。
// タスクのコンストラクタは以下の書式を持ちます
// [名前], function<void(TaskMgr&, TaskArgs&)>, [{引数}[,{引数}[,...]]]
// となっています、名前はタスクの名前で、他のタスクからは名前で参照ができるようになっています。
// 引数リストは、タスクのリストです。タスクは、連携するタスクのリストを引数として受け取るようになっています。
// 引数で指定するタスクは、タスクの関数か、名称（文字列）が使用できます。

#pragma once

#include "NamedObject.hpp"

namespace ts {
namespace namedobj {
  using std::string;
  using std::move;
  using std::cerr;
  using std::endl;
  using std::forward;

  // タスク終了時の動作
  enum class TaskStatus {
	RemoveTask,   // 消滅する
	ContinueTask, // 継続する
  };

  // タスクの引数となるタスクのリスト
  template <typename T>
  struct TaskArgsT {
	using Task = T;
	using Args = std::vector<Task>;
	Args args_;
	typename Task::name_type self_; // 自分のタスク名
	typename Task::name_type parent_; // 呼び出し元のタスク名
	TaskArgsT() = default;
	TaskArgsT(Task&& task) {  create(move(task)); }
	// イニシャライザリストでムーブが使えないのでひと工夫
	TaskArgsT(Task&& task, Task&& tasks...) { create(move(task), forward<Task>(tasks));}

	// createは再帰的に呼び出してすべての引数をargs_にムーブで突っ込む
	void create(Task&& task) {
	  args_.emplace_back(move(task));
	}
	void create(Task&& task, Task&& tasks...) {
	  args_.emplace_back(move(task));
	  create(forward<Task>(tasks));
	}
	
	// 以下コンテナとして振舞うための必要最小限の定義
	using iterator = typename Args::iterator;
	using const_iterator = typename Args::const_iterator;
	using value_type = typename Args::value_type;
	using reference = typename Args::reference;
	using const_reference = typename Args::const_reference;
	iterator begin() { return args_.begin(); }
	iterator end() { return args_.end(); }
	const_iterator begin()const { return args_.begin(); }
	const_iterator end()const { return args_.end(); }
	reference at(size_t n) { return args_.at(n); }
	const_reference at(size_t n) const { return args_.at(n); }
	size_t size() const { return args_.size(); }
	bool empty() const { return args_.empty(); }
  };
  

  // タスククラスの定義
  // タスクは、定義を記述した関数TaskFuncと、その引数TaskArgsを保持するクラス
  template<typename TaskMgr>
  struct TaskT : NamedObject<TaskT<TaskMgr>> {
	using Task = TaskT<TaskMgr>;
	using Super = NamedObject<Task>;
	using Super::name;
	using Super::isReferenceObject;
	using Super::setUniqName;
	using Super::getBody;
	using TaskArgs = TaskArgsT<Task>;
	using TaskFunc = std::function<TaskStatus(TaskMgr&, TaskArgs&)>;

	// 関数（タスクの本体）
	TaskFunc func_;
	// タスクのリスト
	TaskArgs args_;

	// コンストラクタ
	TaskT() noexcept {}
	TaskT(const Task& t) = delete; // コピーコンストラクタは廃止
	
	TaskT(TaskFunc f)                   noexcept : Super(), func_(f) { initialize(); }
	TaskT(TaskFunc f, Task&& t)         noexcept : Super(), func_(f), args_(move(t)) { initialize();  }
	TaskT(TaskFunc f, TaskArgs&& tasks) noexcept : Super(), func_(f), args_(move(tasks)) {	initialize(); }
	
	TaskT(const string& n)                               noexcept : Super(n, true) {}
	TaskT(const string& n, TaskFunc f)                   noexcept : Super(n), func_(f) { initialize(); }
	TaskT(const string& n, TaskFunc f, Task&& t)         noexcept : Super(n), func_(f), args_(move(t)) { initialize();  }
	TaskT(const string& n, TaskFunc f, TaskArgs&& tasks) noexcept : Super(n), func_(f), args_(move(tasks))  {	initialize();  }

	// ムーブコンストラクタ
	TaskT(Task&& t) noexcept
	  : Super(move(static_cast<Super&&>(t)))
	  , func_(move(t.func_))
	  , args_(move(t.args_))
	{
	  valid("move constructor");
	}
	
	~TaskT() noexcept = default;
	
	// 空のタスクの場合true
	bool empty() const {
	  if (isReferenceObject()) return false;
	  if (func_) return false;
	  return true;
	}

	// cloneは参照型のタスクを作る
	Task clone() const {
	  cerr << "CloneTask: " << name() << endl;
	  return makeReference();
	}

	// コピー代入は廃止　
	void operator = (const Task& t) = delete; 

	// ムーブ代入
	void operator = (Task&& t) {
	  Super::operator = (move(static_cast<Super&&>(t)));
	  func_ = move(t.func_);
	  args_ = move(t.args_);
	  valid("operator = ");
	}

	// タスクを実行する
	TaskStatus operator () (TaskMgr& mgr){
	  assert(func_);
	  valid("go");
	  assert(!args_.self_.empty());
	  return func_(mgr, args_);
	}
	
	// 正当性のチェックmsgはデバッグ出力用
	bool valid(const char* msg = "") const {
	  if (isReferenceObject() && !name().empty()) {
		string msg2(msg);
		msg2 += "+ref";
		auto found = getBody();
		if (found) found->valid(msg2.c_str());
		else {
		  //cerr << msg2 << " not found" << endl;
		}
		return true;
	  }
	  if (func_ && !args_.self_.empty()) return true;
	  if (!func_ && args_.self_.empty()) return true;
	  cerr << "valid(" << msg << ") ";
	  cerr << "self:" << args_.self_ << " ";
	  if (func_) cerr << "<has func>"; else cerr << "<no func>";
	  if (args_.empty()) cerr << "<empty args>";
	  cerr << "<name:" << name() << ">";
	  cerr << "<nameref:" << isReferenceObject() << ">";
	  cerr << "<" << this << ">";
	  cerr << " invalid: " << name() <<  endl;
	  assert(!"invalid Task instance");
	  return false;
	}
	
  private:
	// タスクはコピーもクローンもできないため、自分を参照するタスクを生成する。
	Task makeReference() const {
	  setUniqName(); // 自分は参照されるのでユニークな名前をつける
	  valid("makeRef");
	  return Task(name());
	}
	// いろいろ初期設定
	void initialize() {
	  setUniqName();
	  args_.self_ = name();
	  for (auto& a : args_) {
		a.args_.parent_ = name(); // 自分の子供たちに自分の名前を教える　
	  }
	}

  };
  


}} // ts::namedobj

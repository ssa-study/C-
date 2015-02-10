// -*-tab-width:4;c++-*-
//
// タスク管理クラス
//
// Created by TECHNICAL ARTS h.godai 2014

#pragma once

namespace ts {
namespace namedobj {

  
class TaskQueue {
  using Task = TaskT<TaskQueue>;
  using TaskArgs = Task::TaskArgs;
  
  std::deque<Task> queue_;
  std::deque<Task> nextqueue_;
  std::vector<Task> trash_; // for debug
  bool finished_ = false; // 終了フラグ
public:
  // 外からupdate()を呼んでもらう
  TaskQueue() = default;
  // updateで呼ばれる関数を呼び出し元に通知する
  TaskQueue(std::function<void()>& func) {
	func = [this]{
	  cerr << "update" << endl;
	  update();
	};
  }
  
  void addTask(Task&& task) {
	cerr << "addTask: " << task.name() << endl;
	task.valid("addtask");
	nextqueue_.emplace_back(move(task));
  }

  void update() {
	while (!queue_.empty()) {
	  Task task(std::move(queue_.front()));
	  queue_.pop_front();
	  //cerr << "taskname: " << task.name() << endl;
	  auto body = task.getBody();
	  assert(body);
	  assert(!body->isReferenceObject());
	  assert(!body->empty());
	  body->valid("get");
	  //cerr << "update do task()" << endl;
	  auto ret = body.get()(*this);
	  cerr << "update: task '" << body->name() << "' done" << endl;
	  switch (ret) {
	  case TaskStatus::RemoveTask:
		if (!task.isReferenceObject()) {
		  trash_.emplace_back(move(task));
		}
		break;
	  case TaskStatus::ContinueTask:
		body.get().valid("continue");
		addTask(std::move(task));
		break;
	  default:
		break;
	  }
	}
	swap(queue_, nextqueue_);
  }

  // タスクの実行
  void run(Task&& func) {
	cerr << "run: " << func.name() << endl;
	addTask(move(func));
  }

  // 終了通知
  void finish() {
	finished_ = true;
  }

  // 終了チェック
  bool finished() const {
	return finished_;
  }

  // predがtrueになるまで待ってからnextを実行する
  void waitPred(Task& next, std::function<bool()> pred) {
	cerr << "waitPred(" << next.name() << ")" << endl;
	next.valid("waitPred");
	// ラムダ式にムーブでキャプチャできないので名前を渡す
	auto ref = next.clone().name();
	Task waittask([this, ref, pred](TaskQueue&, TaskArgs&){
		Task nh(ref);
		cerr << "waitPred" << endl;
		if (pred()) {
		  // 条件が成立したのでタスクを実行する
		  addTask(std::move(nh));
		  return TaskStatus::RemoveTask;
		}
		else {
		  // 条件が成立しないので次回にまわす
		  return TaskStatus::ContinueTask;
		}
	  });
	// 条件が成立したらタスクを実行するタスクを登録
	addTask(std::move(waittask));
  }
};

  using Task = TaskT<TaskQueue>;
  using TaskArgs = Task::TaskArgs;

}} // ts::namedobj
  

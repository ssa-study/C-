// -*-tab-width:4;c++-*-
//
// C++AdventCalender 2014 22th
// C++11によるゲームプログラミング
// Created by TECHNICAL ARTS h.godai 2014
//
#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <boost/optional.hpp>

#include "NamedObject.hpp"
#include "Task.hpp"
#include "TaskQueue.hpp"

using namespace std;
using namespace ts::namedobj;

// stub関数
bool keyWait();          // キー入力待ち
void initializeScreen(); // 画面を初期化する
int selectedMenu();      // 選択されたメニュー番号を返す
bool gameMainLoop();     // ゲームのメインルーチン


int main() {

  // タイトルロゴを表示するタスク
  auto titleLogo = [](TaskQueue& tq, TaskArgs& ar){
	assert(ar.size() > 0);
	initializeScreen();
	ar.at(0).valid("titlelogo");
	tq.waitPred(ar.at(0), [] { return keyWait(); });
	return TaskStatus::RemoveTask;
  };

  // メインメニューのタスク　
  auto mainMenu = [](TaskQueue& tq, TaskArgs& ar) {
	cerr << "mainMenu" << endl;
	switch (selectedMenu()) {
	default:
	return TaskStatus::ContinueTask;
	case 1:
	  // 引数の最初のタスクを実行する
	  ar.at(0).valid("mainMenu");
	  tq.addTask(ar.at(0).clone());
	  return TaskStatus::RemoveTask;
	case 2:
	  // 引数の二番目のタスクを実行する
	  ar.at(1).valid("mainMenu");
	  tq.addTask(ar.at(1).clone());
	  return TaskStatus::RemoveTask;
	case 3:
	  // 終了する
	  cerr << "finish! ===============" << endl;
	  tq.finish();
	  return TaskStatus::RemoveTask;
	}
  };

  // エンディングのタスク
  auto ending = [](TaskQueue& tq, TaskArgs& ar) {
	// do ending
	cerr << "ending" << endl;
	ar.at(0).valid("ending");
	// keyWait()がtrueを返したら、最初の引数のタスクを実行する
	tq.waitPred(ar.at(0), [] { return keyWait(); });
	return TaskStatus::RemoveTask;
  };

  // ゲームメインルーチンのタスク
  auto gameMain = [](TaskQueue& tq, TaskArgs& ar) {
	// do main
	cerr << "gameMain" << endl;
	// gameMainLoop()がtrueを返したら、最初の引数のタスクを実行する
	tq.waitPred(ar.at(0), [] { return gameMainLoop(); });
	return TaskStatus::RemoveTask;
  };

  // セッティングメニューのタスク
  auto settingMenu = [](TaskQueue& tq, TaskArgs& ar) {
	cerr << "settingMenu" << endl;
	Task ptask(ar.parent_);
	// 何かキーが押されたら、呼び出し元のタスクを実行する
	tq.waitPred(ptask, [] { return keyWait(); });
	return TaskStatus::RemoveTask;
  };

  // タスクマネージャーにタスクを登録
  TaskQueue taskqueue;
  taskqueue.run({
	"titleLogo",
	  titleLogo, {
	  "main", mainMenu,{
		{ // game main
		  gameMain, 
			{ending, {"main"}}
		  
		}
	  , { // setting menu
		  settingMenu
		}
	  }
	}
	});

  // ゲームのメインループ
  uint32_t frame = 0;
  while(!taskqueue.finished()) {
	cerr << "Frame:" << ++frame << endl;
	taskqueue.update();
	//draw(); // ゲームの場合レンダリングの処理が入る
  }
}

// 以下、ありがちな処理のスタブクラス
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
bool gameMainLoop() {
  cout << "gameMainLoop" << endl;
  static int counter = 0;
  return ++counter > 3;
}

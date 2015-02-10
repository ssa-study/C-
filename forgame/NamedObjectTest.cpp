#include <iostream>


#include "Holder.hpp"

using namespace ts::namedobj;

int main() {
  using namespace std;
  
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
  //task_.run({"titleLogo", titleLogo, Task{ mainMenu, Task(ending)}});
  task_.run({
	"titleLogo",
	  titleLogo, {
	  "main", mainMenu, 
		{ // game main
		  gameMain, 
			{ending, Task("main")}
		  
		}
	  , { // setting menu
		settingMenu, {"titleLogo"}
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



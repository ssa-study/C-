#include <vector>
#include <map>
#include <iostream>

struct Point {
  int x_;
  int y_;
  Point(int x,int y) : x_(x), y_(y) {}
};

int main(int ac, char* av[]) {
  using namespace std;

  // vectorなら問題ない
  vector<Point> hoge;
  hoge.emplace_back(1,2);

  // mapの場合は、key,valueでOK
  map<string, int> intMap;
  intMap.emplace("hoge", 1);

  // valueのコンストラクタの引数が2個以上の場合は厄介
  map<string, Point> hogeMap;
  // hogeMap.emplace("hoge", 1, 2); // compile error!
  hogeMap.emplace(piecewise_construct          // pairのコンストラクタを召喚する呪文
				  , forward_as_tuple("hoge")   // tuple型の生成。make_tupleだとコピーされるので、
				  , forward_as_tuple(1,2));    // forward_as_tupleを使う


}

// piecewise_constructについて
// https://sites.google.com/site/cpprefjp/reference/utility/piecewise_construct

// forward_as_tupleについて
// https://sites.google.com/site/cpprefjp/reference/tuple/tuple/forward_as_tuple

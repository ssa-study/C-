#include<iostream>


template<int T>
struct Hoge {
};

template <char const * T>
struct Hoge2 {
};


constexpr char const constchar[] { "hoge" };

typedef const char C5[5];

constexpr C5& foo()  noexcept {
  return constchar;
}

constexpr int foo1() {
  return 1;
}

template <char T>
struct ss {
  constexpr ss() {}
};

template <size_t NN, size_t N>
constexpr char to_char(const char (&value)[N]) {
  return value[NN];
}

template <size_t NN, size_t N, typename ...T>
constexpr int to_string2(const char (&value)[N], T... params) {
  constexpr char c =  'c'; // to_char<NN>(value);
  return NN > 0 ? to_string2<NN-1>(value, ss<c>(), params...) : 0;
}

template <size_t N>
constexpr int to_string(const char (&value)[N]) {
  return to_string2<N-1>(value, N-1, value[0]);
}

constexpr char const hoge3[] = "hoge";

int main(int ac, char* av[]) {
  using namespace std;
  Hoge<foo1()> hoge1;
  Hoge2<constchar> hoge2; // OK
  Hoge2<hoge3> hoge_3;
  //Hoge2<foo()> hoge3;  
  int n = to_string("hogehoge");
  cout << n << endl;
}

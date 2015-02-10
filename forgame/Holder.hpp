#include <cstdint>

namespace ts {
namespace namedobj {
  
  template <typename T>
  class Holder {
  public:
	Holder() = delete;
	//Holder(T& ref)  : ptr_(&ref) {}
	Holder(T* ptr)  : ptr_(ptr) {}
	Holder(T&& rhs) : ptr_(body()), hasBody_(true) {
	  new (body()) T(std::move(rhs));
	}
	Holder(Holder&& t) { operator = (std::move(t)); }
	Holder(const Holder& h) = delete;
	~Holder() {
	  //cerr << "Holder " << get().name() << " destructed" << endl;
	}
	
	Holder& operator = (Holder&& rhs) {
	  hasBody_ = rhs.hasBody_;
	  if (hasBody_) {
		// 実体を持っていたらmove
		new (body()) T(std::move(*rhs.body()));
		hasBody_ = true;
		ptr_ = body();
	  }
	  else {
		// 参照の場合はポインタをコピー
		ptr_ = rhs.ptr_;
		rhs.ptr_ = nullptr;
	  }
	  rhs.moved();
	  return *this;
	}
	void operator = (const Holder&) = delete;
	
	// 自分の複製を作る
	Holder clone(const char* msg = "") const {
	  if (hasBody()) {
		// 実体だったら複製を作る
		return Holder(std::move(get().clone(msg)));
	  }
	  else {
		// 参照だったらポインタをコピー
		return Holder(ptr_);
	  }
	}

	// moveされた時に呼ばれる
	void moved() {
	  ptr_ = nullptr;
	  hasBody_ = false;
	}
	
	bool hasBody() const { return hasBody_; }
	
	T& get() { return *ptr_; }
	const T& get() const { return *ptr_; }

  private:
	T* body() {return reinterpret_cast<T*>(&body_[0]); }
	const T* body() const { return reinterpret_cast<const T*>(&body_[0]); }

	// 実体を格納するためのバッファ
	uint64_t body_[1 + sizeof(T)/sizeof(uint64_t)];
	// 参照のポインタ
	T* ptr_ = nullptr;
	// bodyを持つ時はtrue
	bool hasBody_ = false;

  };

}} // ts::namedobj

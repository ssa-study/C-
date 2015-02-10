// -*-tab-width:4;c++-*-
//
// 名前付きオブジェクトクラス
//
// Created by TECHNICAL ARTS h.godai 2014
//
// NamedObjectは、名前付きオブジェクトクラスです。~
// 名前で検索するためのDB(unorderdmap)を保持しています。~
// NamedObjectは、インスタンスを保持する実体としてのクラスと、インスタンスへの参照をもつ参照クラスの２つの形態があります。
// NamedObjectはコピー不可、ムーブ可能なクラスで、クラスインスタンスの型をとるCRTPの形式となっています。
// 名前の型は指定可能ですが、初期値はstd::stringです。std::unorderedmapのキーに利用できる型ならOKです。

#pragma once

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

namespace ts {
namespace namedobj {

  // NameType 名前を格納するクラス。通常はstd::string
  // ValueType オブジェクトの型
  template <typename ValueType, typename NameType = std::string>
  class NamedObject {
  public:
	using name_type = NameType;
	using value_type = ValueType;
	// default constructor
	NamedObject() {}
	NamedObject(name_type&& n, bool ref = false)
	  : name_(move(n))
	  , reference_(ref)
	{ regist(); }
	NamedObject(const name_type& n, bool ref=false)
	  : name_(n)
	  , reference_(ref)
	{ regist(); }
	// move constructor
	NamedObject(NamedObject&& n)
	  : name_(move(n.name_))
	  , reference_(n.reference_) {
	  regist();
	  n.name_ += name_ + "@moved";
	  n.moved_ = true;
	}
	// コピーコンストラクタは使用禁止
	NamedObject(const NamedObject&) = delete;
	// デストラクタではmoveされずに破棄されるオブジェクトをチェック
	~NamedObject() {
	  if (!(reference_ || moved_)) {
		std::cerr << "destruct: " << name_ << " has not moved" << std::endl;
	  }
	}

	// 代入はmoveのみ可
	NamedObject& operator = (NamedObject&& n) {
	  name_ = move(n.name_);
	  reference_ = n.reference_;
	  regist();
	  n.name_ = name_ + "@moved";
	  n.moved_ = true;
	  return *this;
	}

	// コピーの代入は禁止
	NamedObject& operator = (const NamedObject& n)  = delete; 
	// オブジェクトの取得
	boost::optional<value_type&> getBody() {
	  if (reference_) return lookup(name_);
	  return static_cast<value_type&>(*this);
	}
	// オブジェクトの取得
	boost::optional<const value_type&> getBody() const {
	  if (reference_) {
		if (auto f = lookup(name_)) {
		  return f.get();
		}
		else {
		  return boost::none;
		}
	  }
	  return static_cast<const value_type&>(*this);
	}

	// 名前へのアクセス
	const name_type& name() const { return name_; }
	name_type& nameRef() { return name_; }
	
	// 名前からオブジェクトを検索する
	static boost::optional<value_type&> lookup(const name_type& name) {
	  auto found = namedList_.find(name);
	  if (found != namedList_.end()) {
		return *found->second;
	  }
	  else {
		return boost::none;
	  }
	}
	// 無名のオブジェクトに参照用のユニークな名前を付ける
	void setUniqName() const {
	  if (name_.empty()) {
		size_t n = namedList_.size();
		for(;;) {
		  name_type name = boost::lexical_cast<name_type>(n);
		  if (namedList_.find(name) == namedList_.end()) {
			name_ = name;
			regist();
			return;
		  }
		  ++n;
		}
	  }
	}
	
	// 参照オブジェクトの場合はtrue
	bool isReferenceObject() const { return reference_; }


  private:
	// 名前から実体を検索するDBに登録する
	void regist() const {
	  if (!reference_) {
		// 実体だったら
		if (!name_.empty()) {
		  //std::cerr << "regist:" << name_ << ": " << this << std::endl;
		  namedList_[name_] =
			const_cast<value_type*>(static_cast<const value_type*>(this));
		}
	  }
	  else {
		// 参照だったら名前の有無をチェック
		assert(!name_.empty());
	  }
	}
  private:
	using NamedListType = std::map<name_type, value_type*>;
	static NamedListType namedList_;
	mutable name_type name_;
	bool reference_ = false; // 参照オブジェクトの場合はtrue
	bool moved_ = false; // moveされたオブジェクト(for debug)
	

  };
  
  template <typename V, typename N>
  typename NamedObject<V,N>::NamedListType NamedObject<V,N>::namedList_;
  

}} // ts::namedobj

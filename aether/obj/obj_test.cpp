/* Copyright 2016 Aether authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "obj.h"

#include <iostream>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using aether::Obj;

class PureInterface1 : public virtual Obj {
public:
  AETHER_PURE_INTERFACE(PureInterface1);

  virtual ~PureInterface1() {};
  virtual int Get1() const = 0;
};

class A : public PureInterface1 {
public:
  AETHER_OBJECT(A);
  AETHER_SERIALIZE(A);
  AETHER_INTERFACES(A, PureInterface1);

  virtual ~A() {}
  virtual int Get1() const {
    return val1;
  }
  int val1 = 123;

  template <typename T>
  T& Serializator(T& s) {
    s & val1;
    return s;
  }
};
AETHER_IMPLEMENTATION(A);

TEST_CASE( "Obj: Create, Pure, Cast, Serialize", "obj" ) {
  const char* cls = "A";
  A::ptr a(Obj::CreateClassById(qcstudio::crc32::from_string(cls).value));
  REQUIRE(a);
  PureInterface1::ptr p(a);
  REQUIRE(p);
  REQUIRE(p->Get1() == 123);

  Obj::ptr o(p);
  REQUIRE(o);
  AETHER_OMSTREAM os;
  os << o;
  REQUIRE(!os.stream_.empty());
  AETHER_IMSTREAM is;
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  Obj::ptr o1;
  is >> o1;
  REQUIRE(A::ptr(o1));
  REQUIRE(A::ptr(o)->Get1() == 123);
  REQUIRE(PureInterface1::ptr(o)->Get1() == 123);
}

class PureInterface2 : public virtual Obj {
public:
  AETHER_PURE_INTERFACE(PureInterface2);

  virtual ~PureInterface2(){};
  virtual int Get2() const = 0;
};

TEST_CASE( "Unsupported interface", "obj" ) {
  Obj::ptr o(new A());
  PureInterface2::ptr p(o);
  REQUIRE(!p);
}

class B : public PureInterface1, public PureInterface2 {
public:
  AETHER_OBJECT(B);
  AETHER_SERIALIZE(B);
  AETHER_INTERFACES(B, PureInterface1, PureInterface2);

  virtual ~B() {}
  virtual int Get1() const {
    return val1;
  }
  int val1 = 234;

  virtual int Get2() const {
    return val2;
  }
  int val2 = 456;

  template <typename T>
  T& Serializator(T& s) {
    s & val1 & val2;
    return s;
  }
};
AETHER_IMPLEMENTATION(B);

TEST_CASE( "MultiInterface", "obj" ) {
  B::ptr b(new B());
  AETHER_OMSTREAM os;
  os << b;
  AETHER_IMSTREAM is;
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  B::ptr b1;
  is >> b1;

  PureInterface1::ptr p1(b1);
  REQUIRE(p1);
  REQUIRE(p1->Get1() == 234);
  PureInterface2::ptr p2(b1);
  REQUIRE(p2);
  REQUIRE(p2->Get2() == 456);

  Obj::ptr o(b);
  PureInterface1::ptr p3(o);
  REQUIRE(p3);
  REQUIRE(p3->Get1() == 234);
  PureInterface2::ptr p4(o);
  REQUIRE(p4);
  REQUIRE(p4->Get2() == 456);

  PureInterface2::ptr p5(p3);
  REQUIRE(p5);
  REQUIRE(p5->Get2() == 456);

  B::ptr b2(p3);
  REQUIRE(b2);
  REQUIRE(b2->Get2() == 456);
}


class Pure1 : public virtual aether::Obj {
public:
  AETHER_PURE_INTERFACE(Pure1);
  virtual ~Pure1() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};

class Impl1 : public Pure1 {
public:
  AETHER_OBJECT(Impl1);
  AETHER_SERIALIZE(Impl1);
  AETHER_INTERFACES(Impl1, Pure1);
  virtual ~Impl1() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENTATION(Impl1);

class Pure2 : public virtual aether::Obj {
public:
  AETHER_PURE_INTERFACE(Pure2);
  virtual ~Pure2() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};

class Impl2 : public Pure2 {
public:
  AETHER_OBJECT(Impl2);
  AETHER_SERIALIZE(Impl2);
  AETHER_INTERFACES(Impl2, Pure2);
  virtual ~Impl2() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENTATION(Impl2);

class Pure12 : public Pure1, public Pure2 {
public:
  AETHER_OBJECT(Pure12);
  AETHER_SERIALIZE(Pure12);
  AETHER_INTERFACES(Pure12, Pure1, Pure2);
  virtual ~Pure12() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENTATION(Pure12);

class Impl12 : public Impl1, public Impl2 {
public:
  AETHER_OBJECT(Impl12);
  AETHER_SERIALIZE(Impl12);
  AETHER_INTERFACES(Impl12, Pure1, Pure2);
  virtual ~Impl12() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENTATION(Impl12);

class F : public Obj {
public:
  AETHER_OBJECT(F);
  AETHER_SERIALIZE(F);
  AETHER_INTERFACES(F);
  virtual ~F() {}

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENTATION(F);

TEST_CASE( "Smart pointer", "obj" ) {
  F::ptr f1(new F());
  F::ptr f2 = f1;
  REQUIRE(f1 == f2);
  REQUIRE(!(f1 != f2));
  F::ptr f3(new F());
  REQUIRE(f1 != f3);
  REQUIRE(!(f1 == f3));
  F::ptr f5;
  REQUIRE(f1 != f5);
  REQUIRE(!(f1 == f5));

  AETHER_OMSTREAM os;
  os << f1;
  AETHER_IMSTREAM is;
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  F::ptr f4;
  is >> f4;
  REQUIRE(f1 != f4);
  REQUIRE(!(f1 == f4));
}

TEST_CASE( "Obj::Ptr compare functions", "obj" ) {
  B::ptr b1(new B());
  REQUIRE(b1);
  PureInterface1::ptr p1(b1);
  REQUIRE(p1);
  REQUIRE(b1 == p1);
  PureInterface2::ptr p2(b1);
  REQUIRE(p1 == p2);
  PureInterface2::ptr p3(p1);
  REQUIRE(p1 == p3);

  B::ptr b2(new B());
  REQUIRE(b1 != b2);
  REQUIRE(p1 != b2);
  PureInterface1::ptr p4(b2);
  REQUIRE(p4 != p1);
  REQUIRE(p4 != b1);

  PureInterface1::ptr p5;
  REQUIRE(p5 != b1);
  REQUIRE(p5 != p1);
  PureInterface2::ptr p6;
  REQUIRE(p5 == p6);
}



namespace aether {
class TestAccessor {
public:
  template<class T>
  static void UnregisterClass() {
    aether::Obj::Registry<void>::UnregisterClass(T::class_id_);
  }
};
}

class V1 : public Obj {
public:
  AETHER_OBJECT(V1);
  AETHER_SERIALIZE(V1);
  AETHER_INTERFACES(V1);

  int i = 11;
  template <typename T>
  T& Serializator(T& s) {
    return s & i;
  }
};
AETHER_IMPLEMENTATION(V1);

class V2 : public V1 {
public:
  AETHER_OBJECT(V2);
  AETHER_SERIALIZE(V2, V1);
  AETHER_INTERFACES(V2, V1);

  float f = 2.2f;
  template <typename T>
  T& Serializator(T& s) {
    return s & f;
  }
};
AETHER_IMPLEMENTATION(V2);

class V3 : public V2 {
public:
  AETHER_OBJECT(V3);
  AETHER_SERIALIZE(V3, V2);
  AETHER_INTERFACES(V3, V2, V1);

  std::string s_{"text33"};
  template <typename T>
  T& Serializator(T& s) {
    return s & s_;
  }
};
AETHER_IMPLEMENTATION(V3);

TEST_CASE( "Versioning", "obj" ) {
  V1::ptr v1(new V1());
  v1->i = 111;
  V2::ptr v2(new V2());
  v2->i = 222;
  v2->f = 2.22f;
  V3::ptr v3(new V3());
  v3->i = 333;
  v3->f = 3.33f;
  v3->s_ = "text333";
  {
    // Upgrade serialized version: v1 -> v3, v2 -> v3, v3 -> v3
    AETHER_OMSTREAM os;
    os << v3 << v2 << v1;
    REQUIRE(os.stream_.size() == 103);
    AETHER_IMSTREAM is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Obj::ptr o1, o2, o3;
    is >> o3 >> o2 >> o1;
    REQUIRE(is.stream_.empty());
    V3::ptr v31 = o1;
    REQUIRE(v31);
    REQUIRE(v31->i == 111);
    REQUIRE(v31->f == 2.2f);
    REQUIRE(v31->s_ == "text33");
    V3::ptr v32 = o2;
    REQUIRE(v32);
    REQUIRE(v32->i == 222);
    REQUIRE(v32->f == 2.22f);
    REQUIRE(v32->s_ == "text33");
    V3::ptr v33 = o3;
    REQUIRE(v33);
    REQUIRE(v33->i == 333);
    REQUIRE(v33->f == 3.33f);
    REQUIRE(v33->s_ == "text333");
  }
  {
    // Downgrade serialized version: v3 -> v2, v2 -> v2
    AETHER_OMSTREAM os;
    os << v3 << v2;
    REQUIRE(os.stream_.size() == 87);
    AETHER_IMSTREAM is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Obj::ptr o2, o3;
    aether::TestAccessor::UnregisterClass<V3>();
    is >> o3 >> o2;
    REQUIRE(is.stream_.empty());
    V2::ptr v22 = o2;
    REQUIRE(v22);
    REQUIRE(v22->i == 222);
    REQUIRE(v22->f == 2.22f);
    V2::ptr v23 = o3;
    REQUIRE(v23);
    REQUIRE(v23->i == 333);
    REQUIRE(v23->f == 3.33f);
  }
  {
    // Downgrade serialized version: v3 -> v1, v2 -> v1
    AETHER_OMSTREAM os;
    os << v3 << v2;
    REQUIRE(os.stream_.size() == 87);
    AETHER_IMSTREAM is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Obj::ptr o2, o3;
    aether::TestAccessor::UnregisterClass<V2>();
    is >> o3 >> o2;
    REQUIRE(is.stream_.empty());
    V1::ptr v12 = o2;
    REQUIRE(v12);
    REQUIRE(v12->i == 222);
    V1::ptr v13 = o3;
    REQUIRE(v13);
    REQUIRE(v13->i == 333);
  }
}

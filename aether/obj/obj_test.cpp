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
  AETHER_DEFINE_PURE_CLS(PureInterface1);

  virtual ~PureInterface1() {};
  virtual int Get1() const = 0;
};

class A : public PureInterface1 {
public:
  AETHER_DEFINE_CLS(A, PureInterface1);

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
AETHER_IMPLEMENT_CLS(A);

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
  AETHER_DEFINE_PURE_CLS(PureInterface2);

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
  AETHER_DEFINE_CLS(B, PureInterface1, PureInterface2);

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
AETHER_IMPLEMENT_CLS(B);

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
  AETHER_DEFINE_PURE_CLS(Pure1);
  virtual ~Pure1() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};

class Impl1 : public Pure1 {
public:
  AETHER_DEFINE_CLS(Impl1, Pure1);
  virtual ~Impl1() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENT_CLS(Impl1);

class Pure2 : public virtual aether::Obj {
public:
  AETHER_DEFINE_PURE_CLS(Pure2);
  virtual ~Pure2() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};

class Impl2 : public Pure2 {
public:
  AETHER_DEFINE_CLS(Impl2, Pure2);
  virtual ~Impl2() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENT_CLS(Impl2);

class Pure12 : public Pure1, public Pure2 {
public:
  AETHER_DEFINE_CLS(Pure12, Pure1, Pure2);
  virtual ~Pure12() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENT_CLS(Pure12);

class Impl12 : public Impl1, public Impl2 {
public:
  AETHER_DEFINE_CLS(Impl12, Pure1, Pure2);
  virtual ~Impl12() {};

  template <typename T>
  T& Serializator(T& s) {
    return s;
  }
};
AETHER_IMPLEMENT_CLS(Impl12);

TEST_CASE( "Smart pointer", "obj" ) {
}

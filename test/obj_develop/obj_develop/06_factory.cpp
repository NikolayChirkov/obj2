// Copyright 2016 Aether authors. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================


/*#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>
#include "../../obj/aether/obj/obj.h"

class C : public aether::Obj {
public:
  AETHER_OBJECT(C);
  AETHER_SERIALIZE(C);
  AETHER_INTERFACES(C);
  
  std::string s_ = "test";
  template <typename T> void Serializator(T& s) {
    s & s_;
  }
};
AETHER_IMPLEMENTATION(C);

class B : public aether::Obj {
public:
  AETHER_OBJECT(B);
  AETHER_SERIALIZE(B);
  AETHER_INTERFACES(B);
  //virtual ~B() { std::cout << "~B: " << f << "\n"; }
  
  float f;
  C::ptr c;
  template <typename T> void Serializator(T& s) {
    s & f & c;
  }
};
AETHER_IMPLEMENTATION(B);

class A : public aether::Obj {
public:
  AETHER_OBJECT(A);
  AETHER_SERIALIZE(A);
  AETHER_INTERFACES(A);
  //virtual ~A() { std::cout << "~A\n"; }
  int i;
  B::ptr zero_b;
  B::ptr factory_b;
  B::ptr unloaded_b;
  std::vector<B::ptr> v;
  template <typename T> void Serializator(T& s) {
    s & i & zero_b & unloaded_b & factory_b & v;
  }
};
AETHER_IMPLEMENTATION(A);

void PrintA(const char* t, A::ptr a) {
  std::cout << "------------------------------------\n";
  std::cout << t << "\n";
  std::cout << "a.i = " << a->i << ", unloaded_b: " << (a->unloaded_b ? a->unloaded_b->f : 0.0f) << "\n";
  std::cout << "a.v.size = " << a->v.size() << ": ";
  for (auto b : a->v) std::cout << b->f << "/" << (b->c ? b->c->s_ : "zero") << ", ";
  std::cout << "\n";
}

void SaveToFolder() {
  aether::Domain domain;
  AETHER_OMSTREAM os;
  {
    os.custom_ = &domain;
    A::ptr a(new A());
    a->i = 123;
    {
      C::ptr c{new C()};
      c->s_ = "FFF";
      B::ptr b{new B()};
      b->f = 3.14f;
      b->c = c;
      a->v.emplace_back(b);
    }
    {
      B::ptr b{new B()};
      b->f = 1.23f;
      a->factory_b = b;
      a->factory_b.Unload(domain);
    }
    {
      C::ptr c{new C()};
      c->s_ = "QQQQ";
      B::ptr b{new B()};
      b->f = 4.56f;
      b->c = c;
      a->unloaded_b = b;
      a->unloaded_b.Unload(domain);
    }
    
    os << a;
    
    size_t s = os.stream_.size();
    for (auto d : os.custom_->data_) {
      s += d.second.size();
    }
    std::cout << "size: " << s << "\n";
    domain.saved_ids_.clear();
  }
  A::ptr a;
  {
    AETHER_IMSTREAM is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    is.custom_ = &domain;
    is >> a;
  }
  {
    PrintA("", a);
    a->unloaded_b.Load(domain);
    PrintA("unload_b.Load", a);
    a->unloaded_b->f = 2.71f;
    a->unloaded_b.Unload(domain);
    a->unloaded_b.Load(domain);
    PrintA("unload_b = 2.71 + unload + load", a);
    a->v.emplace_back(a->unloaded_b.clone(domain));
    PrintA("clone unloaded", a);
    a->v.emplace_back(a->factory_b.clone(domain));
    PrintA("clone from factory", a);
  }
  
  // Save domain
  {
    std::filesystem::create_directory("model");
  }
}
*/

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


#include <iostream>
#include <fstream>
#include <map>
#include "../../../aether/obj/obj.h"

#define OBSERVER_DEV
#define OBSERVER_ROOT_ID 666
static std::unordered_map<aether::ObjStorage, std::string> storage_to_path_;

auto saver = [](const aether::ObjId& obj_id, uint32_t class_id, aether::ObjStorage storage, const AETHER_OMSTREAM& os){
  std::filesystem::path dir = std::filesystem::path{"state"} / storage_to_path_[storage] / obj_id.ToString();
  std::filesystem::create_directories(dir);
  auto p = dir / std::to_string(class_id);
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

auto enumerator = [](const aether::ObjId& obj_id, aether::ObjStorage storage){
  std::string path = obj_id.ToString();
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::vector<uint32_t> classes;
  for(auto& f: std::filesystem::directory_iterator(p)) {
    classes.push_back(std::atoi(f.path().filename().c_str()));
  }
  return classes;
};

auto loader = [](const aether::ObjId& obj_id, uint32_t class_id, aether::ObjStorage storage, AETHER_IMSTREAM& is){
  std::filesystem::path dir = std::filesystem::path{"state"} / storage_to_path_[storage] / obj_id.ToString();
  auto p = dir / std::to_string(class_id);
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  if (!f.good()) return;
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};
using namespace aether;

namespace aether {
class TestAccessor {
public:
  template<class T>
  static void UnregisterClass() {
    aether::Obj::Registry<void>::UnregisterClass(T::class_id_);
  }
};
}

class A : public Obj {
public:
  AETHER_CLS(A);
  AETHER_SERIALIZE(A);
  AETHER_INTERFACES(A);
  A() {
#ifdef OBSERVER_DEV
    id_ = aether::ObjId::GenerateUnique();
    i = 123;
#endif  // OBSERVER_DEV
  }
  int i;
  template <typename T> void Serializator(T& s, int flags) { s & i; }
};
AETHER_IMPL(A);

class B1 : public A {
public:
  AETHER_CLS(B1);
  AETHER_SERIALIZE(B1, A);
  AETHER_INTERFACES(B1, A);
  B1() {
#ifdef OBSERVER_DEV
    id_ = aether::ObjId::GenerateUnique();
#endif  // OBSERVER_DEV
    f = 0.123f;  // must be initialized into default value
  }
  float f;
  template <typename T> void Serializator(T& s, int flags) { s & f; }
};
AETHER_IMPL(B1);

class B2 : public A {
public:
  AETHER_CLS(B2);
  AETHER_SERIALIZE(B2, A);
  AETHER_INTERFACES(B2, A);
  B2() {
#ifdef OBSERVER_DEV
    id_ = aether::ObjId::GenerateUnique();
#endif  // OBSERVER_DEV
    s = "123t";  // must be initialized into default value
  }
  std::string s;
  template <typename T> void Serializator(T& ss, int flags) { ss & s; }
};
AETHER_IMPL(B2);

class B2C : public B2 {
public:
  AETHER_CLS(B2C);
  AETHER_SERIALIZE(B2C, B2);
  AETHER_INTERFACES(B2C, B2, A);
  B2C() {
#ifdef OBSERVER_DEV
    id_ = aether::ObjId::GenerateUnique();
#endif  // OBSERVER_DEV
    d = 1.23e200;  // must be initialized into default value
  }
  double d;
  template <typename T> void Serializator(T& s, int flags) { s & d; }
};
AETHER_IMPL(B2C);

class Root : public Obj {
public:
  AETHER_CLS(Root);
  AETHER_SERIALIZE(Root);
  AETHER_INTERFACES(Root);
  
  std::vector<Obj::ptr> v;
  template <typename T> void Serializator(T& s, int flags) { s & v; }
};
AETHER_IMPL(Root);

void Versioning() {
  std::cout << "Obj " << aether::Obj::class_id_ << "\n";
  std::cout << "Root " << Root::class_id_ << "\n";
  std::cout << "A " << A::class_id_ << "\n";
  std::cout << "B1 " << B1::class_id_ << "\n";
  std::cout << "B2 " << B2::class_id_ << "\n";
  std::cout << "B2C " << B2C::class_id_ << "\n";
#ifdef OBSERVER_DEV
  {
    std::filesystem::remove_all("state");
    
    Root::ptr root{new Root()};
    root.SetId(OBSERVER_ROOT_ID);
    
    root->v.push_back({new A()});
    A::ptr(root->v.back())->i = 456;

    root->v.push_back({new B1()});
    B1::ptr(root->v.back())->f = 4.56f;

    root->v.push_back({new B2()});
    B2::ptr(root->v.back())->s = "456abc";

    root->v.push_back({new B2C()});
    B2C::ptr(root->v.back())->d = 4.56e300;
    B2::ptr(root->v.back())->s = "4.56e300";
    A::ptr(root->v.back())->i = 456300;

    root.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
  }
#endif  // OBSERVER_DEV
  Root::ptr root;
  root.SetId(OBSERVER_ROOT_ID);
  root.SetFlags(aether::ObjFlags::kLoaded);
  //TestAccessor::UnregisterClass<V2>();
  root.Load(enumerator, loader);
}

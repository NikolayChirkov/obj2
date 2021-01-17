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
#include <set>
#include <filesystem>
#include "../../../aether/obj/obj.h"
// * - domain initially loaded
// # - domain initially unloaded
// 0 - zero pointer
// B0 - class B, instance 0
// () - class factory

// ZeroObjPointers: nullptr reference to the object
// A0*
// |-B = 0

// SingleObjReference
// A0*
// |-B0

// BaseClassReference: serialization/deserialization through reference to the base class
// A0*
// |-B0 (Obj::ptr)

// MultipleObjReference: referencing different objects of a single class
// A0*
// |-B0
// |-B1

// Hierarchy
// A0*
// |-B0
//   |-C0


// HierachichalDomains
// A0*
// |-B*
//   |-C0*
//   |-C1#

// SharedReference: C0 is referenced twice
// A0*
// |-C0
// |-C0


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// SharedReference: C0 is referenced with child domain
// A0*
// |-C0
// |-B0*
//   |-C0

// CrossRerefenceDifferentLevel
// A0*
// |-B0
//   |-A0

// CrossRerefenceSameLevel
// A0*
// |-B0
//   |-B1
// |-B1
//   |-B0

// LoadUnload: the state is saved/restored
//   Cross references within a single level
//   Cross references with different level
// A0*
// |-C0
// |-B0#
//   |-C0
//     |-B0
//     |-C1
//   |-C1
//     |-C0

/*
#include <fstream>

static std::unordered_map<aether::ObjStorage, std::string> storage_to_path_;

static auto saver = [](const std::string& path, aether::ObjStorage storage, const AETHER_OMSTREAM& os){
  std::filesystem::create_directories(std::filesystem::path{"state"} / storage_to_path_[storage]);
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

static auto loader = [](const std::string& path, aether::ObjStorage storage, AETHER_IMSTREAM& is){
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};

class B : public aether::Obj {
public:
  AETHER_OBJECT(B);
  AETHER_SERIALIZE(B);
  AETHER_INTERFACES(B);
  B() = default;
  B(float f) : f_(f) {}
  virtual ~B() { std::cout << "~B[ " << id_.ToString() << " ]: " << f_ << "\n"; }

  float f_;
  std::vector<B::ptr> o_;
  template <typename T> void Serializator(T& s, int flags) {
    s & f_ & o_;
  }
};
AETHER_IMPLEMENTATION(B);

class Root : public aether::Obj {
public:
  AETHER_OBJECT(Root);
  AETHER_SERIALIZE(Root);
  AETHER_INTERFACES(Root);
  Root() = default;
  Root(int i) : i_(i) {}
  virtual ~Root() { std::cout << "~Root " << i_ << "\n"; }
  int i_;
  std::vector<B::ptr> o_;
  template <typename T> void Serializator(T& s, int flags) {
    s & i_ & o_;
  }
};
AETHER_IMPLEMENTATION(Root);

void DomainTest() {

    std::filesystem::remove_all("state");
//    {
//      Root::ptr root(new Root(123));
//      root.SetFlags(aether::ObjFlags::kLoadable | aether::ObjFlags::kLoaded);
//      B::ptr b(new B(2.71f));
//      B::ptr b2(new B(3.14f));
//      b->o_.push_back(b2);
//      b2->o_.push_back(b);
//      root->o_.push_back(b);
//      root->o_.push_back(b2);
//      b = nullptr;
//      b2 = nullptr;
//      root.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
//      root.Unload();
//      Root::ptr r1 = root.Clone(loader);
//      int sdf=0;
//      root = nullptr;
//      r1 = nullptr;
//    }

  Root::ptr root(new Root(123));
  root.SetId(666);
  {
    B::ptr b1(new B(2.71f));
    b1->o_.push_back({});

    B::ptr b2(new B(3.14f));
    b2.SetFlags(aether::ObjFlags::kLoadable | aether::ObjFlags::kLoaded);
    b2->o_.push_back(b1);
    b1->o_.push_back(b2);

    root->o_.push_back(b2);
    root->o_.push_back({});
//    root->o_.push_back(b1);
  }
  {
    root->o_[0].Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    root->o_[0].Unload();
    root.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    root = nullptr;
    int sdf=0;
  }
  {
    std::cout << "Restoring...\n";
    Root::ptr root;
    root.SetId(666);
    root.SetFlags(aether::ObjFlags::kLoaded);
    root.Load(loader);
    B::ptr r1 = root->o_[0].Clone(loader);
    B::ptr r2 = root->o_[0].Clone(loader);
    root->o_.push_back(r2);
    root->o_[0].Load(loader);
    r1 = nullptr;
    r2 = nullptr;
    root.Unload();
    int sfd=0;
  }
}

*/

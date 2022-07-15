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
#include <fstream>
/*
static std::unordered_map<aether::ObjStorage, std::string> storage_to_path_;
static std::filesystem::path root_path_;

auto saver = [](const aether::ObjId& obj_id, uint32_t class_id, aether::ObjStorage storage, const AETHER_OMSTREAM& os) {
  std::filesystem::path dir = root_path_ / storage_to_path_[storage] / obj_id.ToString();
  std::filesystem::create_directories(dir);
  auto p = dir / std::to_string(class_id);
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.u8string() << " size: " << os.stream_.size() << "\n";
};

auto enumerator = [](const aether::ObjId& obj_id, aether::ObjStorage storage) {
  std::string path = obj_id.ToString();
  auto p = root_path_ / storage_to_path_[storage] / path;
  std::vector<uint32_t> classes;
  for (auto& f : std::filesystem::directory_iterator(p)) {
    // Convert filename into integer value.
    auto c = std::stoull(f.path().filename().string(), nullptr, 10);
    classes.push_back(static_cast<uint32_t>(c));
  }
  return classes;
};

auto loader = [](const aether::ObjId& obj_id, uint32_t class_id, aether::ObjStorage storage, AETHER_IMSTREAM& is) {
  std::filesystem::path dir = root_path_ / storage_to_path_[storage] / obj_id.ToString();
  auto p = dir / std::to_string(class_id);
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  if (!f.good()) return;
  f.seekg(0, f.end);
  std::streamoff length = f.tellg();
  f.seekg(0, f.beg);
  is.stream_.resize(static_cast<size_t>(length));
  f.read((char*)is.stream_.data(), length);
};

class B01 : public aether::Obj {
public:
  AETHER_OBJ(B01);
  B01() = default;
  B01(float f) : f_(f) {}
  virtual ~B01() { std::cout << "~B[ " << id_.ToString() << " ]: " << f_ << "\n"; }

  float f_;
  std::vector<B01::ptr> o_;
  template <typename T> void Serializator(T& s) {
    s & f_ & o_;
  }
};
AETHER_IMPL(B01);

class Root01 : public aether::Obj {
public:
  AETHER_OBJ(Root01);
  Root01() = default;
  Root01(int i) : i_(i) {}
  virtual ~Root01() { std::cout << "~Root " << i_ << "\n"; }
  int i_;
  std::vector<B01::ptr> o_;
  template <typename T> void Serializator(T& s) {
    s & i_ & o_;
  }
};
AETHER_IMPL(Root01);

void DomainTest() {*/

/*  auto storage = [](uint32_t class_id) -> std::string {
    return class_id == B01::kId ? "ru" : "";
  };

  Domain d0;
  std::vector<std::map<std::string, std::map<std::string, std::vector<uint8_t>>>> blob;
  size_t current_state = 0;
  d0.serializator = [&blob, &storage, current_state](const aether::Obj::ptr& o,
                                                     const uint8_t* data,
                                                     size_t offset, size_t size) {
    uint32_t class_id = o->GetId();
    auto obj_id = o.obj_id_;
    blob.resize(current_state + 1);
    auto& f = blob[current_state][storage(class_id) + "/" + std::to_string(class_id)][obj_id.ToString()];
    f.insert(f.begin() + offset, data, data + size);
  };
  d0.deserializator = [&blob, &storage](const aether::ObjId& obj_id, uint32_t class_id, uint8_t* data, size_t offset,
                                        size_t size) {
  };
  d0.enumerator = [](const aether::ObjId& obj_id) { };
  A::ptr a = aether::Obj::CreateObjByClassId(d0, A::kId);
  a.Serialize(aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
*/
  
/*    std::filesystem::remove_all("state");

  Root01::ptr root(new Root01(123));
  root.SetId(666);
  {
    B01::ptr b1(new B01(2.71f));
    b1->o_.push_back({});

    B01::ptr b2(new B01(3.14f));
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
    Root01::ptr root;
    root.SetId(666);
    root.SetFlags(aether::ObjFlags::kLoaded);
    root.Load(enumerator, loader);
    B01::ptr r1 = root->o_[0].Clone(enumerator, loader);
    B01::ptr r2 = root->o_[0].Clone(enumerator, loader);
    root->o_.push_back(r2);
    root->o_[0].Load(enumerator, loader);
    r1 = nullptr;
    r2 = nullptr;
    root.Unload();
    int sfd=0;
  }
}

*/

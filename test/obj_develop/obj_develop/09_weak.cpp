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
#include <filesystem>
#include <fstream>
/*#include "../../../aether/obj/obj.h"

using namespace aether;


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


class A : public Obj {
public:
  AETHER_OBJ(A);
  template <typename T> void Serializator(T& s) { }
};
AETHER_IMPL(A);

class App : public Obj {
public:
  AETHER_OBJ(App);
  App() {
    a_ = aether::Obj::CreateObjByClassId(A::kId);
    a_.SetFlags(aether::ObjFlags::kLoadable);
    a_.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    
    a_weak_.SetFlags(aether::ObjFlags::kWeak);
    a_weak_ = a_;
    a_.Unload();
    assert(!a_);
    assert(!a_weak_);
  }
  A::ptr a_;
  A::ptr a_weak_;
  template <typename T> void Serializator(T& s) { s & a_; }
  
  static App::ptr Create(const std::string& path) {
    static const int kObserverRootId = 666;
    root_path_ = path;
    if (1) {
      std::filesystem::remove_all(root_path_);
      App::ptr app{ aether::Obj::CreateObjByClassId(App::kId) };
      app.SetId(kObserverRootId);
      app.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    }
    App::ptr app;
    app.SetId(kObserverRootId);
    app.SetFlags(aether::ObjFlags::kLoaded);
    app.Load(enumerator, loader);
    return app;
  }
  static void Release(App::ptr&& app) {
    app.Serialize(saver, aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
  }
};
AETHER_IMPL(App);

void Loadable() {
  App::ptr app = App::Create("state");
  app->a_.Load(enumerator, loader);
  App::Release(std::move(app));
}
*/

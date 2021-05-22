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

#include "model.h"

#include <iostream>
#include <filesystem>
#include <fstream>


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



AETHER_IMPL(Event1);

AETHER_IMPL(EventTextChanged);

Text::Text() {
#ifdef AETHER_DOC_DEV
  aether::ObjId id1{111};
  presenter_ = { aether::Obj::CreateObjByClassId(TextPresenter::kId, id1) };
  string_ = "Some initial text";
#endif  // OBSERVER_DEV
}

bool Text::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetId()) {
    case EventTextChanged::kId:
      string_ = EventTextChanged::ptr(event)->inserted_text_;
      presenter_->PushEvent(event);
      return true;
    default:
      return aether::Obj::OnEvent(event);
  }
}
AETHER_IMPL(Text);

DocWindow::DocWindow() {
#ifdef AETHER_DOC_DEV
  aether::ObjId id1{ 789 };
  presenter_ = { aether::Obj::CreateObjByClassId(DocWindowPresenter::kId, id1) };
  text_ = { aether::Obj::CreateObjByClassId(Text::kId) };
  text_->presenter_->text_ = text_;
#endif  // OBSERVER_DEV
}

bool DocWindow::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetId()) {
  default:
    return aether::Obj::OnEvent(event);
  }
}
template <typename T> void DocWindow::Serializator(T& s) {
  s & presenter_ & text_;
}

void DocWindow::OnLoaded() {
}

AETHER_IMPL(DocWindow);


Main::Main() {
#ifdef AETHER_DOC_DEV
  id_ = 123;
  aether::ObjId id1{456};
  presenter_ = { aether::Obj::CreateObjByClassId(MainPresenter::kId, id1) };
  doc_window_prefab_ = { aether::Obj::CreateObjByClassId(DocWindow::kId) };
  doc_window_prefab_.SetFlags(aether::ObjFlags::kLoadable);
  doc_window_prefab_->presenter_->doc_window_ = doc_window_prefab_;
//  text_ = { aether::Obj::CreateObjByClassId(Text::kId) };
//  text_->presenter_->text_ = text_;
#endif  // AETHER_DOC_DEV
}

template <typename T> void Main::Serializator(T& s) {
  s & doc_window_prefab_ & doc_windows_ & presenter_;
}

void Main::OnLoaded() {
}

bool Main::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetId()) {
  case Event1::kId:
    //doc_window_prefab_.Load(enumerator, loader);
    doc_windows_.push_back(doc_window_prefab_.Clone(enumerator, loader));
    presenter_->PushEvent(event);
    return true;
  default:
    return aether::Obj::OnEvent(event);
  }
}
AETHER_IMPL(Main);

App::App() {
#ifdef AETHER_DOC_DEV
  main_ = { aether::Obj::CreateObjByClassId(Main::kId) };
  main_->presenter_->main_ = main_;
#endif  // AETHER_DOC_DEV
}

template <typename T> void App::Serializator(T& s) { s & main_; }

bool App::OnEvent(const aether::Event::ptr& event) {
  main_->PushEvent(event);
  return false;
}

void App::OnLoaded() {
}
AETHER_IMPL(App);

App::ptr App::Create(const std::string& path) {
  static const int kObserverRootId = 666;
  root_path_ = path;
#ifdef AETHER_DOC_DEV
  {
    std::filesystem::remove_all(root_path_);

    App::ptr app{ aether::Obj::CreateObjByClassId(App::kId) };
    app.SetId(kObserverRootId);
    app.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    storage_to_path_.clear();
  }
#endif  // AETHER_DOC_DEV
  App::ptr app;
  app.SetId(kObserverRootId);
  app.SetFlags(aether::ObjFlags::kLoaded);
  app.Load(enumerator, loader);
  return app;
}

void App::Release(App::ptr&& app) {
  app.Serialize(saver, aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
}

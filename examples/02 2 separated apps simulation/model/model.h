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
#ifndef AETHER_DOC_MODEL_H
#define AETHER_DOC_MODEL_H

#include "../../../aether/obj/obj.h"

class EventTextChanged : public aether::Event {
public:
  AETHER_OBJ(EventTextChanged, Event);
  EventTextChanged() = default;
  EventTextChanged(int p, int n, const std::string& t, const aether::ObjId& presenter_id)
    : cursor_pos_(p), num_symbols_(n), inserted_text_(t), presenter_id_(presenter_id){}
  aether::ObjId presenter_id_;
  int cursor_pos_, num_symbols_;
  std::string inserted_text_;
  template <typename T> void Serializator(T& s) {
    Event::Serializator(s);
    s & cursor_pos_ & num_symbols_ & inserted_text_ & presenter_id_;
  }
};

class Text : public aether::Obj {
public:
  AETHER_OBJ(Text);
  std::vector<aether::Obj::ptr> presenters_;
  std::string string_;
  template <typename T> void Serializator(T& s) { s & presenters_ & string_; }
  bool OnEvent(const aether::Event::ptr& event) {
    switch (event->GetId()) {
    case EventTextChanged::kId:
      string_ = EventTextChanged::ptr(event)->inserted_text_;
      // Broadcast text changes to all presenters
      for (const auto& p : presenters_) p->PushEvent(event);
      return true;
    default:
      return aether::Obj::OnEvent(event);
    }
  }
};

class MainWindowPresenter : public aether::Obj {
public:
  AETHER_OBJ(MainWindowPresenter);
  aether::Obj::ptr main_window_;
  Text::ptr text_;
  template <typename T> void Serializator(T& s) { s & main_window_ & text_; }
};

class MainWindow : public aether::Obj {
public:
  AETHER_OBJ(MainWindow);
  MainWindowPresenter::ptr presenter_;
  template <typename T> void Serializator(T& s) { s & presenter_; }
};

class App : public aether::Obj {
public:
  AETHER_OBJ(App);
  App();
  std::vector<MainWindow::ptr> main_windows_;
  template <typename T> void Serializator(T& s) { s & main_windows_; }

  static App::ptr Create(const std::string& path);
  static void Release(App::ptr&& app);
};

#endif  // AETHER_DOC_MODEL_H

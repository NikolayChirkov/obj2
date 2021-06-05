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
  EventTextChanged(int p, int n, const std::string& t) : cursor_pos_(p), num_symbols_(n), inserted_text_(t) {}
  int cursor_pos_, num_symbols_;
  std::string inserted_text_;
  template <typename T> void Serializator(T& s) {
    Event::Serializator(s);
    s & cursor_pos_ & num_symbols_ & inserted_text_;
  }
};

class MainWindowPresenter;
class Text : public aether::Obj {
public:
  AETHER_OBJ(Text);
  aether::Ptr <MainWindowPresenter> presenter_;
  std::string string_;
  template <typename T> void Serializator(T& s) { s & presenter_ & string_; }
  bool OnEvent(const aether::Event::ptr& event) {
    switch (event->GetId()) {
    case EventTextChanged::kId:
      string_ = EventTextChanged::ptr(event)->inserted_text_;
      aether::Obj::ptr(presenter_)->PushEvent(event);
      return true;
    default:
      return aether::Obj::OnEvent(event);
    }
  }
};

class MainWindow;
class MainWindowPresenter : public aether::Obj {
public:
  AETHER_OBJ(MainWindowPresenter);
  MainWindowPresenter();
  aether::Ptr<MainWindow> main_window_;
  Text::ptr text_;
  template <typename T> void Serializator(T& s) { s & main_window_ & text_; }
  virtual void OnTextChanged(int p, int n, const std::string& t) {
    text_->PushEvent(new EventTextChanged{ p, n, t });
  }
};

class MainWindow : public aether::Obj {
public:
  AETHER_OBJ(MainWindow);
  MainWindow();
  aether::Ptr <MainWindowPresenter> presenter_;
  template <typename T> void Serializator(T& s) { s & presenter_; }
};

class App : public aether::Obj {
public:
  AETHER_OBJ(App);
  App();
  MainWindow::ptr main_window_;
  template <typename T> void Serializator(T& s) { s & main_window_; }

  static App::ptr Create(const std::string& path, const std::string& subpath);
  static void Release(App::ptr&& app);
};

#endif  // AETHER_DOC_MODEL_H

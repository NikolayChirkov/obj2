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

class Event1 : public aether::Event {
public:
  AETHER_OBJ(Event1, Event);
  Event1() = default;
  template <typename T> void Serializator(T& s) { Event::Serializator(s); }
};

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


class Text;
class TextPresenter : public aether::Obj {
public:
  AETHER_OBJ(TextPresenter);
  aether::Ptr<Text> text_;
  template <typename T> void Serializator(T& s) { s & text_; }
  virtual void OnTextChanged(int p, int n, const std::string& t) {
    aether::Obj::ptr(text_)->PushEvent(new EventTextChanged{ p, n, t });
  }
};

class Text : public aether::Obj {
public:
  AETHER_OBJ(Text);
  Text();
  TextPresenter::ptr presenter_;
  std::string string_;
  template <typename T> void Serializator(T& s) { s & presenter_ & string_; }
  virtual bool OnEvent(const aether::Event::ptr& event);
};

class DocWindow;
class DocWindowPresenter : public aether::Obj {
public:
  AETHER_OBJ(DocWindowPresenter);
  aether::Ptr<DocWindow> doc_window_;
  template <typename T> void Serializator(T& s) { s & doc_window_; }
};

class DocWindow : public aether::Obj {
public:
  AETHER_OBJ(DocWindow);
  DocWindow();
  DocWindowPresenter::ptr presenter_;
  Text::ptr text_;
  template <typename T> void Serializator(T& s);
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
};

class Main;
class MainPresenter : public aether::Obj {
public:
  AETHER_OBJ(MainPresenter);
  aether::Ptr<Main> main_;
  template <typename T> void Serializator(T& s) { s & main_; }
};

// Main Window with a single button.
class Main : public aether::Obj {
public:
  AETHER_OBJ(Main);
  Main();
  MainPresenter::ptr presenter_;
  DocWindow::ptr doc_window_prefab_;
  std::vector<DocWindow::ptr> doc_windows_;
//  Text::ptr text_;
  template <typename T> void Serializator(T& s);
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
};

class App : public aether::Obj {
public:
  AETHER_OBJ(App);
  App();
  Main::ptr main_;
  template <typename T> void Serializator(T& s);
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();

  static App::ptr Create(const std::string& path);
  static void Release(App::ptr&& app);
};

#endif  // AETHER_DOC_MODEL_H

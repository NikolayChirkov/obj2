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
#include <thread>
#include "../../../aether/obj/obj.h"

/*
#define OBSERVER_DEV
#define OBSERVER_ROOT_ID 666

static std::unordered_map<aether::ObjStorage, std::string> storage_to_path_;

auto saver = [](const std::string& path, aether::ObjStorage storage, const AETHER_OMSTREAM& os){
  std::filesystem::create_directories(std::filesystem::path{"state"} / storage_to_path_[storage]);
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

auto loader = [](const std::string& path, aether::ObjStorage storage, AETHER_IMSTREAM& is){
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};

class EventTimer : public aether::Event {
public:
  AETHER_CLS(EventTimer);
  AETHER_SERIALIZE(EventTimer);
  AETHER_INTERFACES(EventTimer, Event);
  template <typename T> void Serializator(T& s, int flags) { Event::Serializator(s, flags); }
};
AETHER_IMPL(EventTimer);



using LRESULT = int;
using HWND = int;
using UINT = int;
using WPARAM = int;
using LPARAM = int;
#define WM_DISPLAYCHANGE 123
class Window;

class PresenterBase : public aether::Obj {
public:
  AETHER_CLS(PresenterBase);
  AETHER_INTERFACES(PresenterBase);
  AETHER_SERIALIZE(PresenterBase);
  PresenterBase() = default;

  aether::Ptr<Window> window_;
#ifdef OBSERVER_DEV
  PresenterBase(bool) {
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    if (flags & aether::Obj::Serialization::kRefs) s & window_;
  }

  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnDisplayChanged(int w, int h);
};

class Presenter : public PresenterBase {
public:
  AETHER_CLS(Presenter);
  AETHER_SERIALIZE(Presenter);
  AETHER_INTERFACES(Presenter, PresenterBase);
  Presenter() = default;

  aether::Ptr<Window> window_;
#ifdef OBSERVER_DEV
  Presenter(bool) {
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    PresenterBase::Serializator(s, flags);
  }

  virtual void OnLoaded();
  static std::map<HWND, Presenter*> presenters_;
  static LRESULT WndProcInternal(HWND hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam);
};
AETHER_IMPL(Presenter);
std::map<HWND, Presenter*> Presenter::presenters_;


class Window : public aether::Obj {
public:
  AETHER_CLS(Window);
  AETHER_SERIALIZE(Window);
  AETHER_INTERFACES(Window);
  Window() = default;

  int x_, y_;
  int display_w_, display_h_;
  static const int kWidth = 120;
  static const int kHeight = 80;
  Presenter::ptr presenter_;
#ifdef OBSERVER_DEV
  Window(bool) {
    x_ = 123;
    y_ = 345;
    display_w_ = 1920;
    display_h_ = 1080;
    presenter_ = {new Presenter(true)};
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    if (flags & aether::Obj::Serialization::kRefs) s & presenter_;
    if (flags & aether::Obj::Serialization::kData) s & x_ & y_ & display_w_ & display_h_;
  }
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
};
AETHER_IMPL(Window);



class App : public aether::Obj {
public:
  AETHER_CLS(App);
  AETHER_SERIALIZE(App);
  AETHER_INTERFACES(App);
  App() = default;

  Window::ptr window_;
#ifdef OBSERVER_DEV
  App(bool) {
    window_ = {new Window(true)};
    window_->presenter_->window_ = window_;
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    if (flags & aether::Obj::Serialization::kRefs) s & window_;
  }
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
};
AETHER_IMPL(App);


class EventPos : public aether::Event {
public:
  AETHER_CLS(EventPos);
  AETHER_SERIALIZE(EventPos, Event);
  AETHER_INTERFACES(EventPos, Event);
  EventPos() = default;
  EventPos(int x, int y, const aether::Obj::ptr& o) : x_(x), y_(y) {}
  int x_, y_;
  template <typename T> void Serializator(T& s, int flags) {
    Event::Serializator(s, flags);
    if (flags & aether::Obj::Serialization::kData) s & x_ & y_;
  }
};
AETHER_IMPL(EventPos);

class EventDisplayChanged : public aether::Event {
public:
  AETHER_CLS(EventDisplayChanged);
  AETHER_SERIALIZE(EventDisplayChanged, Event);
  AETHER_INTERFACES(EventDisplayChanged, Event);
  EventDisplayChanged() = default;
  EventDisplayChanged(int w, int h) : w_(w), h_(h) {}
  int w_, h_;
  template <typename T> void Serializator(T& s, int flags) {
    Event::Serializator(s, flags);
    if (flags & aether::Obj::Serialization::kData) s & w_ & h_;
  }
};
AETHER_IMPL(EventDisplayChanged);



void PresenterBase::OnDisplayChanged(int w, int h) {
  window_->PushEvent(new EventDisplayChanged{w, h});
}

LRESULT Presenter::WndProcInternal(HWND hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DISPLAYCHANGE: {
      int w = lParam & 0xffff;
      int h = lParam >> 16;
      presenters_[hwnd]->OnDisplayChanged(w, h);
      return 0;
    }
  }
  return -1;
}

void Presenter::OnLoaded() {
  std::cout << "Presenter loaded, x: " << window_->x_ << ", y: " << window_->y_ <<
    ", w: " << window_->display_w_ << ", h: " << window_->display_h_ << "\n";
  const int cur_w = 200;
  const int cur_h = 768;

  // If display resolution has been changed since last app run.
  if (cur_w != window_->display_w_ || cur_h != window_->display_h_) {
    std::cout << "Display changed, w: " << window_->display_w_ << " -> " << cur_w <<
      ", h: " << window_->display_h_ << " -> " << cur_h << "\n";
    EventDisplayChanged::ptr e(new EventDisplayChanged());
    e->w_ = cur_w;
    e->h_ = cur_h;
    window_->PushEvent(e);
  }
}
bool PresenterBase::OnEvent(const aether::Event::ptr& event) {
  switch(event->GetClassId()) {
    case EventTimer::class_id_: {
      static int c=0;
      if (c==1) {
        EventPos::ptr e2(new EventPos());
        e2->x_ = 300;
        e2->y_ = 450;
        window_->PushEvent(e2);
      } else if (c==2) {
        EventDisplayChanged::ptr e(new EventDisplayChanged());
        e->w_ = 650;
        e->h_ = 480;
        window_->PushEvent(e);
      }
      c++;
      return false;
    }
    case EventPos::class_id_: {
      EventPos::ptr e(event);
      std::cout << "Presenter changed, x: " << e->x_ << ", y: " << e->y_ << "\n";
      // TODO: SetWindowPos
      return false;
    }
    default:
      return aether::Obj::OnEvent(event);
  }
}


bool Window::OnEvent(const aether::Event::ptr& event) {
  switch(event->GetClassId()) {
    case EventTimer::class_id_:
      presenter_->PushEvent(event);
      return false;
    case EventPos::class_id_: {
      EventPos::ptr e(event);
      // Ensure the window is complitely inside the display.
      int x = std::max(0, e->x_);
      if (x + kWidth > display_w_) x = display_w_ - kWidth;
      int y = std::max(0, e->y_);
      if (y + kHeight > display_h_) y = display_h_ - kHeight;
      if (x != x_ || y != y_) {
        x_ = x;
        y_ = y;
        std::cout << "Window changed, x: " << x_ << ", y: " << y_ << "\n";
        // Model is changed - notify presenter.
        EventPos::ptr e2(new EventPos());
        e2->x_ = x;
        e2->y_ = y;
        presenter_->PushEvent(e2);
        return true;
      }
      return false;
    }
    case EventDisplayChanged::class_id_: {
      EventDisplayChanged::ptr e(event);
      // If the window is partially or completely outside the desktop with changed resolution
      display_w_ = e->w_;
      display_h_ = e->h_;
      int x = (x_ + kWidth < display_w_) ? x_ : display_w_ - kWidth;
      int y = (y_ + kHeight < display_h_) ? y_ : display_h_ - kHeight;
      if (x != x_ || y != y_) {
        x_ = x;
        y_ = y;
        std::cout << "Window changed, x: " << x_ << ", y: " << y_ << ", w: " << display_w_ << ", h: " << display_h_ << "\n";
        // Model is changed - notify presenter.
        EventPos::ptr e2(new EventPos());
        e2->x_ = x;
        e2->y_ = y;
        presenter_->PushEvent(e2);
        return true;
      } else
        std::cout << "Window changed, w: " << display_w_ << ", h: " << display_h_ << "\n";
      return false;
    }
    default:
      return aether::Obj::OnEvent(event);
  }
}
void Window::OnLoaded() {
  std::cout << "Window loaded, x: " << x_ << ", y: " << y_ << ", w: " << display_w_ << ", h: " << display_h_ << "\n";
}


bool App::OnEvent(const aether::Event::ptr& event) {
  window_->PushEvent(event);
  return false;
}
void App::OnLoaded() {
  std::cout << "App loaded\n";
}


void EventProcessor() {
#ifdef OBSERVER_DEV
  {
    std::filesystem::remove_all("state");

    App::ptr app{new App(true)};
    app.SetId(OBSERVER_ROOT_ID);
    app.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
  }
#endif  // OBSERVER_DEV
  if (1) {
    // Recording...
    App::ptr app;
    app.SetId(OBSERVER_ROOT_ID);
    app.SetFlags(aether::ObjFlags::kLoaded);
    app.Load(loader);
    // Serialize into initial state
    app.Serialize(saver, aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    for (int i = 0; i < 3; i++) {
      std::cout << i << " ---------------------------------\n";
      aether::Event::ptr e = {new EventTimer()};
      app->PushEvent(e);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    //app->events_.Serialize(saver, aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
  } else {
    // Playback
    App::ptr app;
    app.SetId(OBSERVER_ROOT_ID);
    app.SetFlags(aether::ObjFlags::kLoaded);
    app.Load(loader);
//    for (const auto& e : app->events_) {
//      e->obj_->PushEvent(e);
//      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//    }
  }
}
*/

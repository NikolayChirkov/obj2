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
/*#include "../../../aether/obj/obj.h"

#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include "../../../aether/obj/obj.h"
#include <assert.h>
#define REQUIRE assert
void EventProcessor() {}


using namespace aether;
static int version;

class Event : public Obj {
public:
  typedef Ptr<Event> ptr;
  static constexpr uint32_t kClassId = qcstudio::crc32::from_literal("Event").value;
  static constexpr uint32_t kBaseClassId = qcstudio::crc32::from_literal("Obj").value;
  inline static Registrar<Event> registrar_ = Registrar<Event>(kClassId, kBaseClassId);
  virtual uint32_t GetClassId() const { return kClassId; }
  
  virtual void* DynamicCast(uint32_t id) {
    return id == kClassId ? static_cast<Event*>(this) : Obj::DynamicCast(id);
  }
  
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s) {
    AETHER_OMSTREAM os;
    os.custom_ = s.custom_;
    Serializator(os);
    s.custom_->store_facility_(id_, kClassId, os);
    //    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal("Obj").value)
    //      aether::Obj::SerializeBase(s, qcstudio::crc32::from_literal("Obj").value);
  }
  virtual void DeserializeBase(AETHER_IMSTREAM& s) {
    AETHER_IMSTREAM is;
    is.custom_ = s.custom_;
    is.custom_->load_facility_(id_, kClassId, is);
    if (!is.stream_.empty()) Serializator(is);
    //    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal(#BASE).value)
    //      BASE::DeserializeBase(s, qcstudio::crc32::from_literal(#BASE).value);
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ptr& o) {
    if (++s.custom_->cur_depth_ <= s.custom_->max_depth_ && SerializeRef(s, o) == SerializationResult::kWholeObject) {
      o->SerializeBase(s);
    }
    s.custom_->cur_depth_--;
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }
  
  
  virtual ~Event() {}
  template <typename T> void Serializator(T& s) {
    s & pos_ & i_ & o_;
  }
  int pos_;
  int i_;
  Obj::ptr o_;
};


class A_03 : public Obj {
public:
  typedef Ptr<A_03> ptr;
  static constexpr uint32_t kClassId = qcstudio::crc32::from_literal("A_03").value;
  static constexpr uint32_t kBaseClassId = qcstudio::crc32::from_literal("Obj").value;
  inline static Registrar<A_03> registrar_ = Registrar<A_03>(kClassId, kBaseClassId);
  virtual uint32_t GetClassId() const { return kClassId; }
  
  virtual void* DynamicCast(uint32_t id) {
    return id == kClassId ? static_cast<A_03*>(this) : Obj::DynamicCast(id);
  }
  
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s) {
    AETHER_OMSTREAM os;
    os.custom_ = s.custom_;
    Serializator(os);
    s.custom_->store_facility_(id_, kClassId, os);
    //    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal("Obj").value)
    //      aether::Obj::SerializeBase(s, qcstudio::crc32::from_literal("Obj").value);
  }
  virtual void DeserializeBase(AETHER_IMSTREAM& s) {
    AETHER_IMSTREAM is;
    is.custom_ = s.custom_;
    is.custom_->load_facility_(id_, kClassId, is);
    if (!is.stream_.empty()) Serializator(is);
    //    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal(#BASE).value)
    //      BASE::DeserializeBase(s, qcstudio::crc32::from_literal(#BASE).value);
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ptr& o) {
    if (++s.custom_->cur_depth_ <= s.custom_->max_depth_ && SerializeRef(s, o) == SerializationResult::kWholeObject) {
      o->SerializeBase(s);
    }
    s.custom_->cur_depth_--;
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }
  
  
  virtual ~A_03() { }
  template <typename T> void Serializator(T& s) {
    s & i_ & a_;
    EventsSerializator(s);
  }
  std::vector<int> i_;
  std::vector<A_03::ptr> a_;
  
  // Event processing
  int version_ = 0;
  std::deque<Event::ptr> events_;
  template <typename T> void EventsSerializator(T& s) {
    s & version_ & events_;
  }
  bool OnEvent(const Event::ptr& e) {
    version_++;
    if (i_.size() < e->pos_ + 1) i_.resize(e->pos_ + 1);
    i_[e->pos_] = e->i_;
    return true;
  }
};

auto saver03 = [](const aether::ObjId& obj_id, uint32_t class_id, const AETHER_OMSTREAM& os){
  std::filesystem::path dir = std::filesystem::path{"state03"} / std::to_string(version) / obj_id.ToString();
  std::filesystem::create_directories(dir);
  auto p = dir / std::to_string(class_id);
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

auto enumerator03 = [](const aether::ObjId& obj_id){
  std::string path = obj_id.ToString();
  auto p = std::filesystem::path{"state03"} / std::to_string(version) / path;
  std::vector<uint32_t> classes;
  for(auto& f: std::filesystem::directory_iterator(p)) {
    classes.push_back(std::atoi(f.path().filename().c_str()));
  }
  return classes;
};

auto loader03 = [](const aether::ObjId& obj_id, uint32_t class_id, AETHER_IMSTREAM& is){
  std::filesystem::path dir = std::filesystem::path{"state03"} / std::to_string(version) / obj_id.ToString();
  auto p = dir / std::to_string(class_id);
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  if (!f.good()) return;
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};

void PushEvent(int i, int pos, A_03::ptr o) {
  Event::ptr e(o->domain_->CreateObjByClassId(Event::kClassId));
  e->i_ = i;
  e->pos_ = pos;
  e->o_ = o;
  o->events_.push_back(std::move(e));
}

// Applies events to the object to reach the version specified.
void ApplyEvents(A_03::ptr o, int version) {
  while (o->version_ < version && !o->events_.empty()) {
    o->OnEvent(std::move(o->events_.front()));
    o->events_.pop_front();
  }
}

void EventProcessor() {
  std::cout << "\n\n\n";
  std::filesystem::remove_all("state03");
  
  Domain src_domain(nullptr);
  src_domain.store_facility_ = saver03;
  A_03::ptr src(src_domain.CreateObjByClassId(A_03::kClassId, 666));
  src->i_.push_back(0);
  version = 0;
  // Transfer the whole state
  src.Serialize();

  Domain dest_domain(nullptr);
  dest_domain.load_facility_ = loader03;
  dest_domain.enumerate_facility_ = enumerator03;
  A_03::ptr dest;
  dest.SetId(666);
  dest.Load(&dest_domain);
  REQUIRE(!!dest);
  REQUIRE(dest->i_[0] == 0);
  
  PushEvent(1, 1, src);
  PushEvent(2, 2, src);
  ApplyEvents(src, 0);
  REQUIRE(src->version_ == 0);
  REQUIRE(src->events_.size() == 2);
  
  // Transfer events to destination
  {
    std::map<ObjId, std::map<uint32_t, std::vector<uint8_t>>> storage;
    Domain domain(&src_domain);
    domain.store_facility_ = [&src, &storage](const aether::ObjId& obj_id, uint32_t class_id, const AETHER_OMSTREAM& os){
      // Store only events, skip all other objects' bodies and store just references instead.
      if (std::find_if(src->events_.begin(), src->events_.end(), [obj_id](auto o) { return o->id_ == obj_id; }) !=
          src->events_.end()) {
        storage[obj_id][class_id] = os.stream_;
      }
    };
    AETHER_OMSTREAM os;
    os.custom_ = &domain;
    os << src->events_;
    // Transfer stream and storage
    {
      // Use destination domain to link references to existing objects.
      dest_domain.enumerate_facility_ = [&storage](const aether::ObjId& obj_id){
        std::vector<uint32_t> classes(storage[obj_id].size());
        for (const auto& c : storage[obj_id]) classes.push_back(c.first);
        return classes;
      };
      dest_domain.load_facility_ = [&storage](const aether::ObjId& obj_id, uint32_t class_id, AETHER_IMSTREAM& is){
        is.stream_ = std::move(storage[obj_id][class_id]);
      };

      AETHER_IMSTREAM is;
      is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
      is.custom_ = &dest_domain;
      std::deque<Event::ptr> events;
      is >> events;

      dest->events_.insert(dest->events_.end(), std::make_move_iterator(events.begin()),
                           std::make_move_iterator(events.end()));
    }
  }
  
  // Destination is synchronized with source. Need to apply events.
  REQUIRE(src->version_ == dest->version_);
  REQUIRE(src->events_.size() == 2);
  REQUIRE(dest->events_.size() == 2);
  REQUIRE(src->events_[0]->i_ == dest->events_[0]->i_);
  REQUIRE(src->events_[0]->pos_ == dest->events_[0]->pos_);
  REQUIRE(src->events_[1]->i_ == dest->events_[1]->i_);
  REQUIRE(src->events_[1]->pos_ == dest->events_[1]->pos_);
  
  ApplyEvents(src, 1);
  REQUIRE(src->version_ == 1);
  REQUIRE(src->events_.size() == 1);
}
*/

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
  switch(event->GetId()) {
    case EventTimer::kId: {
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
    case EventPos::kId: {
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
  switch(event->GetId()) {
    case EventTimer::kId:
      presenter_->PushEvent(event);
      return false;
    case EventPos::kId: {
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
    case EventDisplayChanged::kId: {
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

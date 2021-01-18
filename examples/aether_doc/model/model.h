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

class EventTimer : public aether::Event {
public:
  AETHER_OBJECT(EventTimer);
  AETHER_SERIALIZE(EventTimer);
  AETHER_INTERFACES(EventTimer, Event);
  template <typename T> void Serializator(T& s, int flags) { Event::Serializator(s, flags); }
};


class EventPos : public aether::Event {
public:
  AETHER_OBJECT(EventPos);
  AETHER_SERIALIZE(EventPos, Event);
  AETHER_INTERFACES(EventPos, Event);
  EventPos() = default;
  EventPos(int x, int y) : x_(x), y_(y) {}
  int x_, y_;
  template <typename T> void Serializator(T& s, int flags) {
    Event::Serializator(s, flags);
    if (flags & aether::Obj::Serialization::kData) s & x_ & y_;
  }
};

class EventDisplayChanged : public aether::Event {
public:
  AETHER_OBJECT(EventDisplayChanged);
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

class Main;
class MainPresenter : public aether::Obj {
public:
  AETHER_PURE_INTERFACE(MainPresenter);
  AETHER_SERIALIZE(MainPresenter);
  aether::Ptr<Main> main_;
  virtual void OnMove(int x, int y) { aether::Obj::ptr(main_)->PushEvent(new EventPos(x, y)); }
  virtual void OnDisplayChanged(int w, int h) { aether::Obj::ptr(main_)->PushEvent(new EventDisplayChanged{ w, h }); }
  template <typename T> void Serializator(T& s, int flags) { if (flags & aether::Obj::Serialization::kRefs) s & main_; }
};

class Main : public aether::Obj {
public:
  AETHER_OBJECT(Main);
  AETHER_SERIALIZE(Main);
  AETHER_INTERFACES(Main);
  Main();
  MainPresenter::ptr presenter_;
  int x_, y_, w_, h_;
  int display_w_, display_h_;
  template <typename T> void Serializator(T& s, int flags);
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
};

class App : public aether::Obj {
public:
  AETHER_OBJECT(App);
  AETHER_SERIALIZE(App);
  AETHER_INTERFACES(App);
  App();
  Main::ptr main_;
  template <typename T> void Serializator(T& s, int flags);
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();

  static App::ptr Create();
  static void Release(App::ptr&& app);
};

#endif  // AETHER_DOC_MODEL_H

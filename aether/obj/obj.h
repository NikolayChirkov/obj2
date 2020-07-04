/* Copyright 2016 Aether authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef AETHER_OBJ_H_
#define AETHER_OBJ_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <unordered_map>

#include "../../third_party/crc32/crc32.h"

//#ifndef AETHER_OMSTREAM
#include "../../third_party/aether/stream/aether/mstream/mstream.h"
#define AETHER_OMSTREAM aether::omstream
#define AETHER_IMSTREAM aether::imstream
//#endif

namespace aether {

template <class T>
class Ptr {
 public:

  Ptr clone() const {
    AETHER_OMSTREAM os;
    os << *this;
    AETHER_IMSTREAM is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Ptr cloned;
    is >> cloned;
    return cloned;
  }

  template <class T1> Ptr(T1* p) { InitCast(p); }
  template <class T1> Ptr(const Ptr<T1>& p) { InitCast(p.ptr_); }

  Ptr(T* p) { Init(p); }
  Ptr(const Ptr& p) { Init(p.ptr_); }

  template <class T1>
  Ptr& operator = (const Ptr<T1>& p) {
    // The object is the same. Perform comparison of pointers of Obj-classes.
    if (*this == p) {
      return *this;
    }
    release();
    InitCast(p.ptr_);
    return *this;
  }

  Ptr& operator = (const Ptr& p) {
    // The object is the same. It's ok to compare pointers because the class is
    // also the same. Copy to itself also detected here.
    if (ptr_ == p.ptr_) {
      return *this;
    }
    release();
    Init(p.ptr_);
    return *this;
  }

  template <class T1> Ptr(Ptr<T1>&& p) { MoveCast(p); }

  Ptr(Ptr&& p) {
    ptr_ = p.ptr_;
    p.ptr_ = nullptr;
  }

  template <class T1>
  Ptr& operator = (Ptr<T1>&& p) {
    // Moving the same object: release the source. Pointers with different
    // classes so don't compare them.
    if (*this == p) {
      p.release();
      return *this;
    }
    // Another object is comming.
    release();
    MoveCast(p);
    return *this;
  }

  Ptr& operator = (Ptr&& p) {
    // Moving to itself.
    if (this == &p) {
      return *this;
    }
    if (ptr_ == p.ptr_) {
      p.release();
      return *this;
    }
    // Another object is comming.
    release();
    ptr_ = p.ptr_;
    if (ptr_) {
      p.ptr_ = nullptr;
    } else {
      p.release();
    }
    return *this;
  }

  ~Ptr() {
    release();
  }

  void release() {
    if (ptr_ != nullptr) {
      if (--ptr_->reference_count_ == 0) {
        delete ptr_;
      }
      ptr_ = nullptr;
    }
  }

  T* ptr_ = nullptr;
  Ptr() = default;
  operator bool() const { return ptr_ != nullptr; }
  T* operator->() const { return ptr_; }
  T* get() const { return ptr_; }
  template <class T1>
  friend bool operator == (T& p1, T1& p2) { return p1.ptr_ == p2.ptr_; }
  template <class T1>
  friend bool operator != (T& p1, T1& p2) { return !(p1 == p2); }

  void Init(T* p) {
    if (p != nullptr) {
      ptr_ = p;
      ptr_->reference_count_++;
    }
  }

  template <class T1> void InitCast(T1* p) {
    if (p != nullptr) {
      ptr_ = reinterpret_cast<T*>(p->DynamicCast(T::class_id_));
      if (ptr_ != nullptr) {
        ptr_->reference_count_++;
      }
    }
  }

  template <class T1> void MoveCast(Ptr<T1>& p) {
    if (p.ptr_ != nullptr) {
      ptr_ = reinterpret_cast<T*>(p.ptr_->DynamicCast(T::class_id_));
      if (ptr_ != nullptr) {
        p.ptr_ = nullptr;
      } else {
        p.release();
      }
    }
  }
};

class Obj {
 public:
  typedef Ptr<Obj> ptr;
  virtual ~Obj() {}
  virtual void Serialize(AETHER_OMSTREAM& s) {}
  virtual void Deserialize(AETHER_IMSTREAM& s) {}
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const Obj::ptr& o) {
    s << o->GetClassId();
    o->Serialize(s);
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, Obj::ptr& o) {
    uint32_t class_id;
    s >> class_id;
    o = CreateClassById(class_id);
    o->Deserialize(s);
    return s;
  }

  template <class Dummy>
  class Registry {
   public:
    static void RegisterClass(uint32_t id, std::function<Obj*()> factory) {
      static bool initialized = false;
      if (!initialized) {
        initialized = true;
        registry_ = new std::unordered_map<uint32_t, std::function<Obj*()>>();
      }
      if (registry_->find(id) != registry_->end()) {
        throw std::runtime_error("Class name already registered or Crc32 "
                                 "collision detected. Please choose another "
                                 "name for class.");
      }
      (*registry_)[id] = factory;
    }

    static void UnregisterClass(uint32_t id) {
      auto it = registry_->find(id);
      if (it != registry_->end()) {
        registry_->erase(it);
      }
    }

    static Obj* CreateClassById(uint32_t id) {
      auto it = registry_->find(id);
      if (it == registry_->end()) {
        return nullptr;
      }
      return it->second();
    }
  private:
    static std::unordered_map<uint32_t, std::function<Obj*()>>* registry_;
  };
  virtual void* DynamicCast(uint32_t id) = 0;
  static constexpr uint32_t class_id_ = 0;
  virtual uint32_t GetClassId() const = 0;
  std::atomic<int> reference_count_{0};

  static Obj* CreateClassById(uint32_t id) {
    return Registry<void>::CreateClassById(id);
  }

  template<class T, class T1>
  friend bool operator == (const Ptr<T>& p1, const Ptr<T1>& p2) {
    // If one pointer is zero and another is not - not equal.
    if (p1 ^ p2) {
      return false;
    }
    // If both are zero - equal.
    if (!p1 && !p2) {
      return true;
    }
    void* o1 = p1.ptr_->DynamicCast(Obj::class_id_);
    void* o2 = p2.ptr_->DynamicCast(Obj::class_id_);
    return o1 == o2;
  }
  template<class T, class T1>
  friend bool operator != (const Ptr<T>& p1, const Ptr<T1>& p2) {
    return !(p1 == p2);
  }

  template<class T>
  friend bool operator == (const Ptr<T>& p1, const Ptr<T>& p2) {
    return p1.ptr_ == p2.ptr_;
  }
  template<class T>
  friend bool operator != (const Ptr<T>& p1, const Ptr<T>& p2) {
    return !(p1 == p2);
  }

};

// DynamicCast casting
#define AETHER_HEAD(CLS) \
  typedef aether::Ptr<CLS> ptr; \
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); } \
  virtual void Deserialize(AETHER_IMSTREAM& s) { Serializator(s); } \
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const CLS::ptr& o) {\
      s << o->GetClassId(); \
      o->Serialize(s); \
      return s; \
    } \
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, CLS::ptr& o) { \
      uint32_t class_id; \
      s >> class_id; \
      o = CreateClassById(class_id); \
      o->Deserialize(s); \
      return s; \
    } \
  static constexpr uint32_t class_id_ = qcstudio::crc32::from_literal(#CLS).value; \
  static aether::Registrar<CLS> registrar_; \
  virtual uint32_t GetClassId() const { return class_id_; } \
  virtual void* DynamicCast(uint32_t id) { \
      switch (id) { \
        case 0: \
          return static_cast<Obj*>(this);

#define AETHER_CASE1(CLS) case qcstudio::crc32::from_literal(#CLS).value: \
  return static_cast<CLS*>(this);
#define AETHER_CASE2(CLS, CLS1) AETHER_CASE1(CLS) AETHER_CASE1(CLS1)
#define AETHER_CASE3(CLS, CLS1, CLS2) AETHER_CASE2(CLS, CLS1) AETHER_CASE1(CLS2)
#define AETHER_CASE4(CLS, CLS1, CLS2, CLS3) AETHER_CASE3(CLS, CLS1, CLS2) \
  AETHER_CASE1(CLS3)

#define AETHER_DEFINE_CLS1(CLS) AETHER_HEAD(CLS) AETHER_CASE1(CLS) } return 0; }
#define AETHER_DEFINE_CLS2(CLS, CLS1) AETHER_HEAD(CLS) AETHER_CASE2(CLS, CLS1) \
  } return 0; }
#define AETHER_DEFINE_CLS3(CLS, CLS1, CLS2) AETHER_HEAD(CLS) AETHER_CASE3(CLS, \
  CLS1, CLS2) } return 0; }
#define AETHER_DEFINE_CLS4(CLS, CLS1, CLS2, CLS3) AETHER_HEAD(CLS) \
  AETHER_CASE4(CLS, CLS1, CLS2, CLS3) } return 0; }

#define AETHER_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define AETHER_DEFINE_CLS(...) AETHER_VSPP(AETHER_GET_MACRO(__VA_ARGS__, \
  AETHER_DEFINE_CLS4, AETHER_DEFINE_CLS3, AETHER_DEFINE_CLS2, \
  AETHER_DEFINE_CLS1)(__VA_ARGS__))
// VS++ bug
#define AETHER_VSPP(x) x

template <class Dummy>
std::unordered_map<uint32_t, std::function<Obj*()>>*
  Obj::Registry<Dummy>::registry_;

template< class T>
class Registrar {
public:
  Registrar(uint32_t id) {
    Obj::Registry<void>::RegisterClass(id, []{ return new T(); });
  }
};

#define AETHER_IMPLEMENT_CLS(CLS) aether::Registrar<CLS> \
  CLS::registrar_(CLS::class_id_);

#define AETHER_DEFINE_PURE_CLS(CLS) \
  typedef aether::Ptr<CLS> ptr; \
  static const uint32_t class_id_ = qcstudio::crc32::from_literal(#CLS).value; \
  virtual uint32_t GetClassId() const { return class_id_; } \
  virtual void* DynamicCast(uint32_t) { \
    return static_cast<Obj*>(this); } \
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, CLS::ptr o) { \
    Obj::ptr op(o); \
    s << op; \
    return s; \
  } \
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, CLS::ptr& o) { \
    Obj::ptr op; \
    s >> op; \
    o = op; \
    return s; \
  }

}  // namespace aether

#endif  // AETHER_OBJ_H_

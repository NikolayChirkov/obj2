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
#include <unordered_set>

#include "../../third_party/crc32/crc32.h"

//#ifndef AETHER_OMSTREAM
#include "../../third_party/aether/stream/aether/mstream/mstream.h"
namespace aether {
class Instances {
public:
  std::unordered_set<uint32_t> serialized_ids_;
  std::string path_;
  bool Add(uint32_t serialized_id) {
    auto it = serialized_ids_.find(serialized_id);
    if (it != serialized_ids_.end()) {
      return false;
    }
    serialized_ids_.insert(serialized_id);
    return true;
  }
};
}
using AETHER_OMSTREAM = aether::omstream<aether::Instances>;
using AETHER_IMSTREAM = aether::imstream<aether::Instances>;
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

  template<class T1, class T2>
  friend bool operator == (const Ptr<T1>& p1, const Ptr<T2>& p2) {
    // If one pointer is zero and another is not - not equal.
    if (p1 ^ p2) {
      return false;
    }
    // If both are zero - equal.
    if (!p1 && !p2) {
      return true;
    }
    constexpr uint32_t class_id = qcstudio::crc32::from_literal("Obj").value;
    void* o1 = p1.ptr_->DynamicCast(class_id);
    void* o2 = p2.ptr_->DynamicCast(class_id);
    return o1 == o2;
  }
  template<class T1, class T2>
  friend bool operator != (const Ptr<T1>& p1, const Ptr<T2>& p2) {
    return !(p1 == p2);
  }

  template<class T1>
  friend bool operator == (const Ptr<T1>& p1, const Ptr<T1>& p2) {
    return p1.ptr_ == p2.ptr_;
  }
  template<class T1>
  friend bool operator != (const Ptr<T1>& p1, const Ptr<T1>& p2) {
    return !(p1 == p2);
  }

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

class Obj;
template<class T>
void SerializeObj(T& s, const Ptr<Obj>& o);
template<class T>
Ptr<Obj> DeserializeObj(T& s);

#define AETHER_SERIALIZE_(CLS, BASE) \
virtual void Serialize(AETHER_OMSTREAM& s) { \
  AETHER_OMSTREAM cur_obj; \
  Serializator(cur_obj); \
  AETHER_OMSTREAM whole; \
  whole << cur_obj.stream_; \
  if (qcstudio::crc32::from_literal(#BASE).value != \
      qcstudio::crc32::from_literal("Obj").value) { \
    whole << qcstudio::crc32::from_literal(#BASE).value; \
    BASE::Serialize(whole); \
  } \
  s << whole.stream_; \
} \
virtual void Deserialize(AETHER_IMSTREAM& s) { \
  Serializator(s); \
  if (qcstudio::crc32::from_literal(#BASE).value != \
      qcstudio::crc32::from_literal("Obj").value) { \
    uint32_t class_id; \
    uint32_t full_size; \
    uint32_t cur_obj_size; \
    s >> class_id >> full_size >> cur_obj_size; \
    BASE::Deserialize(s); \
  } \
} \
virtual void DeserializeBase(AETHER_IMSTREAM& s, uint32_t id) { \
  if (qcstudio::crc32::from_literal(#CLS).value != id) { \
    BASE::DeserializeBase(s, id); \
  } else { \
    CLS::Deserialize(s); \
  } \
} \
static constexpr uint32_t base_id_ = \
  qcstudio::crc32::from_literal(#BASE).value; \
friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const CLS::ptr& o) { \
  SerializeObj(s, o); \
  return s; \
} \
friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, CLS::ptr& o) { \
  o = DeserializeObj(s); \
  return s; \
}

#define AETHER_SERIALIZE_CLS1(CLS) AETHER_SERIALIZE_(CLS, Obj)
#define AETHER_SERIALIZE_CLS2(CLS, CLS1) AETHER_SERIALIZE_(CLS, CLS1)
#define AETHER_GET_MACRO(_1, _2, NAME, ...) NAME
#define AETHER_SERIALIZE(...) AETHER_VSPP(AETHER_GET_MACRO(__VA_ARGS__, \
AETHER_SERIALIZE_CLS2, AETHER_SERIALIZE_CLS1)(__VA_ARGS__))
// VS++ bug
#define AETHER_VSPP(x) x


#define AETHER_INTERFACES(...) \
  template <class ...> struct ClassList {};\
  void* DynamicCastInternal(uint32_t, ClassList<>) { return nullptr; }\
  template <class C, class ...N> \
  void* DynamicCastInternal(uint32_t i, ClassList<C, N...>) {\
    if (C::class_id_ != i) { \
      return DynamicCastInternal(i, ClassList<N...>()); \
    }\
    return static_cast<C*>(this); \
  } \
  template <class ...N> void* DynamicCastInternal(uint32_t i) {\
    return DynamicCastInternal(i, ClassList<N...>());\
  }\
  virtual void* DynamicCast(uint32_t id) { \
    return DynamicCastInternal<__VA_ARGS__, Obj>(id);\
  }

#define AETHER_OBJECT(CLS) \
  typedef aether::Ptr<CLS> ptr; \
  static aether::Obj::Registrar<CLS> registrar_; \
  static constexpr uint32_t class_id_ = \
      qcstudio::crc32::from_literal(#CLS).value; \
  virtual uint32_t GetClassId() const { return class_id_; }

#define AETHER_PURE_INTERFACE(CLS) \
  AETHER_OBJECT(CLS) \
  AETHER_INTERFACES(CLS)

#define AETHER_IMPLEMENTATION(CLS) \
  aether::Obj::Registrar<CLS> CLS::registrar_(CLS::class_id_, CLS::base_id_);

class Obj {
protected:
  template<class T>
  struct Registrar {
    Registrar(uint32_t id, uint32_t base_id) {
      Obj::Registry<void>::RegisterClass(id, base_id, []{ return new T(); });
    }
  };

public:
  static Obj* GetInstance(uint32_t instance_id) {
    return Registry<void>::GetObject(instance_id);
  }
  static Obj* CreateClassById(uint32_t id, uint32_t instance_id) {
    Obj* o = Registry<void>::GetObject(instance_id);
    if (o) {
      return o;
    }
    o = Registry<void>::CreateClassById(id);
    // The instance is registered in constructor with incorrect id.
    Registry<void>::RemoveObject(o);
    o->instance_id_ = instance_id;
    Registry<void>::AddObject(o);
    return o;
  }

  Obj() {
    // All newly created objects use unique instance id.
    // 0 is reserved for null pointer.
    static uint32_t instance_id = 1;
    while (Registry<void>::GetObject(++instance_id)) {
    }
    instance_id_ = instance_id;
    Registry<void>::AddObject(this);
  }
  virtual ~Obj() {
    Registry<void>::RemoveObject(this);
  }

  AETHER_OBJECT(Obj);
  AETHER_INTERFACES(Obj);
  AETHER_SERIALIZE(Obj);
  template <typename T>
  T& Serializator(T& s) {
    return s;
  }

  uint32_t instance_id_ = 0;
 protected:
  template<class T>
  friend class Ptr;
  std::atomic<int> reference_count_{0};
  friend class TestAccessor;

  template <class Dummy>
  class Registry {
   public:
    static void RegisterClass(uint32_t id, uint32_t base_id, std::function<Obj*()> factory) {
      static bool initialized = false;
      if (!initialized) {
        initialized = true;
        registry_ = new std::unordered_map<uint32_t, std::function<Obj*()>>();
        base_to_derived_ = new std::unordered_map<uint32_t, uint32_t>();
        instances_ = new std::unordered_map<uint32_t, Obj*>();
      }
      if (registry_->find(id) != registry_->end()) {
        throw std::runtime_error("Class name already registered or Crc32 "
                                 "collision detected. Please choose another "
                                 "name for the class.");
      }
      (*registry_)[id] = factory;
      if (base_id != qcstudio::crc32::from_literal("Obj").value) {
        (*base_to_derived_)[base_id] = id;
      }
    }

    static void UnregisterClass(uint32_t id) {
      auto it = registry_->find(id);
      if (it != registry_->end()) {
        registry_->erase(it);
      }
      for (auto it = base_to_derived_->begin(); it != base_to_derived_->end(); ) {
        it = it->second == id ? base_to_derived_->erase(it) : std::next(it);
      }
    }

    static Obj* CreateClassById(uint32_t base_id) {
      uint32_t derived_id = base_id;
      while (true) {
        auto d = base_to_derived_->find(derived_id);
        if (d == base_to_derived_->end() || derived_id == d->second) {
          break;
        }
        derived_id = d->second;
      }
      auto it = registry_->find(derived_id);
      if (it == registry_->end()) {
        return nullptr;
      }
      return it->second();
    }
    
    static Obj* GetObject(uint32_t instance_id) {
      auto it = instances_->find(instance_id);
      if (it == instances_->end()) {
        return nullptr;
      }
      return it->second;
    }
    static void AddObject(Obj* o) {
      (*instances_)[o->instance_id_] = o;
    }
    static void RemoveObject(Obj* o) {
      instances_->erase(instances_->find(o->instance_id_));
    }
  private:
    static std::unordered_map<uint32_t, std::function<Obj*()>>* registry_;
    static std::unordered_map<uint32_t, uint32_t>* base_to_derived_;
    // TODO: synchronize access from multiple threads.
    static std::unordered_map<uint32_t, Obj*>* instances_;
  };
};

template <class Dummy>
std::unordered_map<uint32_t, std::function<Obj*()>>*
  Obj::Registry<Dummy>::registry_;
template <class Dummy>
std::unordered_map<uint32_t, uint32_t>*
  Obj::Registry<Dummy>::base_to_derived_;
template <class Dummy>
std::unordered_map<uint32_t, Obj*>*
  Obj::Registry<Dummy>::instances_;

template<class T>
void SerializeObj(T& s, const Ptr<Obj>& o) {
  if (!o) {
    s << decltype(Obj::instance_id_)(0);
    return;
  }
  s << o->instance_id_;
  if (!s.custom_.Add(o->instance_id_)) {
    // The object is already serialized. Store just a reference.
    return;
  }
  s << o->GetClassId();
  o->Serialize(s);
}

template<class T>
Obj::ptr DeserializeObj(T& s) {
  while(true) {
    uint32_t instance_id;
    s >> instance_id;
    if (instance_id == 0) {
      return nullptr;
    }
    if (!s.custom_.Add(instance_id)) {
      return Obj::GetInstance(instance_id);
    }
    uint32_t base_id;
    uint32_t full_size;
    uint32_t cur_obj_size;
    s >> base_id >> full_size >> cur_obj_size;
    // The stored object is supported by the run-time.
    Obj::ptr o = Obj::CreateClassById(base_id, instance_id);
    if (o) {
      if (o->GetClassId() != base_id) {
        o->DeserializeBase(s, base_id);
      } else {
        o->Deserialize(s);
      }
      return o;
    } else {
      // Skip serialized data of the inherited object.
      // TODO: use seek for performance
      for (int i = 0; i < cur_obj_size; i++) {
        uint8_t c;
        s >> c;
      }
      // object size is included into the full stream, add +4.
      if (full_size == cur_obj_size + sizeof(uint32_t)) {
        return {};
      }
    }
  }

}

}  // namespace aether

#endif  // AETHER_OBJ_H_

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
#include <algorithm>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <unordered_map>

#include "../../third_party/crc32/crc32.h"

//#ifndef AETHER_OMSTREAM
#include "../../third_party/aether/stream/aether/mstream/mstream.h"
namespace aether {
class Domain;
}
using AETHER_OMSTREAM = aether::omstream<aether::Domain*>;
using AETHER_IMSTREAM = aether::imstream<aether::Domain*>;
//#endif

namespace aether {

class InstanceId {
public:
  using Type = uint32_t;
  InstanceId() = default;
  InstanceId(const Type& i, Type flags) : id_(i), flags_(flags) {}
  static Type GenerateUnique() {
    static int i=0;
    return ++i;//std::rand());
  }
  void Invalidate() { id_ = 0; }
  void SetId(Type i) { id_ = i; }
  Type GetId() const { return id_; }
  enum Flags {
    kLoaded = ~(std::numeric_limits<Type>::max() >> 1),
  };
  Type GetFlags() const { return flags_; }
  void SetFlags(Type flags) { flags_ |= flags; }
  void ClearFlags(Type flags) { flags_ &= ~flags; }
  void ClearAndSetFlags(Type flags) { flags_ = flags; }

  bool IsValid() const { return id_ != 0; }
  friend bool operator == (const InstanceId& i1, const InstanceId& i2) {
    return i1.id_ == i2.id_;
  }
  friend bool operator != (const InstanceId& i1, const InstanceId& i2) {
    return !(i1 == i2);
  }
  friend bool operator < (const InstanceId& i1, const InstanceId& i2) {
    return i1.id_ < i2.id_;
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const InstanceId& i) {
    s << (i.id_ | i.flags_);
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, InstanceId& i) {
    s >> i.id_;
    i.flags_ = i.id_ & (~kIdBitMask);
    i.id_ &= kIdBitMask;
    return s;
  }
protected:
  Type id_;
  Type flags_;
  constexpr static Type kIdBitMask = ~(kLoaded);
};

using StoreFacility = std::function<void(const std::string& path, const AETHER_OMSTREAM& os)>;
using LoadFacility = std::function<void(const std::string& path, AETHER_IMSTREAM& is)>;

template <class T>
class Ptr {
 public:
  InstanceId instance_id_;
  void Serialize(StoreFacility s) const;
  void Unload();
  void Load(LoadFacility l);
  Ptr Clone() const;
  Ptr<T> DeepClone() const;

  template <class T1> Ptr(T1* p) {
    InitCast(p);
    if (ptr_) instance_id_ = ptr_->instance_id_;
  }
  template <class T1> Ptr(const Ptr<T1>& p) {
    InitCast(p.ptr_);
    instance_id_ = p.instance_id_;
  }

  Ptr(T* p) {
    Init(p);
    if (ptr_) instance_id_ = ptr_->instance_id_;
  }
  Ptr(const Ptr& p) {
    Init(p.ptr_);
    instance_id_ = p.instance_id_;
  }

  template <class T1>
  Ptr& operator = (const Ptr<T1>& p) {
    // The object is the same. Perform comparison of pointers of Obj-classes.
    if (*this == p) {
      return *this;
    }
    release();
    InitCast(p.ptr_);
    instance_id_ = p.instance_id_;
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
    instance_id_ = p.instance_id_;
    return *this;
  }

  template <class T1> Ptr(Ptr<T1>&& p) {
    MoveCast(p);
  }

  Ptr(Ptr&& p) {
    ptr_ = p.ptr_;
    instance_id_ = p.instance_id_;
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
    instance_id_ = p.instance_id_;
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
      // reference_count_ is set to 0 to resolve cyclic references.
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
      return p1.instance_id_ == p2.instance_id_;
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
    instance_id_ = p.instance_id_;
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
void SerializeObj(T& s, Ptr<Obj> o);
template<class T>
Ptr<Obj> DeserializeObj(T& s);

#define AETHER_SERIALIZE_(CLS, BASE) \
virtual void Serialize(AETHER_OMSTREAM& s) { \
  Serializator(s); \
} \
virtual void Deserialize(AETHER_IMSTREAM& s) { \
  Serializator(s); \
} \
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
  aether::Obj::Registrar<CLS> CLS::registrar_(CLS::class_id_, 0);

class Domain {
public:
  StoreFacility store_facility_;
  LoadFacility load_facility_;
  std::map<InstanceId, Ptr<Obj>> objects_;
  std::map<InstanceId, int> reference_counts_;
  void IncrementReferenceCount(InstanceId instance_id) {
    reference_counts_[instance_id]++;
  }
  inline void ReleaseObjects();

  Ptr<Obj> FindObject(InstanceId instance_id) const {
    auto it = objects_.find(instance_id);
    if (it != objects_.end()) {
      return it->second;
    }
    return {};
  }
  
  void AddObject(Ptr<Obj> o, InstanceId instance_id) {
    objects_[instance_id] = o;
  }
};

class Obj {
protected:
  template<class T>
  struct Registrar {
    Registrar(uint32_t id, uint32_t base_id) {
      Obj::Registry<void>::RegisterClass(id, base_id, []{ return new T(); });
    }
  };

public:
  static Obj* CreateClassById(uint32_t id, InstanceId instance_id) {
    Obj* o = Registry<void>::CreateClassById(id);
    o->instance_id_ = instance_id;
    return o;
  }

  Obj() {
    instance_id_ = {InstanceId::GenerateUnique(), InstanceId::kLoaded};
  }
  virtual ~Obj() {}

  AETHER_OBJECT(Obj);
  AETHER_INTERFACES(Obj);
  AETHER_SERIALIZE(Obj);
  template <typename T>
  void Serializator(T& s) {}

  InstanceId instance_id_;
 protected:
  template<class T> friend class Ptr;
  friend class Domain;
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
    
  private:
    static std::unordered_map<uint32_t, std::function<Obj*()>>* registry_;
    static std::unordered_map<uint32_t, uint32_t>* base_to_derived_;
  };
};

template <class Dummy>
std::unordered_map<uint32_t, std::function<Obj*()>>*
  Obj::Registry<Dummy>::registry_;
template <class Dummy>
std::unordered_map<uint32_t, uint32_t>*
  Obj::Registry<Dummy>::base_to_derived_;


template<class T>
void SerializeObj(T& s, Ptr<Obj> o) {
  if (!o) {
    s << o.instance_id_;
    return;
  }
  s.custom_->IncrementReferenceCount(o->instance_id_);
  s << o->instance_id_;
  // Object is already serialized.
  if (s.custom_->FindObject(o->instance_id_)) {
    return;
  }
  s.custom_->AddObject(o, o->instance_id_);

  AETHER_OMSTREAM os;
  os.custom_ = s.custom_;
  os << o->GetClassId();
  o->Serialize(os);
  s.custom_->store_facility_(std::to_string(o->instance_id_.GetId()), os);
}

template<class T>
Obj::ptr DeserializeObj(T& s) {
  InstanceId instance_id;
  s >> instance_id;
  Obj::ptr o;
  if (!instance_id.IsValid() || !(instance_id.GetFlags() & InstanceId::kLoaded)) {
    o.instance_id_ = instance_id;
    return o;
  }

  // If object is already deserialized.
  o = s.custom_->FindObject(instance_id);
  if (o) {
    return o;
  }

  AETHER_IMSTREAM is;
  is.custom_ = s.custom_;
  s.custom_->load_facility_(std::to_string(instance_id.GetId()), is);
  uint32_t class_id;
  is >> class_id;
  o = Obj::CreateClassById(class_id, instance_id);
  o->instance_id_ = instance_id;
  // Add object to the list of already loaded before deserialization to avoid infinite loop of cyclic references.
  s.custom_->AddObject(o, instance_id);
  o->Deserialize(is);
  return o;
}

template<typename T>
void Ptr<T>::Serialize(StoreFacility store_facility) const {
  Domain domain;
  domain.store_facility_ = store_facility;
  AETHER_OMSTREAM os;
  os.custom_ = &domain;
  os << *this;
}

template<typename T>
void Ptr<T>::Unload() {
  Domain domain;
  domain.store_facility_ = [](const std::string& path, const AETHER_OMSTREAM& os){
  };
  AETHER_OMSTREAM os;
  os.custom_ = &domain;
  os << *this;
  release();
  domain.ReleaseObjects();
}

void Domain::ReleaseObjects() {
  std::vector<Obj*> objects_to_release;
  for (auto it : objects_) {
    // The object's total references count. 2 additional references come with Domain.
    int total_refs = it.second->reference_count_ - 2;
    // The object's reference count within the domain.
    int domain_refs = reference_counts_[it.second->instance_id_];
    if (total_refs == domain_refs) {
      // The object is referenced only within the domain so must be released.
      it.second->reference_count_ = 0;
      objects_to_release.push_back(it.second.ptr_);
    }
  }
  objects_.clear();
  std::for_each(objects_to_release.begin(), objects_to_release.end(), [](auto o){ delete o;});
}

template<typename T>
void Ptr<T>::Load(LoadFacility load_facility) {
  if (*this || !(instance_id_.GetFlags() & InstanceId::kLoaded)) {
    return;
  }
  AETHER_IMSTREAM is;
  aether::Domain domain;
  domain.load_facility_ = load_facility;
  is.custom_ = &domain;
  AETHER_OMSTREAM os;
  os << InstanceId{instance_id_.GetId(), InstanceId::kLoaded};
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  is >> *this;
}

template<typename T> Ptr<T> Ptr<T>::Clone() const {
  if (*this) {
    // Clone loaded object.
    AETHER_IMSTREAM s;
    Serialize([&s](const std::string& path, const AETHER_OMSTREAM& os){
      s.stream_ = os.stream_;
    });
    Obj::ptr o;
    o.instance_id_ = {InstanceId::GenerateUnique(), InstanceId::kLoaded};
    o.Load([&s](const std::string& path, AETHER_IMSTREAM& is){
      is = s;
    });
    return o;
  }
  return {};
}

template<typename T> Ptr<T> Ptr<T>::DeepClone() const {
  if (*this) {
    // Clone loaded object with cloning hierarchy.
  }
  return Clone();
}

}  // namespace aether

#endif  // AETHER_OBJ_H_

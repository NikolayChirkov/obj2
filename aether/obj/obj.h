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

#include <algorithm>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include "../../third_party/crc32/crc32.h"

//#ifndef AETHER_OMSTREAM
#include "../../third_party/aether/stream/aether/mstream/mstream.h"
namespace aether {
class Domain;
class Obj;
}
using AETHER_OMSTREAM = aether::omstream<aether::Domain*>;
using AETHER_IMSTREAM = aether::imstream<aether::Domain*>;
//#endif

namespace aether {

class InstanceId {
public:
  using Type = uint32_t;
  InstanceId() = default;
  InstanceId(const Type& i) : id_(i) {}
  static Type GenerateUnique() {
    static int i=0;
    return ++i;//std::rand());
  }
  void Invalidate() { id_ = 0; }
  void SetId(Type i) { id_ = i; }
  Type GetId() const { return id_; }

  bool IsValid() const { return id_ != 0; }
  friend bool operator == (const InstanceId& i1, const InstanceId& i2) { return i1.id_ == i2.id_; }
  friend bool operator != (const InstanceId& i1, const InstanceId& i2) { return !(i1 == i2); }
  friend bool operator < (const InstanceId& i1, const InstanceId& i2) { return i1.id_ < i2.id_; }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const InstanceId& i) {
    s << i.id_;
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, InstanceId& i) {
    s >> i.id_;
    return s;
  }
  
  std::string ToString() const { return std::to_string(GetId()); }
protected:
  Type id_;
};

using StoreFacility = std::function<void(const std::string& path, const AETHER_OMSTREAM& os)>;
using LoadFacility = std::function<void(const std::string& path, AETHER_IMSTREAM& is)>;

template <class T>
class Ptr {
 public:
  // Points to an T or to the placeholder of type Obj.
  T* ptr_;
  Ptr() { Init(nullptr); }
  Ptr(T* p) { Init(p); }
  Ptr(const Ptr& p) { Init(p.ptr_); }
  template <class T1> Ptr(T1* p) { InitCast(p); }
  template <class T1> Ptr(const Ptr<T1>& p) { InitCast(p.ptr_); }
  
  ~Ptr() { Release(); }
  
  Ptr& operator = (const Ptr& p) {
    // The object is the same. It's ok to compare pointers because the class is
    // also the same. Copy to itself also detected here.
    if (ptr_ == p.ptr_) return *this;
    Release();
    Init(p.ptr_);
    return *this;
  }

  template <class T1> Ptr& operator = (const Ptr<T1>& p) {
    // The object is the same. Perform comparison of pointers casted to base Obj.
    if (p.ptr_->DynamicCast(kObjClassId) == ptr_->DynamicCast(kObjClassId)) return *this;
    Release();
    InitCast(p.ptr_);
    return *this;
  }

  template <class T1> Ptr(Ptr<T1>&& p) {
    Init(reinterpret_cast<T*>(p.ptr_->DynamicCast(T::class_id_)));
    p.Release();
    p.Init(nullptr);
  }

  Ptr(Ptr&& p) {
    ptr_ = p.ptr_;
    p.Init(nullptr);
  }

  Ptr& operator = (Ptr&& p) {
    // Moving to itself.
    if (this == &p) return *this;
    if (ptr_ == p.ptr_) {
      p.Release();
      p.Init(nullptr);
      return *this;
    }
    // Another object is comming.
    Release();
    ptr_ = p.ptr_;
    p.Init(nullptr);
    return *this;
  }

  template <class T1> Ptr& operator = (Ptr<T1>&& p) {
    // Moving the same object: release the source. Pointers with different
    // classes so don't compare them.
    if (p.ptr_->DynamicCast(kObjClassId) == ptr_->DynamicCast(kObjClassId)) {
      p.Release();
      p.Init(nullptr);
      return *this;
    }
    // Another object is comming.
    Release();
    // Placeholder also must be moved.
    void* ptr = p.ptr_->DynamicCast(T::class_id_);
    Init(reinterpret_cast<T*>(ptr ? ptr : p.ptr_));
    p.Release();
    p.Init(nullptr);
    return *this;
  }
  
  operator bool() const {
    assert(ptr_);
    return !IsPlaceholder();
  }
  T* get() const {
    assert(ptr_);
    return (IsPlaceholder() ? nullptr : ptr_);
  }
  T* operator->() const { return get(); }
  
  // Different type comparison.
  template <class T1, class T2> friend bool operator == (const Ptr<T1>& p1, const Ptr<T2>& p2) {
    assert(p1.ptr_);
    assert(p2.ptr_);
    void* o1 = p1.ptr_->DynamicCast(kObjClassId);
    void* o2 = p2.ptr_->DynamicCast(kObjClassId);
    return o1 == o2;
  }
  template <class T1, class T2> friend bool operator != (const Ptr<T1>& p1, const Ptr<T2>& p2) { return !(p1 == p2); }
  // Same type comparison.
  template <class T1> friend bool operator == (const Ptr<T1>& p1, const Ptr<T1>& p2) {
    assert(p1.ptr_);
    assert(p2.ptr_);
    return p1.ptr_ == p2.ptr_;
  }
  template <class T1> friend bool operator != (const Ptr<T1>& p1, const Ptr<T1>& p2) { return !(p1 == p2); }

  void SetId(InstanceId::Type i) { ptr_->id_.SetId(i); }
  InstanceId::Type GetId() const { return ptr_->id_.GetId(); }
  uint8_t GetFlags() const { return ptr_->flags_; }
  void SetFlags(uint8_t flags) { ptr_->flags_ = flags; }
  
  void Serialize(StoreFacility s) const;
  void Unload();
  void Load(LoadFacility l);
  Ptr Clone() const;
  

  // Protected section.
  void Init(T* p);
  
  template <class T1> void InitCast(T1* p) { Init(p ? reinterpret_cast<T*>(p->DynamicCast(T::class_id_)) : nullptr); }
  T* NewPlaceholder() const;
  void Release();
  bool IsPlaceholder() const {
    assert(ptr_);
    return ptr_->GetClassId() == kObjClassId;
  }
  static constexpr uint32_t kObjClassId = qcstudio::crc32::from_literal("Obj").value;
};

template <class T, class T1> void SerializeObj(T& s, const Ptr<T1>& o1);
template <class T> Ptr<Obj> DeserializeObj(T& s);

#define AETHER_SERIALIZE_(CLS, BASE) \
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); } \
  virtual void Deserialize(AETHER_IMSTREAM& s) { Serializator(s); } \
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
  template <class ...> struct ClassList {}; void* DynamicCastInternal(uint32_t, ClassList<>) { return nullptr; }\
  template <class C, class ...N> void* DynamicCastInternal(uint32_t i, ClassList<C, N...>) {\
    if (C::class_id_ != i) return DynamicCastInternal(i, ClassList<N...>()); \
    return static_cast<C*>(this); \
  } \
  template <class ...N> void* DynamicCastInternal(uint32_t i) { return DynamicCastInternal(i, ClassList<N...>()); }\
  virtual void* DynamicCast(uint32_t id) { return DynamicCastInternal<__VA_ARGS__, Obj>(id); }

#define AETHER_OBJECT(CLS) \
  typedef aether::Ptr<CLS> ptr; \
  static aether::Obj::Registrar<CLS> registrar_; \
  static constexpr uint32_t class_id_ = qcstudio::crc32::from_literal(#CLS).value; \
  virtual uint32_t GetClassId() const { return class_id_; }

#define AETHER_PURE_INTERFACE(CLS) \
  AETHER_OBJECT(CLS) \
  AETHER_INTERFACES(CLS)

#define AETHER_IMPLEMENTATION(CLS) aether::Obj::Registrar<CLS> CLS::registrar_(CLS::class_id_, 0);

class Domain {
public:
  StoreFacility store_facility_;
  LoadFacility load_facility_;
  std::unordered_map<Obj*, int> objects_;
  bool FindAndAddObject(Obj* o) {
    auto& c = objects_[o];
    c++;
    return c > 1;
  }
};

class Obj {
protected:
  template <class T> struct Registrar {
    Registrar(uint32_t id, uint32_t base_id) {
      Obj::Registry<void>::RegisterClass(id, base_id, []{ return new T(); });
    }
  };

public:
  static Obj* CreateClassById(uint32_t id, InstanceId instance_id) {
    Obj* o = Registry<void>::CreateClassById(id);
    o->id_ = instance_id;
    return o;
  }

  Obj() {
    id_ = InstanceId::GenerateUnique();
    flags_ = Flags::kLoaded;
    if (!Registry<void>::root_) Registry<void>::root_ = this;
  }
  virtual ~Obj() {
    if (Registry<void>::root_ == this) Registry<void>::root_ = nullptr;
    auto it = Registry<void>::all_objects_.find(id_.GetId());
    if (it != Registry<void>::all_objects_.end()) Registry<void>::all_objects_.erase(it);
  }
  
  void AddObject() { Registry<void>::all_objects_[id_.GetId()] = this; }
  
  static Obj* FindObject(InstanceId instance_id) {
    auto it = Registry<void>::all_objects_.find(instance_id.GetId());
    if (it != Registry<void>::all_objects_.end()) return it->second;
    return nullptr;
  }

  AETHER_OBJECT(Obj);
  AETHER_INTERFACES(Obj);
  AETHER_SERIALIZE(Obj);
  template <typename T> void Serializator(T& s) const {}

  InstanceId id_;
  struct Flags {
    uint8_t value_;
    enum { kLoadable = 1, kLoaded = 2 };
    operator uint8_t&() { return value_; }
    Flags(decltype(value_) v) : value_(v) {}
    Flags() : value_(kLoaded) {}
  };
  Flags flags_;
 protected:
  template <class Dummy> class Registry {
  public:
    static void RegisterClass(uint32_t id, uint32_t base_id, std::function<Obj*()> factory) {
      static bool initialized = false;
      if (!initialized) {
        initialized = true;
        registry_ = new std::unordered_map<uint32_t, std::function<Obj*()>>();
        base_to_derived_ = new std::unordered_map<uint32_t, uint32_t>();
      }
      if (registry_->find(id) != registry_->end()) {
        throw std::runtime_error("Class name already registered or Crc32 collision detected. Please choose another "
                                 "name for the class.");
      }
      (*registry_)[id] = factory;
      if (base_id != qcstudio::crc32::from_literal("Obj").value) (*base_to_derived_)[base_id] = id;
    }
    
    static void UnregisterClass(uint32_t id) {
      auto it = registry_->find(id);
      if (it != registry_->end()) registry_->erase(it);
      for (auto it = base_to_derived_->begin(); it != base_to_derived_->end(); ) {
        it = it->second == id ? base_to_derived_->erase(it) : std::next(it);
      }
    }
    
    static Obj* CreateClassById(uint32_t base_id) {
      uint32_t derived_id = base_id;
      while (true) {
        auto d = base_to_derived_->find(derived_id);
        if (d == base_to_derived_->end() || derived_id == d->second) break;
        derived_id = d->second;
      }
      auto it = registry_->find(derived_id);
      if (it == registry_->end()) return nullptr;
      return it->second();
    }
    
    static Obj* root_;
    static std::map<uint32_t, Obj*> all_objects_;
    static bool first_release_;
  private:
    static std::unordered_map<uint32_t, std::function<Obj*()>>* registry_;
    static std::unordered_map<uint32_t, uint32_t>* base_to_derived_;
  };
  template <class T> friend class Ptr;
  friend class Domain;
  int reference_count_ = 0;
  friend class TestAccessor;
};

template <class Dummy> std::unordered_map<uint32_t, std::function<Obj*()>>* Obj::Registry<Dummy>::registry_;
template <class Dummy> std::unordered_map<uint32_t, uint32_t>* Obj::Registry<Dummy>::base_to_derived_;
template <class Dummy> Obj* Obj::Registry<Dummy>::root_ = nullptr;
template <class Dummy> bool Obj::Registry<Dummy>::first_release_ = true;
template <class Dummy> std::map<uint32_t, Obj*> Obj::Registry<Dummy>::all_objects_;



template <class T, class T1> void SerializeObj(T& s, const Ptr<T1>& o) {
  if (!o && !(o.GetFlags() & Obj::Flags::kLoadable)) {
    InstanceId id1;
    id1.SetId(0);
    s << id1 << uint8_t(Obj::Flags{});
    return;
  }
  s << o.ptr_->id_ << uint8_t(o.ptr_->flags_);
  if (!o || s.custom_->FindAndAddObject(o.ptr_)) return;

  AETHER_OMSTREAM os;
  os.custom_ = s.custom_;
  os << o->GetClassId();
  o->Serialize(os);
  s.custom_->store_facility_(o->id_.ToString(), os);
}

template <typename T> void Ptr<T>::Release() {
  assert(ptr_);
  if (Obj::Registry<void>::first_release_) {
    Obj::Registry<void>::first_release_ = false;
    
    // Count all references to all objects which are accessible from this pointer that is going to be released.
    Domain domain;
    domain.store_facility_ = [](const std::string& path, const AETHER_OMSTREAM& os) {};
    AETHER_OMSTREAM os2;
    os2.custom_ = &domain;
    os2 << *this;
    
    std::vector<Obj*> release;
    std::vector<Obj*> keep;
    for (auto it : domain.objects_) {
      if (it.first->reference_count_ == it.second) release.push_back(it.first);
      else keep.push_back(it.first);
    }
    
    // If a candidate for releasing is referenced directly or indirectly by the object that is kept then don't release.
    for (auto k : keep) {
      domain.objects_.clear();
      k->Serialize(os2);
      for (auto it = release.begin(); it != release.end(); ) {
        if (domain.objects_.find(*it) != domain.objects_.end()) it = release.erase(it);
        else ++it;
      }
    }

    // Maually release each object without recursive releasing.
    for (auto r : release) r->reference_count_ = 0;
    for (auto r : release) delete r;
    Obj::Registry<void>::first_release_ = true;
  }
  
  // reference_count_ is set to 0 to resolve cyclic references.
  if (--ptr_->reference_count_ == 0) delete ptr_;
}





template <class T> Obj::ptr DeserializeObj(T& s) {
  InstanceId instance_id;
  uint8_t f;
  s >> instance_id >> f;
  Obj::Flags flags(f);
  if (!instance_id.IsValid()) return {};
  if(!(flags & Obj::Flags::kLoaded)) {
    Obj::ptr o;
    // Distinguish 'unloaded' from 'nullptr'
    o.ptr_->id_ = instance_id;
    o.ptr_->flags_ = flags;
    return o;
  }

  // If object is already deserialized.
  Obj* obj = Obj::FindObject(instance_id);
  if (obj) return obj;
  
  AETHER_IMSTREAM is;
  is.custom_ = s.custom_;
  s.custom_->load_facility_(instance_id.ToString(), is);
  uint32_t class_id;
  is >> class_id;
  obj = Obj::CreateClassById(class_id, instance_id);
  obj->id_ = instance_id;
  obj->flags_ = flags;
  // Add object to the list of already loaded before deserialization to avoid infinite loop of cyclic references.
  obj->AddObject();
  obj->Deserialize(is);
  return obj;
}

template <typename T> T* Ptr<T>::NewPlaceholder() const {
  // The pointer is visible as nullptr from the user side. Obj instance is designated to keep instance_id etc.
  auto o = new Obj();
  o->reference_count_ = 1;
  return static_cast<T*>(o);
}

template <typename T> void Ptr<T>::Serialize(StoreFacility store_facility) const {
  Domain domain;
  domain.store_facility_ = store_facility;
  AETHER_OMSTREAM os;
  os.custom_ = &domain;
  os << *this;
}

template <typename T> void Ptr<T>::Unload() {
  // Preserve ID to allow further Load().
  auto temp_id =  ptr_->id_;
  auto temp_flags = ptr_->flags_;
  Release();
  ptr_ = NewPlaceholder();
  ptr_->id_ = temp_id;
  ptr_->flags_ = temp_flags & (~Obj::Flags::kLoaded);
}

template <typename T> void Ptr<T>::Load(LoadFacility load_facility) {
  if (!IsPlaceholder()) return;
  AETHER_IMSTREAM is;
  aether::Domain domain;
  domain.load_facility_ = load_facility;
  is.custom_ = &domain;
  AETHER_OMSTREAM os;
  os << ptr_->id_.GetId() << uint8_t(Obj::Flags(Obj::Flags::kLoadable | Obj::Flags::kLoaded));
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  Obj::Registry<void>::first_release_ = false;
  is >> *this;
  Obj::Registry<void>::first_release_ = true;
}

template <typename T> Ptr<T> Ptr<T>::Clone() const {
  return {};
}

template <typename T> void Ptr<T>::Init(T* p) {
  if (!p || (p->GetClassId() == kObjClassId && !(p->flags_ & Obj::Flags::kLoadable))) {
    // Paceholder means nullptr so a placeholder is referenced only by a single Ptr.
    ptr_ = NewPlaceholder();
  } else {
    ptr_ = p;
    ptr_->reference_count_++;
  }
}

}  // namespace aether

#endif  // AETHER_OBJ_H_

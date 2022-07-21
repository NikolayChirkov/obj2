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

#ifndef AETHER_OBJ_H_
#define AETHER_OBJ_H_

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include "../../third_party/crc32/crc32.h"
#include <random>
#include <limits>
#include <memory>

namespace aether {
class Domain;
class Obj;
}

#include "mstream.h"
#define AETHER_OMSTREAM aether::omstream<std::shared_ptr<aether::Domain>>
#define AETHER_IMSTREAM aether::imstream<std::shared_ptr<aether::Domain>>

namespace aether {

class ObjId {
public:
  using Type = uint32_t;
  ObjId() { Invalidate(); }
  ObjId(const Type& i) : id_(i) {}
  static ObjId GenerateUnique() {
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_int_distribution<std::mt19937::result_type> dist6(1, std::numeric_limits<Type>::max());
    return dist6(rng);
  }
  void Invalidate() { id_ = 0; }
  bool IsValid() const { return id_ != 0; }
  bool operator < (const ObjId& i) const { return id_ < i.id_; }
  bool operator != (const ObjId& i) const { return id_ != i.id_; }
  bool operator == (const ObjId& i) const { return id_ == i.id_; }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ObjId& i) { return s << i.id_; }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ObjId& i) { return s >> i.id_; }

  std::string ToString() const { return std::to_string(id_); }
protected:
  Type id_;
};

class ObjFlags {
  uint8_t value_ = 0;
public:
  enum {
    // The object is not loaded with deserialization. Obj::Load method must be used for loading.
    kUnloadedByDefault = 1,
    kUnloaded = 2,
  };
  operator uint8_t&() { return value_; }
  ObjFlags(decltype(value_) v) : value_(v) {}
  ObjFlags() = default;
};

using StoreFacility = std::function<void(const ObjId& obj_id, uint32_t class_id, const AETHER_OMSTREAM& os)>;
using EnumerateFacility = std::function<std::vector<uint32_t>(const ObjId& obj_id)>;
using LoadFacility = std::function<void(const ObjId& obj_id, uint32_t class_id, AETHER_IMSTREAM& is)>;

template <class T> class Ptr {
 public:
  // If pointer is null then Id, flags are in the pointer.
  T* ptr_;
  ObjId id_;
  ObjFlags flags_;

  // Example: A::ptr a;
  Ptr() { ptr_ = nullptr; }
  // Example: A::ptr a(nullptr);
  // Example: a = nullptr;
  Ptr(T* p) { Init(p); }
  // Example: A::ptr a(other_a_ptr);
  // Example: A::ptr a = other_a_ptr;
  Ptr(const Ptr& p) {
    Init(p.ptr_);
    SetId(p.GetId());
    SetFlags(p.GetFlags());
  }
  // Example: B::ptr b((A*)nullptr);
  template <class T1> Ptr(T1* p) { InitCast(p); }
  // Example: B::ptr b(a);
  // Example: B::ptr b = a;
  template <class T1> Ptr(const Ptr<T1>& p) {
    InitCast(p.ptr_);
    SetId(p.GetId());
    SetFlags(p.GetFlags());
  }
  // Example: A::ptr a2(std::move(a1));
  Ptr(Ptr&& p) {
    ptr_ = p.ptr_;
    SetId(p.GetId());
    SetFlags(p.GetFlags());
    p.id_.Invalidate();
    p.ptr_ = nullptr;
  }
  // Example: B::ptr b(std::move(a));
  template <class T1> Ptr(Ptr<T1>&& p) {
    if (!p) {
      ptr_ = nullptr;
      return;
    }
    T* ptr = static_cast<T*>(p.ptr_->DynamicCast(T::kClassId));
    if (ptr) {
      ptr_ = ptr;
      p.ptr_ = nullptr;
    } else {
      ptr_ = nullptr;
      // Can't cast to this pointer so just release the original pointer.
      p.Release();
    }
    SetId(p.GetId());
    SetFlags(p.GetFlags());
    p.id_.Invalidate();
  }

  // Example: a1 = a2;
  Ptr& operator = (const Ptr& p) {
    // The object is the same. It's ok to compare pointers because the class is
    // also the same. Copy to itself also detected here.
    if (ptr_ != p.ptr_) {
      // Release the old pointer first.
      Release();
      Init(p.ptr_);
      SetId(p.GetId());
      SetFlags(p.GetFlags());
    }
    return *this;
  }
  // Example: b = a;
  template <class T1> Ptr& operator = (const Ptr<T1>& p) {
    // Same object copy.
    if (*this == p) {
      return *this;
    }
    Release();
    InitCast(p.ptr_);
    SetId(p.GetId());
    SetFlags(p.GetFlags());
    return *this;
  }
  // Example: a2 = std::move(a1);
  Ptr& operator = (Ptr&& p) {
    // Don't move to itself.
    if (this != &p) {
      if (ptr_ == p.ptr_) {
        // Pointing to the same object - release the source.
        SetId(p.GetId());
        SetFlags(p.GetFlags());
        p.Release();
      } else {
        // Another object is comming.
        Release();
        ptr_ = p.ptr_;
        SetId(p.GetId());
        SetFlags(p.GetFlags());
        p.ptr_ = nullptr;
        p.id_.Invalidate();
      }
    }
    return *this;
  }
  // Example: b = std::move(a);
  template <class T1> Ptr& operator = (Ptr<T1>&& p) {
    if (!p) {
      SetId(p.GetId());
      SetFlags(p.GetFlags());
      Release();
    } else {
      T* ptr = static_cast<T*>(p.ptr_->DynamicCast(T::kClassId));
      if (!ptr) {
        Release();
        p.Release();
      } else {
        ptr_ = ptr;
        p.ptr_ = nullptr;
        p.id_.Invalidate();
      }
    }
    return *this;
  }

  ~Ptr() { Release(); }

  operator bool() const { return ptr_; }
  T* operator->() const { return ptr_; }

  const ObjId& GetId() const { return ptr_ ? ptr_->id_ : id_; }
  void SetId(const ObjId& i) {
    if (ptr_) ptr_->id_ = i;
    id_ = i;
  }
  ObjFlags GetFlags() const { return ptr_ ? ptr_->flags_ : flags_; }
  void SetFlags(ObjFlags flags) {
    if (ptr_) ptr_->flags_ = flags;
    flags_ = flags;
  }

  void Serialize() const;
  void Unload();
  void Load(std::shared_ptr<Domain> domain);

  // Protected section.
  void Init(T* p);
  template <class T1> void InitCast(T1* p) { Init(p ? static_cast<T*>(p->DynamicCast(T::kClassId)) : nullptr); }
  void Release();
  static constexpr uint32_t kObjClassId = qcstudio::crc32::from_literal("Obj").value;
};

// Same type comparison.
template <class T1> bool operator == (const Ptr<T1>& p1, const Ptr<T1>& p2) { return p1.ptr_ == p2.ptr_; }
template <class T1> bool operator != (const Ptr<T1>& p1, const Ptr<T1>& p2) { return !(p1 == p2); }
// Different type comparison.
template <class T1, class T2> bool operator == (const Ptr<T1>& p1, const Ptr<T2>& p2) {
  // If one or both pointers are zero - not equal.
  if (!p1 || !p2) return false;

  constexpr uint32_t class_id = qcstudio::crc32::from_literal("Obj").value;
  return p1.ptr_->DynamicCast(class_id) == p2.ptr_->DynamicCast(class_id);
}
template <class T1, class T2> bool operator != (const Ptr<T1>& p1, const Ptr<T2>& p2) { return !(p1 == p2); }

enum class SerializationResult { kReferenceOnly, kWholeObject };
template <class T, class T1> SerializationResult SerializeRef(T& s, const Ptr<T1>& o1);
template <class T> Ptr<Obj> DeserializeRef(T& s);


class Domain {
public:
  StoreFacility store_facility_;
  EnumerateFacility enumerate_facility_;
  LoadFacility load_facility_;
  std::vector<std::pair<Obj*, int>> objects_;
  int max_depth_ = std::numeric_limits<int>::max();
  int cur_depth_ = 0;

  Obj* Find(ObjId obj_id) const {
    if (auto it = std::find_if(objects_.begin(), objects_.end(), [&obj_id](auto o) { return o.first->id_ == obj_id; });
        it != objects_.end()) return it->first;
    return nullptr;
  }

  enum class Result { kFound, kAdded };
  Result FindOrAddObject(Obj* o) {
    if (auto it = std::find_if(objects_.begin(), objects_.end(), [o](auto i){ return i.first == o; });
        it != objects_.end()) {
      it->second++;
      return Result::kFound;
    } else {
      objects_.push_back({o, 1});
      return Result::kAdded;
    }
  }
};

class Obj {
protected:
  template <class T> struct Registrar {
    Registrar(uint32_t cls_id, uint32_t base_id) {
      Registry::RegisterClass(cls_id, base_id, []{ return new T(); });
    }
  };

public:
  static Obj* CreateObjByClassId(std::shared_ptr<Domain> domain, uint32_t cls_id, ObjId obj_id) {
    Obj* o = Registry::CreateObjByClassId(cls_id);
    o->id_ = obj_id;
    o->domain_ = domain;
    return o;
  }

  static Obj* CreateObjByClassId(std::shared_ptr<Domain> domain, uint32_t cls_id) {
    Obj* o = Registry::CreateObjByClassId(cls_id);
    o->id_ = ObjId::GenerateUnique();
    o->domain_ = domain;
    return o;
  }

  Obj() = default;
  
  virtual ~Obj() {
    std::remove_if(domain_->objects_.begin(), domain_->objects_.end(), [this](auto i){ return i.first == this; });
  }

  virtual void OnLoaded() {}

  static bool IsLast(uint32_t class_id) {
    return Registry::base_to_derived_->find(class_id) == Obj::Registry::base_to_derived_->end();
  }
  static bool IsExist(uint32_t class_id) {
    return Registry::registry_->find(class_id) != Registry::registry_->end();
  }

  typedef Ptr<Obj> ptr;
  static constexpr uint32_t kClassId = qcstudio::crc32::from_literal("Obj").value;
  static constexpr uint32_t kBaseId = qcstudio::crc32::from_literal("Obj").value;
  virtual uint32_t GetClassId() const { return kClassId; }
  
  virtual void* DynamicCast(uint32_t id) { return id == Obj::kClassId ? static_cast<Obj*>(this) : nullptr; }
  
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s) { }
  virtual void DeserializeBase(AETHER_IMSTREAM& s) { }
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
  template <typename T> void Serializator(T& s) const {}

  ObjId id_;
  ObjFlags flags_;
  int reference_count_ = 0;
  std::shared_ptr<Domain> domain_;

  class Registry {
  public:
    static void RegisterClass(uint32_t cls_id, uint32_t base_id, std::function<Obj*()> factory) {
      static bool initialized = false;
      if (!initialized) {
        initialized = true;
        registry_ = new std::unordered_map<uint32_t, std::function<Obj*()>>();
        base_to_derived_ = new std::unordered_map<uint32_t, std::vector<uint32_t>>();
      }
      if (registry_->find(cls_id) != registry_->end()) {
        throw std::runtime_error("Class name already registered or Crc32 collision detected. Please choose another "
                                 "name for the class.");
      }
      (*registry_)[cls_id] = factory;
      if (base_id != qcstudio::crc32::from_literal("Obj").value) (*base_to_derived_)[base_id].push_back(cls_id);
    }
    
    static void UnregisterClass(uint32_t cls_id) {
      auto it = registry_->find(cls_id);
      if (it != registry_->end()) registry_->erase(it);
      for (auto it = base_to_derived_->begin(); it != base_to_derived_->end(); ) {
        it->second.erase(std::remove(it->second.begin(), it->second.end(), cls_id), it->second.end());
        it = it->second.empty() ? base_to_derived_->erase(it) : std::next(it);
      }
    }
    
    // Creates the most far derivative without ambiguous inheritance.
    static Obj* CreateObjByClassId(uint32_t base_id) {
      uint32_t derived_id = base_id;
      while (true) {
        auto d = base_to_derived_->find(derived_id);
        // If the derived is not found or multiple derives are found.
        if (d == base_to_derived_->end() || d->second.size() > 1) break;
        derived_id = d->second[0];
      }
      auto it = registry_->find(derived_id);
      if (it == registry_->end()) return nullptr;
      return it->second();
    }
    
    inline static bool first_release_ = true;
    inline static bool manual_release_ = false;
    inline static std::unordered_map<uint32_t, std::vector<uint32_t>>* base_to_derived_;
    inline static std::unordered_map<uint32_t, std::function<Obj*()>>* registry_;
  };
};


template <class T, class T1> SerializationResult SerializeRef(T& s, const Ptr<T1>& o) {
  s << o.GetId() << o.GetFlags();
  if (!o) return SerializationResult::kReferenceOnly;
  if (s.custom_->FindOrAddObject(o.ptr_) == Domain::Result::kFound) {
    return SerializationResult::kReferenceOnly;
  }
  return SerializationResult::kWholeObject;
}

template <typename T> void Ptr<T>::Release() {
  if (!ptr_) return;
  if (Obj::Registry::manual_release_) {
    ptr_ = nullptr;
    return;
  }
  if (Obj::Registry::first_release_) {
    Obj::Registry::first_release_ = false;

    // Collect all objects reachable from the releasing pointer. Count references for objects.
    auto domain = std::make_shared<Domain>();
    domain->store_facility_ = [](const ObjId&, uint32_t, const AETHER_OMSTREAM&) {};
    AETHER_OMSTREAM os2;
    os2.custom_ = domain;
    os2 << *this;

    std::vector<Obj*> subgraph;
    std::vector<Obj*> externally_referenced;
    for (auto it : domain->objects_) {
      // Determine if the object is referenced from outside the subgraph.
      if (it.first->reference_count_ == it.second) subgraph.push_back(it.first);
      else externally_referenced.push_back(it.first);
    }

    // Externally referenced objects are not released. Keep all objects referenced by the externally referenced object.
    for (auto k : externally_referenced) {
      domain->objects_.clear();
      k->Serialize(os2);
      for (auto it = subgraph.begin(); it != subgraph.end(); ) {
        if (std::find_if(domain->objects_.begin(), domain->objects_.end(),
                      [it](auto i){ return i.first == *it; }) != domain->objects_.end()) {
          it = subgraph.erase(it);
        } else {
          ++it;
        }
      }
    }

    if (subgraph.empty()) {
      ptr_->reference_count_--;
    } else {
      os2.custom_->max_depth_ = 1;
      for (auto r : subgraph) {
        domain->objects_.clear();
        r->Serialize(os2);
        for (auto k : externally_referenced) {
          if (std::find_if(domain->objects_.begin(), domain->objects_.end(),
                        [k](auto i) { return i.first == k; }) != domain->objects_.end()) {
            k->reference_count_--;
          }
        }
      }
      // Maually release each object without recursive releasing.
      Obj::Registry::manual_release_ = true;
      for (auto r : subgraph) delete r;
      Obj::Registry::manual_release_ = false;
    }
    Obj::Registry::first_release_ = true;
    ptr_ = nullptr;
    return;
  }
  if (--ptr_->reference_count_ == 0) delete ptr_;
  ptr_ = nullptr;
}

template <class T> Obj::ptr DeserializeRef(T& s) {
  ObjId obj_id;
  ObjFlags obj_flags;
  s >> obj_id >> obj_flags;
  if (!obj_id.IsValid()) return {};
  if(obj_flags & (ObjFlags::kUnloadedByDefault | ObjFlags::kUnloaded)) {
    Obj::ptr o;
    // Distinguish 'unloaded' from 'nullptr'
    o.SetId(obj_id);
    o.SetFlags(obj_flags);
    return o;
  }

  // If object is already deserialized.
  Obj* obj = s.custom_->Find(obj_id);
  if (obj) return obj;

  std::vector<uint32_t> classes = s.custom_->enumerate_facility_(obj_id);
  uint32_t class_id = classes[0];
  for (auto c : classes) {
    if (Obj::IsExist(c) && Obj::IsLast(c)) {
      class_id = c;
      break;
    }
  }
  obj = Obj::CreateObjByClassId(s.custom_, class_id, obj_id);
  obj->id_ = obj_id;
  obj->flags_ = obj_flags & (~ObjFlags::kUnloaded);
  // Add object to the list of already loaded before deserialization to avoid infinite loop of cyclic references.
  // Track all deserialized objects.
  assert(s.custom_->FindOrAddObject(obj) == Domain::Result::kAdded);
  obj->DeserializeBase(s);
  return obj;
}


template <typename T> void Ptr<T>::Serialize() const {
  // Create an empty domain to track already serialized objects during the serialization.
  auto domain = std::make_shared<Domain>();
  domain->store_facility_ = ptr_->domain_->store_facility_;
  AETHER_OMSTREAM os;
  os.custom_ = domain;
  os << *this;
}

template <typename T> void Ptr<T>::Unload() {
  if (!ptr_) return;
  id_ = GetId();
  flags_ = GetFlags() | ObjFlags::kUnloaded;
  Release();
}

template <typename T> void Ptr<T>::Load(std::shared_ptr<Domain> domain) {
  if (ptr_) return;
  AETHER_IMSTREAM is;
  is.custom_ = domain;
  // Preserve kUnloadedByDefault flag
  auto flags = GetFlags() & (~ObjFlags::kUnloaded);
  AETHER_OMSTREAM os;
  os << GetId() << (flags & (~ObjFlags::kUnloadedByDefault));
  is.stream_ = std::move(os.stream_);
  Obj::Registry::first_release_ = false;
  is >> *this;
  SetFlags(flags);
  Obj::Registry::first_release_ = true;
  for (auto o : domain->objects_) {
    o.first->OnLoaded();
  }
}

template <typename T> void Ptr<T>::Init(T* p) {
  ptr_ = p;
  if (ptr_) ptr_->reference_count_++;
}

}  // namespace aether

#endif  // AETHER_OBJ_H_

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
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "../../third_party/crc32/crc32.h"

namespace aether {
class Domain;
class Obj;
}  // namespace aether

#include "mstream.h"
#define AETHER_OMSTREAM aether::tomstream<aether::Domain*>
#define AETHER_IMSTREAM aether::timstream<aether::Domain*>

namespace aether {

class ObjId {
 public:
  using Type = uint32_t;
  ObjId() { Invalidate(); }
  ObjId(const Type& i) : id_(i) {}
  static ObjId GenerateUnique() {
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_int_distribution<std::mt19937::result_type> dist6(
        1, std::numeric_limits<Type>::max());
    return dist6(rng);
  }
  void Invalidate() { id_ = 0; }
  bool IsValid() const { return id_ != 0; }
  bool operator<(const ObjId& i) const { return id_ < i.id_; }
  bool operator!=(const ObjId& i) const { return id_ != i.id_; }
  bool operator==(const ObjId& i) const { return id_ == i.id_; }
  friend AETHER_OMSTREAM& operator<<(AETHER_OMSTREAM& s, const ObjId& i) {
    return s << i.id_;
  }
  friend AETHER_IMSTREAM& operator>>(AETHER_IMSTREAM& s, ObjId& i) {
    return s >> i.id_;
  }

  std::string ToString() const { return std::to_string(id_); }

 protected:
  Type id_;
};

class ObjFlags {
  uint8_t value_ = 0;

 public:
  enum {
    // The object is not loaded with deserialization. Obj::Load method must be
    // used for loading.
    kUnloadedByDefault = 1,
    kUnloaded = 2,
  };
  operator uint8_t&() { return value_; }
  ObjFlags(uint8_t v) : value_(v) {}
  ObjFlags() = default;
  friend AETHER_OMSTREAM& operator<<(AETHER_OMSTREAM& s, const ObjFlags& i) {
    return s << i.value_;
  }
  friend AETHER_IMSTREAM& operator>>(AETHER_IMSTREAM& s, ObjFlags& i) {
    return s >> i.value_;
  }
};

using StoreFacility =
    std::function<void(const aether::Domain& domain, const ObjId& obj_id,
                       uint32_t class_id, const AETHER_OMSTREAM& os)>;
using EnumerateFacility = std::function<std::vector<uint32_t>(
    const aether::Domain& domain, const ObjId& obj_id)>;
using LoadFacility =
    std::function<void(const aether::Domain& domain, const ObjId& obj_id,
                       uint32_t class_id, AETHER_IMSTREAM& is)>;

template <class T>
class Ptr {
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
  template <class T1>
  Ptr(T1* p) {
    InitCast(p);
  }
  // Example: B::ptr b(a);
  // Example: B::ptr b = a;
  template <class T1>
  Ptr(const Ptr<T1>& p) {
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
  template <class T1>
  Ptr(Ptr<T1>&& p) {
    if (!p) {
      ptr_ = nullptr;
      return;
    }
    T* ptr = static_cast<T*>(p.ptr_->DynamicCast(T::kClassId));
    if (ptr) {
      ptr_ = ptr;
      SetId(p.GetId());
      SetFlags(p.GetFlags());
      p.ptr_ = nullptr;
    } else {
      ptr_ = nullptr;
      // Can't cast to this pointer so just release the original pointer.
      p.Release();
    }
    p.id_.Invalidate();
  }

  // Example: a1 = a2;
  Ptr& operator=(const Ptr& p) {
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
  template <class T1>
  Ptr& operator=(const Ptr<T1>& p) {
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
  Ptr& operator=(Ptr&& p) {
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
  template <class T1>
  Ptr& operator=(Ptr<T1>&& p) {
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
  // Returns a vector of newly created objects.
  std::vector<Obj*> Load(Domain* domain);

  // Protected section.
  void Init(T* p) {
    ptr_ = p;
    if (ptr_) ptr_->reference_count_++;
  }
  template <class T1>
  void InitCast(T1* p) {
    Init(p ? static_cast<T*>(p->DynamicCast(T::kClassId)) : nullptr);
  }
  void Release();
  static constexpr uint32_t kObjClassId =
      qcstudio::crc32::from_literal("Obj").value;
};

// Same type comparison.
template <class T1>
bool operator==(const Ptr<T1>& p1, const Ptr<T1>& p2) {
  return p1.ptr_ == p2.ptr_;
}
template <class T1>
bool operator!=(const Ptr<T1>& p1, const Ptr<T1>& p2) {
  return !(p1 == p2);
}
// Different type comparison.
template <class T1, class T2>
bool operator==(const Ptr<T1>& p1, const Ptr<T2>& p2) {
  // If one or both pointers are zero - not equal.
  if (!p1 || !p2) return false;

  constexpr uint32_t class_id = qcstudio::crc32::from_literal("Obj").value;
  return p1.ptr_->DynamicCast(class_id) == p2.ptr_->DynamicCast(class_id);
}
template <class T1, class T2>
bool operator!=(const Ptr<T1>& p1, const Ptr<T2>& p2) {
  return !(p1 == p2);
}

enum class SerializationResult { kReferenceOnly, kWholeObject };
template <class T, class T1>
SerializationResult SerializeRef(T& s, const Ptr<T1>& o1);
template <class T>
Ptr<Obj> DeserializeRef(T& s);

class Registry {
 public:
  static void RegisterClass(uint32_t cls_id, uint32_t base_id,
                            std::function<Obj*()> factory) {
    Factories& factories = GetFactories();
    if (factories.find(cls_id) != factories.end()) {
      throw std::runtime_error(
          "Class name already registered or Crc32 collision detected. Please "
          "choose another "
          "name for the class.");
    }
    factories[cls_id] = factory;
    if (base_id != qcstudio::crc32::from_literal("Obj").value)
      GetRelations()[base_id].push_back(cls_id);
  }

  using Relations = std::unordered_map<uint32_t, std::vector<uint32_t>>;
  Relations base_to_derived_;
  using Factories = std::unordered_map<uint32_t, std::function<Obj*()>>;
  Factories factories_;
  Registry() : factories_(GetFactories()), base_to_derived_(GetRelations()) {}

  void UnregisterClass(uint32_t cls_id) {
    if (auto it = factories_.find(cls_id); it != factories_.end())
      factories_.erase(it);
    if (auto it = base_to_derived_.find(cls_id); it != base_to_derived_.end())
      base_to_derived_.erase(it);
    for (auto it = base_to_derived_.begin(); it != base_to_derived_.end();) {
      it->second.erase(
          std::remove(it->second.begin(), it->second.end(), cls_id),
          it->second.end());
      it = it->second.empty() ? base_to_derived_.erase(it) : std::next(it);
    }
  }

  bool IsExisting(uint32_t class_id) const {
    return factories_.find(class_id) != factories_.end();
  }

  int GenerationDistanceInternal(uint32_t base_id, uint32_t derived_id) const {
    auto d = base_to_derived_.find(base_id);
    // The base class is final.
    if (d == base_to_derived_.end()) return -1;
    for (auto& c : d->second) {
      if (derived_id == c) return 1;
      int distance = GenerationDistanceInternal(c, derived_id);
      if (distance >= 0) return distance + 1;
    }
    return -1;
  }

  // Calculates distance from base to derived in generations:
  //  -1 - derived is not inherited directly or indirectly from base or any
  //  class doesn't exist.
  int GenerationDistance(uint32_t base_id, uint32_t derived_id) const {
    if (!IsExisting(base_id) || !IsExisting(derived_id)) return -1;
    if (base_id == derived_id) return 0;
    return GenerationDistanceInternal(base_id, derived_id);
  }

  // Creates the most far derivative without ambiguous inheritance.
  Obj* CreateObj(uint32_t base_id) {
    uint32_t derived_id = base_id;
    while (true) {
      auto d = base_to_derived_.find(derived_id);
      // If the derived is not found or multiple derives are found.
      if (d == base_to_derived_.end() || d->second.size() > 1) break;
      derived_id = d->second[0];
    }
    auto it = factories_.find(derived_id);
    if (it == factories_.end()) return nullptr;
    return it->second();
  }

  static Relations& GetRelations() {
    static Relations relations;
    return relations;
  }
  static Factories& GetFactories() {
    static Factories factories;
    return factories;
  }
};

class Domain {
 public:
  StoreFacility store_facility_;
  EnumerateFacility enumerate_facility_;
  LoadFacility load_facility_;
  std::vector<std::pair<Obj*, int>> objects_;
  int max_depth_ = std::numeric_limits<int>::max();
  int cur_depth_ = 0;
  Domain* parent_;
  Registry registry_;
  inline static bool manual_release_ = false;
  inline static bool first_release_ = true;

  Domain(Domain* parent) : parent_(parent) {}
  inline Obj* CreateObj(uint32_t cls_id, ObjId obj_id);
  inline Obj* CreateObj(uint32_t cls_id);
  bool IsLast(uint32_t class_id) const {
    return registry_.base_to_derived_.find(class_id) ==
           registry_.base_to_derived_.end();
  }
  bool IsExisting(uint32_t class_id) const {
    return registry_.IsExisting(class_id);
  }

  // Search for the object including parent domains.
  Obj* Find(ObjId obj_id) const {
    if (auto it =
            std::find_if(objects_.begin(), objects_.end(),
                         [&obj_id](auto o) { return o.first->id_ == obj_id; });
        it != objects_.end())
      return it->first;
    return parent_ ? parent_->Find(obj_id) : nullptr;
  }

  // Find the object in the current domain only. Tracks reference count of the
  // searches.
  enum class Result { kFound, kAdded };
  Result FindOrAddObject(Obj* o) {
    if (auto it = std::find_if(objects_.begin(), objects_.end(),
                               [o](auto i) { return i.first == o; });
        it != objects_.end()) {
      it->second++;
      return Result::kFound;
    } else {
      objects_.emplace_back(o, 1);
      return Result::kAdded;
    }
  }
  void RemoveObject(Obj* o) {
    objects_.erase(std::remove_if(objects_.begin(), objects_.end(),
                                  [o](auto i) { return i.first == o; }));
  }
};

class Obj {
 protected:
  template <class T>
  struct Registrar {
    Registrar(uint32_t cls_id, uint32_t base_id) {
      Registry::RegisterClass(cls_id, base_id, [] { return new T(); });
    }
  };

 public:
  Obj() = default;

  virtual ~Obj() { domain_->RemoveObject(this); }

  typedef Ptr<Obj> ptr;
  static constexpr uint32_t kClassId =
      qcstudio::crc32::from_literal("Obj").value;
  static constexpr uint32_t kBaseId =
      qcstudio::crc32::from_literal("Obj").value;
  virtual uint32_t GetClassId() const { return kClassId; }

  virtual void* DynamicCast(uint32_t id) {
    return id == Obj::kClassId ? static_cast<Obj*>(this) : nullptr;
  }

  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s) {}
  virtual void DeserializeBase(AETHER_IMSTREAM& s) {}
  friend AETHER_OMSTREAM& operator<<(AETHER_OMSTREAM& s, const ptr& o) {
    if (++s.custom_->cur_depth_ <= s.custom_->max_depth_ &&
        SerializeRef(s, o) == SerializationResult::kWholeObject) {
      o->SerializeBase(s);
    }
    s.custom_->cur_depth_--;
    return s;
  }
  friend AETHER_IMSTREAM& operator>>(AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }
  template <typename T>
  void Serializator(T& s) const {}

  ObjId id_;
  ObjFlags flags_;
  int reference_count_ = 0;
  Domain* domain_;
};

Obj* Domain::CreateObj(uint32_t cls_id, ObjId obj_id) {
  Obj* o = registry_.CreateObj(cls_id);
  o->id_ = obj_id;
  o->domain_ = this;
  objects_.emplace_back(o, 1);
  return o;
}

Obj* Domain::CreateObj(uint32_t cls_id) {
  Obj* o = registry_.CreateObj(cls_id);
  o->id_ = ObjId::GenerateUnique();
  o->domain_ = this;
  objects_.emplace_back(o, 1);
  return o;
}

template <class T, class T1>
SerializationResult SerializeRef(T& s, const Ptr<T1>& o) {
  s << o.GetId() << o.GetFlags();
  if (!o) return SerializationResult::kReferenceOnly;
  if (s.custom_->FindOrAddObject(o.ptr_) == Domain::Result::kFound) {
    return SerializationResult::kReferenceOnly;
  }
  return SerializationResult::kWholeObject;
}

// FIX: Crashes if multiple domains are used. Track domains correctly.
template <typename T>
void Ptr<T>::Release() {
  if (!ptr_) return;
  // The pointer is valid but the object is already released in manual releasing
  // mode. DON'T use 'ptr_'
  if (Domain::manual_release_) {
    ptr_ = nullptr;
    return;
  }
  if (Domain::first_release_) {
    Domain::first_release_ = false;

    // Collect all objects reachable from the releasing pointer. Count
    // references for objects.
    Domain domain(ptr_->domain_);
    domain.store_facility_ = [](const aether::Domain& domain, const ObjId&,
                                uint32_t, const AETHER_OMSTREAM&) {};
    AETHER_OMSTREAM os2;
    os2.custom_ = &domain;
    os2 << *this;

    std::vector<Obj*> subgraph;
    std::vector<Obj*> externally_referenced;
    for (auto it : domain.objects_) {
      // Determine if the object is referenced from outside the subgraph.
      if (it.first->reference_count_ == it.second)
        subgraph.push_back(it.first);
      else
        externally_referenced.push_back(it.first);
    }

    // Externally referenced objects are not released. Keep all objects
    // referenced by the externally referenced object.
    for (auto k : externally_referenced) {
      domain.objects_.clear();
      k->Serialize(os2);
      for (auto it = subgraph.begin(); it != subgraph.end();) {
        if (std::find_if(domain.objects_.begin(), domain.objects_.end(),
                         [it](auto i) { return i.first == *it; }) !=
            domain.objects_.end()) {
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
      // Dereference objects which are referenced externally.
      for (auto r : subgraph) {
        domain.objects_.clear();
        r->Serialize(os2);
        for (auto k : externally_referenced) {
          if (std::find_if(domain.objects_.begin(), domain.objects_.end(),
                           [k](auto i) { return i.first == k; }) !=
              domain.objects_.end()) {
            k->reference_count_--;
          }
        }
      }
      // Maually release each object without recursive releasing.
      Domain::manual_release_ = true;
      for (auto r : subgraph) {
        delete r;
      }
      Domain::manual_release_ = false;
    }
    Domain::first_release_ = true;
    ptr_ = nullptr;
    return;
  }
  if (--ptr_->reference_count_ == 0) delete ptr_;
  ptr_ = nullptr;
}

template <class T>
Obj::ptr DeserializeRef(T& s) {
  ObjId obj_id;
  ObjFlags obj_flags;
  s >> obj_id >> obj_flags;
  if (!obj_id.IsValid()) return {};
  if (obj_flags & (ObjFlags::kUnloadedByDefault | ObjFlags::kUnloaded)) {
    Obj::ptr o;
    // Distinguish 'unloaded' from 'nullptr'
    o.SetId(obj_id);
    o.SetFlags(obj_flags);
    return o;
  }

  // If object is already deserialized.
  Obj* obj = s.custom_->Find(obj_id);
  if (obj) return obj;

  // Find the last class in the inheritance chain.
  std::vector<uint32_t> classes =
      s.custom_->enumerate_facility_(*s.custom_, obj_id);
  // Remove all unsupported classes.
  for (auto it = classes.begin(); it != classes.end();) {
    if (s.custom_->IsExisting(*it))
      ++it;
    else
      it = classes.erase(it);
  }
  // Build inheritance chain.
  std::deque<uint32_t> chain;
  chain.push_back(classes.back());
  classes.resize(classes.size() - 1);
  while (!classes.empty()) {
    for (auto it = classes.begin(); it != classes.end(); ++it) {
      auto c = *it;
      int distance = s.custom_->registry_.GenerationDistance(chain.front(), c);
      if (distance == 1) {
        chain.push_front(c);
        classes.erase(it);
        break;
      }
      distance = s.custom_->registry_.GenerationDistance(c, chain.back());
      if (distance == 1) {
        chain.push_back(c);
        classes.erase(it);
        break;
      }
    }
  }
  // Find the Final class for the most derived class provided and create it.
  for (const auto& f : s.custom_->registry_.factories_) {
    if (s.custom_->IsLast(f.first)) {
      int distance =
          s.custom_->registry_.GenerationDistance(chain.front(), f.first);
      if (distance >= 0) {
        obj = s.custom_->CreateObj(f.first, obj_id);
        obj->flags_ = obj_flags & (~ObjFlags::kUnloaded);
        obj->DeserializeBase(s);
        return obj;
      }
    }
  }
  return nullptr;
}

template <typename T>
void Ptr<T>::Serialize() const {
  // Create an empty domain to track already serialized objects during the
  // serialization.
  Domain domain(ptr_->domain_);
  domain.store_facility_ = ptr_->domain_->store_facility_;
  AETHER_OMSTREAM os;
  os.custom_ = &domain;
  os << *this;
}

template <typename T>
void Ptr<T>::Unload() {
  if (!ptr_) return;
  if (ptr_->reference_count_ > 1)
    throw "Unable to unload the object with multiple references to it";
  id_ = GetId();
  flags_ = GetFlags() | ObjFlags::kUnloaded;
  Release();
}

template <typename T>
std::vector<Obj*> Ptr<T>::Load(Domain* domain) {
  if (ptr_) return {};
  AETHER_IMSTREAM is;
  is.custom_ = domain;
  // Preserve kUnloadedByDefault flag
  ObjFlags flags = GetFlags() & (~ObjFlags::kUnloaded);
  AETHER_OMSTREAM os;
  os << GetId() << ObjFlags(flags & (~ObjFlags::kUnloadedByDefault));
  is.stream_ = std::move(os.stream_);
  Domain::first_release_ = false;
  is >> *this;
  SetFlags(flags);
  Domain::first_release_ = true;
  std::vector<Obj*> created_objects;
  created_objects.resize(domain->objects_.size());
  for (auto o : domain->objects_) {
    created_objects.push_back(o.first);
  }
  return created_objects;
}

}  // namespace aether

#endif  // AETHER_OBJ_H_

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
#include <set>
#include "../../../aether/obj/obj.h"

using namespace aether;

std::set<int> erased;

class A_00 : public Obj {
public:
  //AETHER_CLS(A_00);
  typedef Ptr<A_00> ptr;
  // TODO: use UUID as class ID
  // TODO: rename to kClassId, kBaseClassId, GetClassId
  static constexpr uint32_t kId = qcstudio::crc32::from_literal("A_00").value;
  // TODO: support multiple base classes
  static constexpr uint32_t kBaseId = qcstudio::crc32::from_literal("Obj").value;
  inline static Obj::Registrar<A_00> registrar_ = aether::Obj::Registrar<A_00>(kId, kBaseId);
  virtual uint32_t GetId() const { return kId; }

  //AETHER_INTERFACES(A_00);
  virtual void* DynamicCast(uint32_t id) {
    switch (id) {
      case kId: return static_cast<A_00*>(this);
      case Obj::kId: return static_cast<Obj*>(this);
      default: return nullptr;
    }
  }

  //AETHER_SERIALIZE(A_00);
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s, uint32_t class_id) {
    AETHER_OMSTREAM os;
    os.custom_ = s.custom_;
    A_00::Serializator(os);
    s.custom_->store_facility_(id_, class_id, os);
//    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal("Obj").value)
//      aether::Obj::SerializeBase(s, qcstudio::crc32::from_literal("Obj").value);
  }
  virtual void DeserializeBase(AETHER_IMSTREAM& s, uint32_t class_id) {
    AETHER_IMSTREAM is;
    is.custom_ = s.custom_;
    is.custom_->load_facility_(id_, class_id, is);
    if (!is.stream_.empty()) Serializator(is);
//    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal(#BASE).value)
//      BASE::DeserializeBase(s, qcstudio::crc32::from_literal(#BASE).value);
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ptr& o) {
    if (SerializeRef(s, o)) o->SerializeBase(s, o->GetId());
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }

  
  virtual ~A_00() {
    erased.insert(i_);
  }
  template <typename T> void Serializator(T& s) { s & i_ & a_;}
  int i_ = 123;
  std::vector<A_00::ptr> a_;
};

auto saver = [](const aether::ObjId& obj_id, uint32_t class_id, const AETHER_OMSTREAM& os){
  std::filesystem::path dir = std::filesystem::path{"state"} / obj_id.ToString();
  std::filesystem::create_directories(dir);
  auto p = dir / std::to_string(class_id);
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

auto enumerator = [](const aether::ObjId& obj_id){
  std::string path = obj_id.ToString();
  auto p = std::filesystem::path{"state"} / path;
  std::vector<uint32_t> classes;
  for(auto& f: std::filesystem::directory_iterator(p)) {
    classes.push_back(std::atoi(f.path().filename().c_str()));
  }
  return classes;
};

auto loader = [](const aether::ObjId& obj_id, uint32_t class_id, AETHER_IMSTREAM& is){
  std::filesystem::path dir = std::filesystem::path{"state"} / obj_id.ToString();
  auto p = dir / std::to_string(class_id);
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  if (!f.good()) return;
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};

namespace aether {
class TestAccessor {
public:
  template<class T>
  static void UnregisterClass() {
    aether::Obj::Registry<void>::UnregisterClass(T::kId);
  }
};
}

class V1 : public Obj {
public:
  typedef Ptr<V1> ptr;
  static constexpr uint32_t kId = qcstudio::crc32::from_literal("V1").value;
  static constexpr uint32_t kBaseId = qcstudio::crc32::from_literal("Obj").value;
  inline static Obj::Registrar<V1> registrar_ = aether::Obj::Registrar<V1>(kId, kBaseId);
  virtual uint32_t GetId() const { return kId; }
  
  virtual void* DynamicCast(uint32_t id) {
    switch (id) {
      case kId: return static_cast<V1*>(this);
      case Obj::kId: return static_cast<Obj*>(this);
      default: return nullptr;
    }
  }
  
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s, uint32_t class_id) {
    AETHER_OMSTREAM os;
    os.custom_ = s.custom_;
    V1::Serializator(os);
    s.custom_->store_facility_(id_, class_id, os);
    //    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal("Obj").value)
    //      aether::Obj::SerializeBase(s, qcstudio::crc32::from_literal("Obj").value);
  }
  virtual void DeserializeBase(AETHER_IMSTREAM& s, uint32_t class_id) {
    AETHER_IMSTREAM is;
    is.custom_ = s.custom_;
    is.custom_->load_facility_(id_, class_id, is);
    if (!is.stream_.empty()) Serializator(is);
    //    if (qcstudio::crc32::from_literal("Obj").value != qcstudio::crc32::from_literal(#BASE).value)
    //      BASE::DeserializeBase(s, qcstudio::crc32::from_literal(#BASE).value);
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ptr& o) {
    if (SerializeRef(s, o)) o->SerializeBase(s, o->GetId());
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }
  int i = 11;
  template <typename T>
  T& Serializator(T& s) {
    return s & i;
  }
};

class V2 : public V1 {
public:
  typedef Ptr<V2> ptr;
  static constexpr uint32_t kId = qcstudio::crc32::from_literal("V2").value;
  static constexpr uint32_t kBaseId = qcstudio::crc32::from_literal("V1").value;
  inline static Obj::Registrar<V2> registrar_ = aether::Obj::Registrar<V2>(kId, kBaseId);
  virtual uint32_t GetId() const { return kId; }
  
  virtual void* DynamicCast(uint32_t id) {
    switch (id) {
      case kId: return static_cast<V2*>(this);
      case V1::kId: return static_cast<V1*>(this);
      case Obj::kId: return static_cast<Obj*>(this);
      default: return nullptr;
    }
  }
  
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s, uint32_t class_id) {
    AETHER_OMSTREAM os;
    os.custom_ = s.custom_;
    Serializator(os);
    s.custom_->store_facility_(id_, class_id, os);
    V1::SerializeBase(s, kBaseId);
  }
  virtual void DeserializeBase(AETHER_IMSTREAM& s, uint32_t class_id) {
    AETHER_IMSTREAM is;
    is.custom_ = s.custom_;
    is.custom_->load_facility_(id_, class_id, is);
    if (!is.stream_.empty()) Serializator(is);
    V1::DeserializeBase(s, kBaseId);
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ptr& o) {
    if (SerializeRef(s, o)) o->SerializeBase(s, o->GetId());
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }

  float f = 2.2f;
  template <typename T>
  T& Serializator(T& s) {
    return s & f;
  }
};

class V3 : public V2 {
public:
  typedef Ptr<V3> ptr;
  static constexpr uint32_t kId = qcstudio::crc32::from_literal("V3").value;
  static constexpr uint32_t kBaseId = qcstudio::crc32::from_literal("V2").value;
  inline static Obj::Registrar<V3> registrar_ = aether::Obj::Registrar<V3>(kId, kBaseId);
  virtual uint32_t GetId() const { return kId; }
  
  virtual void* DynamicCast(uint32_t id) {
    switch (id) {
      case kId: return static_cast<V3*>(this);
      case V2::kId: return static_cast<V2*>(this);
      case V1::kId: return static_cast<V1*>(this);
      case Obj::kId: return static_cast<Obj*>(this);
      default: return nullptr;
    }
  }
  
  //AETHER_SERIALIZE(A_00);
  virtual void Serialize(AETHER_OMSTREAM& s) { Serializator(s); }
  virtual void SerializeBase(AETHER_OMSTREAM& s, uint32_t class_id) {
    AETHER_OMSTREAM os;
    os.custom_ = s.custom_;
    Serializator(os);
    s.custom_->store_facility_(id_, class_id, os);
    V2::SerializeBase(s, kBaseId);
  }
  virtual void DeserializeBase(AETHER_IMSTREAM& s, uint32_t class_id) {
    AETHER_IMSTREAM is;
    is.custom_ = s.custom_;
    is.custom_->load_facility_(id_, class_id, is);
    if (!is.stream_.empty()) Serializator(is);
    V2::DeserializeBase(s, kBaseId);
  }
  friend AETHER_OMSTREAM& operator << (AETHER_OMSTREAM& s, const ptr& o) {
    if (SerializeRef(s, o)) o->SerializeBase(s, o->GetId());
    return s;
  }
  friend AETHER_IMSTREAM& operator >> (AETHER_IMSTREAM& s, ptr& o) {
    o = DeserializeRef(s);
    return s;
  }

  std::string s_{"text33"};
  template <typename T>
  T& Serializator(T& s) {
    return s & s_;
  }
};

#include <assert.h>
#define REQUIRE assert

void FF() {
  V1::ptr v1(aether::Obj::CreateObjByClassId(V1::kId, 1));
  v1->i = 111;
  V2::ptr v2(aether::Obj::CreateObjByClassId(V2::kId, 2));
  v2->i = 222;
  v2->f = 2.22f;
  V3::ptr v3(aether::Obj::CreateObjByClassId(V3::kId, 3));
  v3->i = 333;
  v3->f = 3.33f;
  v3->s_ = "text333";
  {
    // Upgrade serialized version: v1 -> v3, v2 -> v3, v3 -> v3
    Domain domain;
    domain.store_facility_ = saver;
    domain.load_facility_ = loader;
    domain.enumerate_facility_ = enumerator;
    AETHER_OMSTREAM os;
    os.custom_ = &domain;
    os << v3 << v2 << v1;
    // assert(os.stream_.size() == 103);
    Domain domain1;
    domain1.store_facility_ = saver;
    domain1.load_facility_ = loader;
    domain1.enumerate_facility_ = enumerator;
    AETHER_IMSTREAM is;
    is.custom_ = &domain1;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Obj::ptr o1, o2, o3;
    is >> o3 >> o2 >> o1;
    REQUIRE(is.stream_.empty());
    V3::ptr v31 = o1;
    REQUIRE(v31);
    REQUIRE(v31->i == 111);
    REQUIRE(v31->f == 2.2f);
    REQUIRE(v31->s_ == "text33");
    V3::ptr v32 = o2;
    REQUIRE(v32);
    REQUIRE(v32->i == 222);
    REQUIRE(v32->f == 2.22f);
    REQUIRE(v32->s_ == "text33");
    V3::ptr v33 = o3;
    REQUIRE(v33);
    REQUIRE(v33->i == 333);
    REQUIRE(v33->f == 3.33f);
    REQUIRE(v33->s_ == "text333");
  }
  {
    // Downgrade serialized version: v3 -> v2, v2 -> v2
    Domain domain;
    domain.store_facility_ = saver;
    domain.load_facility_ = loader;
    domain.enumerate_facility_ = enumerator;
    AETHER_OMSTREAM os;
    os.custom_ = &domain;
    os << v3 << v2;
    //REQUIRE(os.stream_.size() == 87);
    Domain domain1;
    domain1.store_facility_ = saver;
    domain1.load_facility_ = loader;
    domain1.enumerate_facility_ = enumerator;
    AETHER_IMSTREAM is;
    is.custom_ = &domain1;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Obj::ptr o2, o3;
    aether::TestAccessor::UnregisterClass<V3>();
    is >> o3 >> o2;
    REQUIRE(is.stream_.empty());
    V2::ptr v22 = o2;
    REQUIRE(v22);
    REQUIRE(v22->i == 222);
    REQUIRE(v22->f == 2.22f);
    V2::ptr v23 = o3;
    REQUIRE(v23);
    REQUIRE(v23->i == 333);
    REQUIRE(v23->f == 3.33f);
  }
  {
    // Upgrading by loading into the pointer
    Domain domain;
    domain.store_facility_ = saver;
    domain.load_facility_ = loader;
    domain.enumerate_facility_ = enumerator;
    AETHER_OMSTREAM os;
    os.custom_ = &domain;
    os << v2 << v1;
    // REQUIRE(os.stream_.size() == 48);
    Domain domain1;
    domain1.store_facility_ = saver;
    domain1.load_facility_ = loader;
    domain1.enumerate_facility_ = enumerator;
    AETHER_IMSTREAM is;
    is.custom_ = &domain1;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    V2::ptr o2;
    V2::ptr o1;
    is >> o2 >> o1;
    REQUIRE(is.stream_.empty());
    REQUIRE(o2);
    REQUIRE(o2->i == 222);
    REQUIRE(o2->f == 2.22f);
    REQUIRE(o1);
    REQUIRE(o1->i == 111);
    REQUIRE(o1->f == 2.2f);
  }
  
  {
    // Downgrade serialized version: v3 -> v1, v2 -> v1
    Domain domain;
    domain.store_facility_ = saver;
    domain.load_facility_ = loader;
    domain.enumerate_facility_ = enumerator;
    AETHER_OMSTREAM os;
    os.custom_ = &domain;
    os << v3 << v2;
    // REQUIRE(os.stream_.size() == 87);
    Domain domain1;
    domain1.store_facility_ = saver;
    domain1.load_facility_ = loader;
    domain1.enumerate_facility_ = enumerator;
    AETHER_IMSTREAM is;
    is.custom_ = &domain1;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    Obj::ptr o2, o3;
    aether::TestAccessor::UnregisterClass<V2>();
    is >> o3 >> o2;
    REQUIRE(is.stream_.empty());
    V1::ptr v12 = o2;
    REQUIRE(v12);
    REQUIRE(v12->i == 222);
    V1::ptr v13 = o3;
    REQUIRE(v13);
    REQUIRE(v13->i == 333);
  }
}


void Versioning() {
  std::filesystem::remove_all("state");
  FF();
  {
    // Both nullptrs
    A_00::ptr a1;
    A_00::ptr a2(nullptr);
    A_00::ptr a3(a1);
    A_00::ptr a4 = a1;
    a1 = nullptr;
    a1 = a2;
    a1 = a1;
    a3 = std::move(a2);
    a3 = std::move(a3);
    A_00::ptr a5(std::move(a3));
  }
  {
    A_00::ptr a0(aether::Obj::CreateObjByClassId(A_00::kId, 1));
  }
  {
    A_00::ptr root(aether::Obj::CreateObjByClassId(A_00::kId, 666));
    root->i_ = 345;
    root.Serialize(saver);
  }
  {
    A_00::ptr root;
    root.SetId(666);
    root.SetFlags(ObjFlags::kLoaded);
    root.Load(enumerator, loader);
  }
  {
    std::cout << "\n\n\n";
    A_00::ptr a(aether::Obj::CreateObjByClassId(A_00::kId, 1));
    a->i_ = 1;
    A_00::ptr b1{aether::Obj::CreateObjByClassId(A_00::kId, 2)};
    b1->i_ = 2;
    A_00::ptr b2{b1};

    A_00::ptr d1(aether::Obj::CreateObjByClassId(A_00::kId, 3));
    d1->i_ = 3;
    A_00::ptr d2{d1};
    A_00::ptr c{aether::Obj::CreateObjByClassId(A_00::kId, 4)};
    c->i_ = 4;
    
    a->a_.push_back(std::move(b1));
    c->a_.push_back(std::move(b2));
    c->a_.push_back(std::move(d2));
    d1->a_.push_back(std::move(c));

    erased.clear();
    a = nullptr;
    REQUIRE((erased == std::set{1}));
    erased.clear();
    d1 = nullptr;
    REQUIRE((erased == std::set{2, 3, 4}));
  }
  {
    std::cout << "\n\n\n";
    A_00::ptr a(aether::Obj::CreateObjByClassId(A_00::kId, 1));
    a->i_ = 1;
    A_00::ptr b1{aether::Obj::CreateObjByClassId(A_00::kId, 2)};
    b1->i_ = 2;
    A_00::ptr b2{b1};
    
    A_00::ptr d1(aether::Obj::CreateObjByClassId(A_00::kId, 3));
    d1->i_ = 3;
    A_00::ptr d2{d1};
    A_00::ptr c{aether::Obj::CreateObjByClassId(A_00::kId, 4)};
    c->i_ = 4;
    
    a->a_.push_back(std::move(b1));
    c->a_.push_back(std::move(b2));
    c->a_.push_back(std::move(d2));
    d1->a_.push_back(std::move(c));
    
    erased.clear();
    d1 = nullptr;
    REQUIRE((erased == std::set{3, 4}));
    erased.clear();
    a = nullptr;
    REQUIRE((erased == std::set{1, 2}));
  }
  {
    std::cout << "\n\n\n";
    A_00::ptr a1(aether::Obj::CreateObjByClassId(A_00::kId, 1));
    a1->i_ = 1;
    A_00::ptr a2{a1};
    A_00::ptr b{aether::Obj::CreateObjByClassId(A_00::kId, 2)};
    b->i_ = 2;
    
    b->a_.push_back(std::move(a2));
    a1->a_.push_back(std::move(b));
    
    erased.clear();
    a1 = nullptr;
    REQUIRE((erased == std::set{1, 2}));
  }
  {
    std::cout << "\n\n\n";
    A_00::ptr a1(aether::Obj::CreateObjByClassId(A_00::kId, 1));
    a1->i_ = 1;
    A_00::ptr a2{a1};
    A_00::ptr b{aether::Obj::CreateObjByClassId(A_00::kId, 2)};
    b->i_ = 2;
    A_00::ptr c1(aether::Obj::CreateObjByClassId(A_00::kId, 3));
    c1->i_ = 3;
    A_00::ptr c2{c1};

    b->a_.push_back(std::move(a2));
    a1->a_.push_back(std::move(c2));
    c1->a_.push_back(std::move(b));

    erased.clear();
    a1 = nullptr;
    REQUIRE(erased.empty());
    erased.clear();
    c1 = nullptr;
    REQUIRE((erased == std::set{1, 2, 3}));
  }

  {
    std::cout << "\n\n\n";
    A_00::ptr a(aether::Obj::CreateObjByClassId(A_00::kId, 1));
    a->i_ = 1;
    A_00::ptr b1{aether::Obj::CreateObjByClassId(A_00::kId, 2)};
    b1->i_ = 2;
    A_00::ptr b2{b1};
    A_00::ptr b3{b1};
    A_00::ptr c(aether::Obj::CreateObjByClassId(A_00::kId, 3));
    c->i_ = 3;
    
    a->a_.push_back(std::move(b1));
    c->a_.push_back(std::move(b2));
    b3->a_.push_back(std::move(c));
    
    erased.clear();
    a = nullptr;
    REQUIRE((erased == std::set{1}));
    erased.clear();
    b3 = nullptr;
    REQUIRE((erased == std::set{2, 3}));
  }

//  {
//    // nullptr = A*
//    A_00::ptr a1{new A_00()};
//    a1 = a1;
//    A_00* a = new A_00();
//    A_00::ptr a2(a);
//    A_00::ptr a3(a);
//    A_00::ptr a4 = a1;
//    A_00::ptr a5;
//    a5 = std::move(a2);
//    a5 = std::move(a5);
//    A_00::ptr aa;
//    a5 = std::move(aa); // source is not released
//    a5 = std::move(a1); // source is released
//    A_00::ptr a6(std::move(a3));
//    a6 = std::move(aa);
//    int asf=0;
//  }
//  {
//    // Obj::nullptr = A*
//    A_00::ptr a1{new A_00()};
//    B_00::ptr b1{new B_00()};
//    a1 = std::move(b1);
//    int asf=0;
//  }
}

/*#define OBSERVER_DEV
#define OBSERVER_ROOT_ID 666
static std::unordered_map<aether::ObjStorage, std::string> storage_to_path_;

auto saver = [](const aether::ObjId& obj_id, uint32_t class_id, aether::ObjStorage storage, const AETHER_OMSTREAM& os){
  std::filesystem::path dir = std::filesystem::path{"state"} / storage_to_path_[storage] / obj_id.ToString();
  std::filesystem::create_directories(dir);
  auto p = dir / std::to_string(class_id);
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

auto enumerator = [](const aether::ObjId& obj_id, aether::ObjStorage storage){
  std::string path = obj_id.ToString();
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::vector<uint32_t> classes;
  for(auto& f: std::filesystem::directory_iterator(p)) {
    classes.push_back(std::atoi(f.path().filename().c_str()));
  }
  return classes;
};

auto loader = [](const aether::ObjId& obj_id, uint32_t class_id, aether::ObjStorage storage, AETHER_IMSTREAM& is){
  std::filesystem::path dir = std::filesystem::path{"state"} / storage_to_path_[storage] / obj_id.ToString();
  auto p = dir / std::to_string(class_id);
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  if (!f.good()) return;
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};
using namespace aether;

namespace aether {
class TestAccessor {
public:
  template<class T>
  static void UnregisterClass() {
    aether::Obj::Registry<void>::UnregisterClass(T::kId);
  }
};
}

#ifdef OBSERVER_DEV
decltype(aether::Obj::id_) GenId() { return aether::ObjId::GenerateUnique(); }
#endif  // OBSERVER_DEV

class A_05 : public Obj {
public:
  AETHER_OBJ(A_05);
  A_05() { GenId(); }
  int i;
  template <typename T> void Serializator(T& s) {
    s & i;
  }
};
AETHER_IMPL(A_05);

class B1_A_05 : public A_05 {
public:
  AETHER_OBJ(B1_A_05, A_05);
  B1_A_05() { GenId(); }
  float f;
  template <typename T> void Serializator(T& s) {
    s & f;
  }
};
AETHER_IMPL(B1_A_05);

class B2_A_05 : public A_05 {
public:
  AETHER_OBJ(B2_A_05, A_05);
  B2_A_05() { GenId(); }
  std::string s;
  template <typename T> void Serializator(T& ss) {
    ss & s;
  }
};
AETHER_IMPL(B2_A_05);

class C_B2_A_05 : public B2_A_05 {
public:
  AETHER_OBJ(C_B2_A_05, B2_A_05);
  C_B2_A_05() { GenId(); }
  double d;
  template <typename T> void Serializator(T& s) {
    s & d;
  }
};
AETHER_IMPL(C_B2_A_05);

class Root_05 : public Obj {
public:
  AETHER_OBJ(Root_05);
  std::vector<Obj::ptr> v;
  template <typename T> void Serializator(T& s) { s & v; }
};
AETHER_IMPL(Root_05);

void Versioning() {
  std::cout << "Obj " << aether::Obj::kId << "\n";
  std::cout << "Root " << Root_05::kId << "\n";
  std::cout << "A " << A_05::kId << "\n";
  std::cout << "B1 " << B1_A_05::kId << "\n";
  std::cout << "B2 " << B2_A_05::kId << "\n";
  std::cout << "B2C " << C_B2_A_05::kId << "\n";
#ifdef OBSERVER_DEV
  {
    std::filesystem::remove_all("state");
    
    Root_05::ptr root{new Root_05()};
    root.SetId(OBSERVER_ROOT_ID);
    
    root->v.push_back({new A_05()});
    A_05::ptr(root->v.back())->i = 456;

    root->v.push_back({new B1_A_05()});
    B1_A_05::ptr b(root->v.back());
    b->f = 4.56f;
    A_05::ptr(root->v.back())->i = 456;

    root->v.push_back({new B2_A_05()});
    B2_A_05::ptr(root->v.back())->s = "456abc";
    A_05::ptr(root->v.back())->i = 456;

    root->v.emplace_back(new C_B2_A_05());
    C_B2_A_05::ptr(root->v.back())->d = 4.56e3_00;
    B2_A_05::ptr(root->v.back())->s = "456abc";
    A_05::ptr(B2_A_05::ptr(root->v.back()))->i = 456;

    root.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
  }
#endif  // OBSERVER_DEV
  Root_05::ptr root;
  root.SetId(OBSERVER_ROOT_ID);
  root.SetFlags(aether::ObjFlags::kLoaded);
  //TestAccessor::UnregisterClass<V2>();
  root.Load(enumerator, loader);
}

*/





/* =====================================================================================================================
 Multiple interfaces and implementation inheritance example:
class Obj1 {
public:
  virtual ~Obj1() {}
};

class Texture : public Obj1 {
public:
  virtual int* GetHandle() = 0;
  virtual int w() const = 0;
  virtual int h() const = 0;
};

class TextureRead : public virtual Texture {
public:
  virtual int GetPixel(int x, int y) const = 0;
};

class TextureWrite : public virtual Texture {
public:
  virtual void SetPixel(int x, int y, int p) = 0;
};

class TextureImpl : public virtual Texture {
public:
  TextureImpl(int _width, int _height) : width(_width), height(_height) {
    data.resize(width * height, 0);
  }
  virtual int w() const { return width; }
  virtual int h() const { return height; }
  virtual int* GetHandle() { return data.data(); }
protected:
  int width, height;
  std::vector<int> data;
};

class TextureReadImpl : public TextureImpl, public TextureRead {
public:
  TextureReadImpl(int _width, int _height) : TextureImpl(_width, _height) { }
  virtual int GetPixel(int x, int y) const { return data[y * w() + x]; }
};

class TextureWriteImpl : public TextureImpl, public TextureWrite {
public:
  TextureWriteImpl(int _width, int _height) : TextureImpl(_width, _height) { }
  virtual void SetPixel(int x, int y, int p) { data[y * w() + x] = p;  }
};

// class TextureRWImpl : public TextureRead, public TextureWriteImpl {
class TextureRWImpl : public TextureReadImpl, public TextureWrite {
public:
  //  TextureRWDirect(int _width, int _height) : TextureWriteImpl(_width, _height) {}
  //  virtual int GetPixel(int x, int y) const { return data[y * w() + x]; }
  TextureRWImpl(int _width, int _height) : TextureReadImpl(_width, _height) {}
  virtual void SetPixel(int x, int y, int p) { data[y * w() + x] = p;  }
};

void Versioning() {
  auto t0 = new TextureRWImpl(1,1);
  t0->SetPixel(0,0,123);
  auto r1 = t0->GetHandle();
  auto r2 = t0->GetPixel(0,0);
  auto r3 = t0->w();
  auto r4 = t0->h();
  auto t1 = static_cast<Texture*>(t0);
  auto r5 = t1->w();
  auto t2 = static_cast<TextureRead*>(t0);
  auto r6 = t2->w();
  auto t3 = static_cast<Texture*>(t2);
  auto r7 = t3->w();
  int sdf=0;
};
===================================================================================================================== */

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


/*#include <iostream>
#include <fstream>
#include <map>
#include "../../obj/aether/obj/obj.h"

class A2;

class B2 : public aether::Obj {
public:
  AETHER_CLS(B2);
  AETHER_SERIALIZE(B2);
  AETHER_INTERFACES(B2);
  virtual ~B2() { std::cout << "~B: " << f << "\n"; }
  
  float f;
  aether::Ptr<A2> a;
  template <typename T> void Serializator(T& s) {
    s & f & a;
  }
};
AETHER_IMPL(B2);

class A2 : public aether::Obj {
public:
  AETHER_CLS(A2);
  AETHER_SERIALIZE(A2);
  AETHER_INTERFACES(A2);
  virtual ~A2() { std::cout << "~A\n"; }
  int i;
  B2::ptr b;
  template <typename T> void Serializator(T& s) {
    s & i & b;
  }
};
AETHER_IMPL(A2);

void SerializeReferences() {
  aether::Domain domain;
  AETHER_OMSTREAM os;
  {
    os.custom_ = &domain;
    A2::ptr a(new A2());
    a->i = 123;
    B2::ptr b{new B2()};
    b->f = 3.14f;
    a->b = b;
    b->a = a;

    os << a;
    
    size_t s = os.stream_.size();
    for (auto d : os.custom_->data_) {
      s += d.second.size();
    }
    std::cout << "size: " << s << "\n";
    domain.saved_ids_.clear();
  }
  std::cout << "RELEASED\n";
  A2::ptr a;
  {
//    AETHER_IMSTREAM is;
//    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
//    is.custom_ = &domain;
//    is >> a;
  }
}
*/

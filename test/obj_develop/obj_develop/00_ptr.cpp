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
#include "../../../aether/obj/obj.h"

/*using namespace aether;

class A : public Obj {
public:
  AETHER_OBJECT(A);
  AETHER_SERIALIZE(A);
  AETHER_INTERFACES(A);
  A() = default;
  virtual ~A() { std::cout << "~A\n"; }
  template <typename T> void Serializator(T& s, int flags) {}
};
AETHER_IMPLEMENTATION(A);

class B : public Obj {
public:
  AETHER_OBJECT(B);
  AETHER_SERIALIZE(B);
  AETHER_INTERFACES(B);
  B() = default;
  virtual ~B() { std::cout << "~B\n"; }
  template <typename T> void Serializator(T& s, int flags) {}
};
AETHER_IMPLEMENTATION(B);

 void PtrTest() {
   {
     // Both nullptrs
     A::ptr a1;
     A::ptr a2(nullptr);
     A::ptr a3(a1);
     A::ptr a4 = a1;
     a1 = nullptr;
     a1 = a2;
     a1 = a1;
     a3 = std::move(a2);
     a3 = std::move(a3);
     A::ptr a5(std::move(a3));
     
     B::ptr b1(a1);
     B::ptr b2 = a2;
     b2 = a3;
     b1 = std::move(a3);
     B::ptr b3(std::move(a5));
     B::ptr b4(a1.ptr_);
     int asf=0;
   }
   {
     // nullptr = A*
     A::ptr a1{new A()};
     a1 = a1;
     A* a = new A();
     A::ptr a2(a);
     A::ptr a3(a);
     A::ptr a4 = a1;
     A::ptr a5;
     a5 = std::move(a2);
     a5 = std::move(a5);
     A::ptr aa;
     a5 = std::move(aa); // source is not released
     a5 = std::move(a1); // source is released
     A::ptr a6(std::move(a3));
     a6 = std::move(aa);
     int asf=0;
   }
   {
     // Obj::nullptr = A*
     A::ptr a1{new A()};
     B::ptr b1{new B()};
     a1 = std::move(b1);
     int asf=0;
   }
}
 
*/

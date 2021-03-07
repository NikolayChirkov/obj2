# Æether Object
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
![GitHub file size in bytes](https://img.shields.io/github/size/aethernet-io/obj/blob/master/aether/obj/obj.h)

is a C++17 cross-platform architecture heder-only framework for creating lightweight, highly structured, easily supportable, reliable, distributed applications.

The core concept is well-known from other frameworks: an application is represented as a graph, and each node is a serializable class instance.

## Hello, world!

In the example, an application graph is constructed with the application objects **A** and **B** and then serialized into the user-provided storage, for example, into files.

Then the application graph with the whole hierarchy can be restored from that serialized state.

```cpp
#include "aether/obj/obj.h"
class A : public aether::Obj {
public:
  AETHER_OBJ(A);
  template <typename T> void Serializator(T& s, int flags) { s & i_; }
  int i_;
};
AETHER_IMPL(A);

class B : public aether::Obj {
public:
  AETHER_OBJ(B);
  template <typename T> void Serializator(T& s, int flags) { s & objs_; }
  std::vector<A::ptr> objs_;
};
AETHER_IMPL(B);

int main() {
  {
    // Constructing and serializing the application's graph.
    B::ptr o{aether::Obj::CreateObjByClassId(B::kId)};
    o.Serialize([](stream& s) {/*writing files*/}, flags);
  }
  {
    // Loading the application's graph.
    B::ptr o;
    o.Load([](...) {/*enumerate files*/}, [](...) { /*loading files*/});
    // The graph is restored and can be accessed.
    std::cout << "A::i_ = " << o->objs_[0]->i_ << "\n";
  }
}
```

## Key features

*   **Serialization** of the whole application state with a little effort
*   **Hibernating/waking-up** parts of the application’s graph
*   **Upgrade** of the application is just the replacing a part of serialized state
*   **Versioning** of the serialized state and executable is automatic: simplifies upgrade and messaging between application’s instances
*   **Model-Presenter** for cross-platform applications with event-driven model change
*   **Debugging, automated testing**: event-driven model records all state changes and can be reproduced in any platform-specific environment
*   **Code size, performance, reliability**. The application is split into development mode, where the graph is constructed, all intermediate resources are pre-compiled and verified and is just deserialized then from binary.
*   **Distributed** applications just share the part of the graph.


## Example application

A cross-platform (Windows, macOS, iOS, Android) text editor with automatic saving text, window position and size.


- [Object smart pointer](#object-smart-pointer)
- [serializable smart pointer](#serializable-smart-pointer)
- [Development runtime modes](#development-runtime-modes)
- [Serialization](#serialization)
- [Serialize state events](#serialize-state-events)
- [App specific](#app-specific)
- [Header-only, custom streams, exception handling, RAII](#header-only-custom-streams-exception-handling-raii)
- [Multithreading fast serialization and double-buffering](#multithreading-fast-serialization-and-double-buffering)
- [Developing changing resource in run-time](#developing-changing-resource-in-run-time)


## Class casting

Any class of an application is inherited from the Obj class. *AETHER_OBJ(class name)* macro is used to declare all supporting internal functions. An Object is wrapped into the Ptr<T> template class and the pointer type is declared as *MyClass::ptr*. Obj base class contains the references counter.
```cpp
MyClass* ptr = new MyClass();
MyClass::ptr p1(ptr);  // Increments the reference counter = 1.
MyClass::ptr p2(ptr);  // Increments the reference counter = 2.
p1 = nullptr;  // Decrements ref_counter = 1.
p2 = nullptr;  // Decrements ref_counter = 0 and the class instance is deleted.
```
*AETHER_IMPL(ClassName)* should be declared in cpp-file. The macro registers the class factory function.

### Inheritance
Each class inherited from the Obj class supports efficient dynamic poiter downcasting without using C++ RTTI.
```cpp
class A : public aether::Obj {
 public:
  AETHER_OBJ(A);
};
class B : public aether::Obj {
 public:
  AETHER_OBJ(B);
};
aether::Obj::ptr o{new B()};
A::ptr a = o;  // Can't cast to A* so the pointer remains nullptr
B::ptr b = o;  // Resolved to B*
```
The "Diamond problem" can be resolved by the "class A : public **virtual** aether::Obj" inheritance.
If a class supports casting to multiple base classes with multiple inheritance or with chain of base classes then multiple classes can be enumerated in th *AETHER_OBJ(ClassName)* macro.
```cpp
class Base1 : public virtual aether::Obj {
 public:
  AETHER_OBJ(Base1);
};
class Base2 : public virtual aether::Obj {
 public:
  AETHER_OBJ(Base2);
};
class Derived : public Base1, public Base2 {
 public:
  AETHER_OBJ(Derived, Base1, Base2);  // Cuurent class must be listed at the first place.
};
aether::Obj::ptr o{new Derived()};
Base1::ptr b1 = o;
Base2::ptr b2 = o;
```

Interface classes (i.e. not able to be instantiated) are defined with the same way but without AETHER_IMPL macro.

### Inheritance chain
If *aether::Obj::CreateObjByClassId("ClassName")* method is used instead of *new ClassName* then the inheritance chain is evaluated and the last class is instantiated. It is useful for instantiating the interface implementation:
```cpp
class Interface : public virtual aether::Obj {
 public:
  AETHER_OBJ(Interface);
};
class Implementation : public Interface {
 public:
  AETHER_OBJ(Implementation, Interface);  // The inheritance chain is listed here.
};
// The pointer refers to the Implementation class instance.
Interface::ptr interface = aether::Obj::CreateObjByClassId("Interface");
```

## Serialization
Serialization of the application model/state is done with input/output stream that saves an object data, references to other objects. A special *mstream* class is provided but any other custom stream with required functionality can be used. A user-side call-backs implement saving/loading the serialized data, for example as files, or database etc.

### Class state serialization
To avoid possible mistmatches when class members are serialized and deserialized a unified bidirectional method is used:
```cpp
class A : public aether::Obj {
 public:
  AETHER_OBJ(Interface);
  int i_;
  std::vector<std::string> strings_;  
  template <typename T> void Serializator(T& s, int flags) {
    s & i_ & strings_;
  }
  virtual void OnLoaded() {}
};
```
**Serializator** method is instantiated with in in/out stream and '<<', '>>' operators are replaced with a single '&'operator.
It is possible to determine the type of the stream at compile time for creating some platform-specific resources:
```cpp
if constexpr (std::is_base_of<aether::istream, T>::value) { ... }
```
If an object accesses another objects are being loaded then it is more convenient to do some logic in
```cpp
  virtual void OnLoaded() {}
```
method which is called after all objects in the subgraph are deserialize.

### Class pointer serialization
A pointer to another object can also be serialized/deserialized in the same way like a built-in type:
```cpp
class A : public aether::Obj {
 public:
  AETHER_OBJ(A);
  int i_;
class B : public aether::Obj {
 public:
  AETHER_OBJ(B);
  aether::Obj::ptr o_;  // A reference to the A* class but casted to aether::Obj*
  std::map<int, A::ptr> a_;
  template <typename T> void Serializator(T& s, int flags) {
    s & o_ & a_;
  }
  virtual void OnLoaded() {
    A::ptr(o_)->i_++;  // Valid
    a_[0]->i_++;  // Valid
  }
};
```
If multiple pointers are referencing a single class instance then after deserialization a single class is constructed and then referenced multiple times. Cyclic references are also supported. Each class is registed with the factory function and the unique ClassId. Each class instance = object contains unique InstanceId. Both these values are used for reconstructing the original graph on deserialization.

### Versioning
CreateObjByClassId
*AETHER_SERIALIZE(ClassName)*
App upgrade

### Hibernate / Wake-up
Serialize
Load / Unload
user-defined i/o call-backs

### Multiple references to the upper object

### Cyclic refs

### Constant objects

### Domain, example - localization
Distributed applications
        Domains: local / user / global states
        Superroot
        Different versions of events
        Simultaneous editing of a document

## Development runtime modes

### #ifdefs - initial state

### Runtime obj creation - cloning
cloning from alive obj
cloning from unloaded obj
subgraph cloning
* shallow
* deep
* full

## Event-driven
### Collapse events into state, per sub-graph {#collapse-events-into-state-per-sub-graph}

## Model-Presenter
### Cross-platform
### Mix: model-presenter-model-presenter
### Empty presenter, reproducing problems = telemetry {#empty-presenter-reproducing-problems-=-telemetry}
### Automated tests for whole app with empty presenters {#automated-tests-for-whole-app-with-empty-presenters}


## Header-only, custom streams, exception handling, RAII
## Multithreading fast serialization and double-buffering
## Developing changing resource in run-time

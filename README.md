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

## Class state serialization
operator << >> &
template<T> void Serializatior = bidirectional method
class id, object id
*AETHER_SERIALIZE(ClassName)*

### Versioning
*AETHER_SERIALIZE(ClassName)*

## Obj::ptr setialization

### Hibernate / Wake-up
Serialize
Load / Unload

### Constant objects

### Multiple references to the upper object

### Cyclic refs

### Domain, example - localization


## Development runtime modes

### #ifdefs - initial state

### Runtime obj creation - cloning
cloning from alive obj
cloning from unloaded obj
subgraph cloning
* shallow
* deep
* full


## Model-Presenter {#model-presenter}

### Cross-platform {#cross-platform}
### Mix: model-presenter-model-presenter {#mix-model-presenter-model-presenter}
### Events {#events}
### Collapse events into state, per sub-graph {#collapse-events-into-state-per-sub-graph}
### Empty presenter, reproducing problems = telemetry {#empty-presenter-reproducing-problems-=-telemetry}
### Automated tests for whole app with empty presenters {#automated-tests-for-whole-app-with-empty-presenters}
### Distributed applications: {#distributed-applications}
        Domains: local / user / global states
        Superroot
        Different versions of events
        Simultaneous editing of a document

## App specific
### onLoaded, onTimer etc.
### Same app state between runs

## Header-only, custom streams, exception handling, RAII
## Multithreading fast serialization and double-buffering
## Developing changing resource in run-time

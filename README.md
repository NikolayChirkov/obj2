# Æether Object
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
![GitHub file size in bytes](https://img.shields.io/github/size/aethernet-io/obj/blob/master/aether/obj/obj.h)

is a C++17 cross-platform architecture heder-only framework for creating lightweight, highly structured, easily supportable, reliable, distributed applications.

The core concept is well-known from other frameworks: an application is represented as a graph, and each node is a serializable class instance.


- [Hello, world!](#hello-world)
- [Key features](#key-features)
- [Example application](#example-application)
- [Detailed description](#detailed-description)
  - [Object smart pointer](#object-smart-pointer)
  - [serializable smart pointer](#serializable-smart-pointer)
  - [Development runtime modes](#development-runtime-modes)
  - [Serialization](#serialization)
  - [Serialize state events](#serialize-state-events)
  - [App specific](#app-specific)
  - [Header-only, custom streams, exception handling, RAII](#header-only-custom-streams-exception-handling-raii)
  - [Multithreading fast serialization and double-buffering](#multithreading-fast-serialization-and-double-buffering)
  - [Developing changing resource in run-time](#developing-changing-resource-in-run-time)



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



*   **Serialization** of the whole application state with small effort
*   **Loading/unloading** parts of the application’s graph with state serialization
*   **Upgrade** of the application is just the replacing a part of serialized state
*   **Versioning** of the serialized state and executable is automatic: simplifies upgrade and messaging between application’s instances
*   **Model-Presenter** for cross-platform applications with event-driven model change
*   **Debugging, automated testing**: event-driven model records all state changes and can be reproduced in any platform-specific environment
*   **Code size, performance, reliability**. The application is split into development mode, where the graph is constructed, all intermediate resources are pre-compiled and verified and is just deserialized then from binary.
*   **Distributed** applications just share the part of the graph.


## Example application

A cross-platform (Windows, macOS, iOS, Android) text editor with automatic saving text, window position and size.


## Detailed description


### Object smart pointer
Any class of an application is inherited from Obj class. *AETHER_OBJ(class name)* macro is used to declare all supporting internal functions. An Object is wrapped into the Ptr<T> template class and the pointer type is declared as *MyClass::ptr*.

#### 
    class registering, creating, class id, object id


#### 
    Inheritance, interface casting, pure_class


#### 
    Serialize template {#serialize-template}


#### 
    Linear inheritance = Versioning, program upgrade {#linear-inheritance-=-versioning-program-upgrade}


### serializable smart pointer


#### 
    Serialize obj::ptr = graph, root {#serialize-obj-ptr-=-graph-root}


#### 
    multiple refs {#multiple-refs}


#### 
    Unloading - cyclic refs {#unloading-cyclic-refs}


#### 
    Graph partial loading, deserialization of unloaded subgraph {#graph-partial-loading-deserialization-of-unloaded-subgraph}


### Development runtime modes


#### 
    #ifdefs - initial state {##ifdefs-initial-state}


#### 
    Runtime obj creation {#runtime-obj-creation}


#### 
    Cloning from alive obj {#cloning-from-alive-obj}


#### 
    Cloning from unloaded obj {#cloning-from-unloaded-obj}


#### 
    Cloning options: {#cloning-options}


            Shallow - just this object


            Deep - al objects referenced within this hierarchy (not referenced by external objects) are cloned


            Full = Deep + constant objects not referenced by external objects


### Serialization


#### 
    Serialization of everything is not good - eliminate const obj {#serialization-of-everything-is-not-good-eliminate-const-obj}


#### 
    Data, refs {#data-refs}


#### 
    Domain, example - localization {#domain-example-localization}


### Model-Presenter {#model-presenter}


#### 
    Cyclic references {#cyclic-references}


#### 
    Cross-platform {#cross-platform}


#### 
    Mix: model-presenter-model-presenter {#mix-model-presenter-model-presenter}


#### 
    Events {#events}


### Serialize state events


#### 
    Collapse events into state, per sub-graph {#collapse-events-into-state-per-sub-graph}


#### 
    Empty presenter, reproducing problems = telemetry {#empty-presenter-reproducing-problems-=-telemetry}


#### 
    Automated tests for whole app with empty presenters {#automated-tests-for-whole-app-with-empty-presenters}


#### 
    Distributed applications: {#distributed-applications}


        Domains: local / user / global states


        Superroot


        Different versions of events


        Simultaneous editing of a document


### App specific


#### 
    onLoaded, onTimer etc. {#onloaded-ontimer-etc}


#### 
    Same app state between runs {#same-app-state-between-runs}


### Header-only, custom streams, exception handling, RAII


### Multithreading fast serialization and double-buffering


### Developing changing resource in run-time

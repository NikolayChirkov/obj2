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
#include "../../../aether/obj/obj.h"
/*
#define AETHER_DEVMODE 1
#define AETHER_WIN 1
#define AETHER_MAC_CPU 1
#define AETHER_MAC_METAL 1

#if AETHER_DEVMODE
static bool devmode = true;
#endif  // AETHER_DEVMODE

class App : public aether::Obj {
public:
  AETHER_OBJECT(App);
  AETHER_SERIALIZE(App);
  AETHER_INTERFACES(App);
  
  std::string bin_path;
  std::string modified_bin_path;
  template <typename T>
  T& Serializator(T& s) {
    s & modified_bin_path & bin_path;
    return s;
  }
};

class AppWinCpu : public App {
public:
  AETHER_OBJECT(AppWinCpu);
  AETHER_SERIALIZE(AppWinCpu, App);
  AETHER_INTERFACES(App);
  
#if AETHER_DEVMODE
  AppWinCpu() {
    if (devmode) {
      bin_path = "%PROGRAM_DATA%/Aether";
      modified_bin_path = "%PROGRAM_DATA%/Aether";
    }
  }
#endif  // AETHER_DEVMODE
  
  int i1 = 1;
  template <typename T>
  T& Serializator(T& s) {
    s & i1;
    return s;
  }
};
AETHER_IMPLEMENTATION(AppWinCpu);

class AppWinDx9 : public App {
public:
  AETHER_OBJECT(AppWinDx9);
  AETHER_SERIALIZE(AppWinDx9, App);
  AETHER_INTERFACES(App);
  
  int i2 = 2;
  template <typename T>
  T& Serializator(T& s) {
    s & i2;
    return s;
  }
};
AETHER_IMPLEMENTATION(AppWinDx9);

class AppMacMetal : public App {
public:
  AETHER_OBJECT(AppMacMetal);
  AETHER_SERIALIZE(AppMacMetal, App);
  AETHER_INTERFACES(App);
  
  int i3 = 3;
  template <typename T>
  T& Serializator(T& s) {
    s & i3;
    return s;
  }
};
AETHER_IMPLEMENTATION(AppMacMetal);


class Root1 : public aether::Obj {
public:
  AETHER_OBJECT(Root1);
  AETHER_SERIALIZE(Root1);
  AETHER_INTERFACES(Root1);
  
#if AETHER_DEVMODE
  Root1() {
    if (devmode) {
      apps["win_cpu"] = new AppWinCpu();
      //      apps["win_cpu"]->save();
      apps["win_dx9"] = new AppWinDx9();
      apps["mac_metal"] = new AppMacMetal();
      init();
    }
  }
#endif  // AETHER_DEVMODE
  
  void init() {
    std::cout << "init\n";
    
  }
  
  std::map<std::string, App::ptr> apps;
  template <typename T>
  T& Serializator(T& s) {
    s & apps;
    if (std::is_base_of<aether::istream, T>::value) {
      init();
    }
    return s;
  }
};
AETHER_IMPLEMENTATION(Root1);

#include <sys/stat.h>

void DevMode() {
#if AETHER_DEVMODE
  {
    devmode = true;
    Root1::ptr root(new Root1());
    AETHER_OMSTREAM os;
    os.custom_.path_ = "Root";
    mkdir(os.custom_.path_.c_str(), 0755);
    os << root;
    std::ofstream f("Root/self.bin", std::ios::out | std::ios::binary);
    f.write((const char*)os.stream_.data(), os.stream_.size());
    std::cout << os.stream_.size() << "\n";
    devmode = false;
  }
#endif  // AETHER_DEVMODE
  {
    std::ifstream f("Root/self.bin", std::ios::in | std::ios::binary);
    f.seekg (0, f.end);
    std::streamoff length = f.tellg();
    f.seekg (0, f.beg);
    AETHER_IMSTREAM is;
    is.stream_.resize(length);
    f.read((char*)is.stream_.data(), length);
    
    Root1::ptr root;
    is >> root;
  }
}

*/

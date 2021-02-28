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
#include <string>
#include <filesystem>
#include <locale>
#include <type_traits>
#include "../../../aether/obj/obj.h"
#ifdef __APPLE__
  #include <CoreFoundation/CoreFoundation.h>
#endif

/*#define OBSERVER_DEV
#define OBSERVER_ROOT_ID 666

static std::unordered_map<aether::ObjStorage, std::string> storage_to_path_;

static auto saver = [](const std::string& path, aether::ObjStorage storage, const AETHER_OMSTREAM& os){
  std::filesystem::create_directories(std::filesystem::path{"state"} / storage_to_path_[storage]);
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::ofstream f(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  //   bool b = f.good();
  f.write((const char*)os.stream_.data(), os.stream_.size());
  std::cout << p.c_str() << " size: " << os.stream_.size() << "\n";
};

static auto loader = [](const std::string& path, aether::ObjStorage storage, AETHER_IMSTREAM& is){
  auto p = std::filesystem::path{"state"} / storage_to_path_[storage] / path;
  std::ifstream f(p.c_str(), std::ios::in | std::ios::binary);
  f.seekg (0, f.end);
  std::streamoff length = f.tellg();
  f.seekg (0, f.beg);
  is.stream_.resize(length);
  f.read((char*)is.stream_.data(), length);
};


// Only one locale is loaded at a time. All instances of the Locale use one objecti ID to allow all subsequent
// object references to a proper Locale object related to the current system.
class Locale : public aether::Obj {
public:
  AETHER_CLS(Locale);
  AETHER_SERIALIZE(Locale);
  AETHER_INTERFACES(Locale);
  Locale() = default;

  std::string app_name_;
#ifdef OBSERVER_DEV
  Locale(const std::string& lang) {
    flags_ |= aether::ObjFlags::kConst;
    if (lang == "en") app_name_ = "ApplicationName";
    else if (lang == "ru") app_name_ = "ImyaPrilozheniya";
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    if (flags & aether::Obj::Serialization::kData) s & app_name_;
  }
};
AETHER_IMPL(Locale);


// Constant object that contains all supported languages. If a language is added then:
//  1) The selector state is updated with the language
//  2) The folder with the language is added.
// This logic is moved-out from the App object to simplify updates with new supported language.
class LocaleSelector : public aether::Obj {
public:
  AETHER_CLS(LocaleSelector);
  AETHER_SERIALIZE(LocaleSelector);
  AETHER_INTERFACES(LocaleSelector);
  LocaleSelector() = default;
  
  std::vector<std::string> langs_;
#ifdef OBSERVER_DEV
  LocaleSelector(bool) : langs_{"en", "ru"} {
    flags_ |= aether::ObjFlags::kConst;
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    if (flags & aether::Obj::Serialization::kData) s & langs_;
  }
  
  std::string SelectClosestLanguage(const std::string& lang) const {
    for (auto l : langs_) {
      if (l == lang) return lang;
    }
    // The language is not supported - return english instead.
    return "en";
  }
};
AETHER_IMPL(LocaleSelector);



class App2 : public aether::Obj {
public:
  AETHER_CLS(App2);
  AETHER_SERIALIZE(App2);
  AETHER_INTERFACES(App2);
  App2() = default;
  
  std::string lang_;
  LocaleSelector::ptr locale_selector_;
  Locale::ptr locale_;
#ifdef OBSERVER_DEV
  App2(bool) {
    locale_selector_ = {new LocaleSelector(true)};
    for (auto l : locale_selector_->langs_) {
      storage_to_path_[1] = l;
      locale_ = {new Locale(l)};
      locale_.SetStorage(1);
      locale_.SetId(123);
      locale_.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    }
  }
#endif  // OBSERVER_DEV
  template <typename T> void Serializator(T& s, int flags) {
    if (flags & aether::Obj::Serialization::kRefs) s & locale_selector_;
    if (flags & aether::Obj::Serialization::kData) {
      s & lang_;
      if constexpr (std::is_base_of<aether::istream, T>::value) {
        if (lang_.empty()) {
#ifdef __APPLE__
          // CFArrayRef langs = CFLocaleCopyPreferredLanguages();
          // CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex (langs, 0);
          CFLocaleRef locale = CFLocaleCopyCurrent();
          CFStringRef lang = (CFStringRef)CFLocaleGetValue(locale, kCFLocaleLanguageCode);
          char converted[3];
          CFStringGetCString(lang, converted, sizeof(converted), kCFStringEncodingUTF8);
          // TODO: release refs
          lang_ = converted;
#else
          // TODO: check size >= 2
          lang_ = std::locale("").name().substr(0,2);
#endif
          // Select the best supported language.
          lang_ = locale_selector_->SelectClosestLanguage(lang_);
        }
        storage_to_path_[1] = lang_;
      }
    }
    if (flags & aether::Obj::Serialization::kRefs) s & locale_;
  }
};
AETHER_IMPL(App2);


void AppRun() {
  
#ifdef OBSERVER_DEV
  {
    std::filesystem::remove_all("state");

    App2::ptr app{new App2(true)};
    app.SetId(OBSERVER_ROOT_ID);
    app.Serialize(saver, aether::Obj::Serialization::kConsts | aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
    storage_to_path_.clear();
  }
#endif  // OBSERVER_DEV
  {
    App2::ptr app;
    app.SetId(OBSERVER_ROOT_ID);
    app.SetFlags(aether::ObjFlags::kLoaded);
    app.Load(loader);
    std::cout << app->locale_->app_name_ << "\n";
    app->lang_ = app->locale_selector_->langs_[1];
    app.Serialize(saver, aether::Obj::Serialization::kRefs | aether::Obj::Serialization::kData);
  }
//  while (!done) {
//    root->Execute();
//  }
}

*/



/*
class Base : public aether::Obj {
public:
  AETHER_CLS(Base);
  AETHER_SERIALIZE(Base);
  AETHER_INTERFACES(Base);
  virtual ~Base() { std::cout << "~Base\n"; }
  
  template <typename T> void Serializator(T& s) {
  }
  virtual void Load() {
  }
  virtual void Execute() {
  }
  virtual void OnLoaded() {
  }
  virtual void OnUnloading() {
  }
};
AETHER_IMPL(Base);




class Language : public Base {
public:
  AETHER_CLS(Language);
  AETHER_SERIALIZE(Language);
  AETHER_INTERFACES(Base, Language);
  virtual ~Language() { std::cout << "~Language\n"; }
  
  std::string app_name_;
  std::string tray_show_;
  std::string tray_exit_;
  template <typename T> void Serializator(T& s) {
    s & app_name_ & tray_show_ & tray_exit_;
  }
};
AETHER_IMPL(Language);




class Resources : public Base {
public:
  AETHER_CLS(Resources);
  AETHER_SERIALIZE(Resources);
  AETHER_INTERFACES(Base, Resources);
  virtual ~Resources() { std::cout << "~Resources\n"; }
  
  // This ptr is not serialized
  Language::ptr lang_;
  
  std::map<std::string, std::vector<uint32_t>> tray_icons_;
  std::string build_version_;
  std::map<std::string, Language::ptr> localization_;
  App::ptr app_;
  template <typename T> void Serializator(T& s) {
    s & app_ & tray_icons_ & build_version_ & localization_;
  }
  void OnLoaded() {
    lang_ = localization_[app_->language];
    // Don't need to call lang_->Load(); because one language is loaded.
  }
  void Execute() {
  }
  void OnUnloading() {
  }
};
AETHER_IMPL(Resources);



class TrayPresenter : public Base {
public:
  AETHER_CLS(TrayPresenter);
  AETHER_SERIALIZE(TrayPresenter);
  AETHER_INTERFACES(Base, TrayPresenter);
  virtual ~TrayPresenter() { std::cout << "~TrayPresenter\n"; }
  
  std::map<std::string, std::vector<uint32_t>> icons_;
  std::string app_name_;
  App::ptr_ app_;
  template <typename T> void Serializator(T& s) {
    s & icons_ & app_ & app_name_;
  }
  void OnLoaded() {
    // Platform specific code
    auto l = app_->resources_->lang_->app_name_;
    // SetLabel(l);
    // SetIcon(app_->resources_->tray_icons_[app_->status_]);
  }
  void Execute() {
  }
  void OnUnloading() {
  }
};
AETHER_IMPL(TrayPresenter);


class App : public Base {
public:
  AETHER_CLS(App);
  AETHER_SERIALIZE(App);
  AETHER_INTERFACES(Base, App);
  virtual ~App() { std::cout << "~App\n"; }
  
  std::string status_;
  std::string language_;
  Resources::ptr resources_;
  TrayPresenter::ptr tray_presenter_;
  template <typename T> void Serializator(T& s) {
    s & status_ & language_ & resources_ & tray_presenter_;
  }
  void OnLoaded() {
  }
  void Load() {
  }
  void Execute() {
  }
  void OnUnloading() {
  }
};
AETHER_IMPL(App);


class User : public Base {
public:
  AETHER_CLS(User);
  AETHER_SERIALIZE(User);
  AETHER_INTERFACES(Base, User);
  virtual ~User() { std::cout << "~User\n"; }

  App::ptr app_;
  template <typename T> void Serializator(T& s) {
    s & app_;
  }
  void Load() {
  }
  void Execute() {
  }
  void OnLoaded() {
  }
  void OnUnloading() {
  }
};
AETHER_IMPL(User);



class Root : public Base {
public:
  AETHER_CLS(Root);
  AETHER_SERIALIZE(Root);
  AETHER_INTERFACES(Base, Root);
  virtual ~Root() { std::cout << "~Root\n"; }
  
  User::ptr user_;
  template <typename T> void Serializator(T& s) {
    s & user_;
  }
  void Load() {
  }
  void Execute() {
  }
  void OnLoaded() {
  }
  void OnUnloading() {
  }
};
AETHER_IMPL(Root);

void AppRun() {
  AETHER_IMSTREAM is;
  Root::ptr root{new Root()};
  is >> root;
  root->OnLoaded();
  while (!done) {
    root->Execute();
  }
  root->OnUnloading();
}
*/

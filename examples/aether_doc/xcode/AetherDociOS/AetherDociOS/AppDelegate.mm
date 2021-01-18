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

#import "AppDelegate.h"
#include <filesystem>
#include "../../../model/model.h"

class MainPresenterIOS : public MainPresenter {
public:
  AETHER_OBJECT(MainPresenterIOS);
  AETHER_SERIALIZE(MainPresenterIOS, MainPresenter);
  AETHER_INTERFACES(MainPresenterIOS, MainPresenter);
  template <typename T> void Serializator(T& s, int flags) { }
  virtual bool OnEvent(const aether::Event::ptr& event) { return true; };
  virtual void OnLoaded() {};
};
AETHER_IMPLEMENTATION(MainPresenterIOS);


@interface AppDelegate ()
@end

@implementation AppDelegate {
  App::ptr app;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  auto writablePath = std::filesystem::path{[[paths objectAtIndex:0] UTF8String]} / "state";
  NSLog(@"Copy state from this path: %s", writablePath.c_str());
#ifndef AETHER_DOC_DEV
  std::filesystem::remove_all(writablePath);
  NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
  std::filesystem::copy(std::filesystem::path{[resourcePath UTF8String]} / "state", writablePath,
                        std::filesystem::copy_options::recursive | std::filesystem::copy_options::copy_symlinks);
#endif  // AETHER_DOC_DEV
  app = App::Create(writablePath.c_str());
  return YES;
}


#pragma mark - UISceneSession lifecycle


- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
  // Called when a new scene session is being created.
  // Use this method to select a configuration to create the new scene with.
  return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
  // Called when the user discards a scene session.
  // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
  // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}


@end

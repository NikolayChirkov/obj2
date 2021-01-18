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

#import "MainWindow.h"
#include "../../../model/model.h"

class MainPresenterMac : public MainPresenter {
public:
  AETHER_OBJECT(MainPresenterMac);
  AETHER_SERIALIZE(MainPresenterMac, MainPresenter);
  AETHER_INTERFACES(MainPresenterMac, MainPresenter);
  template <typename T> void Serializator(T& s, int flags) { }
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
  MainWindow* window_;
};

@implementation MainWindow {
  MainPresenterMac* presenter;
  Boolean isMoving;
}

- (void)setPresenter:(MainPresenterMac*)p {
  presenter = p;
}

- (void)windowDidLoad {
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(windowWillMove:)
                                               name:@"NSWindowWillMoveNotification"
                                             object:nil];
  // Send EventPos only when mouse is unclicked on the window's title bar.
  isMoving = NO;
  [NSEvent addLocalMonitorForEventsMatchingMask:(NSEventMaskLeftMouseUp) handler:^(NSEvent *event) {
    if (self->isMoving) {
      // Race condition?
      self->isMoving = NO;
      int x = self.frame.origin.x;
      int y = [MainWindow translatoPos:(self.frame.origin.y + self->presenter->main_->h_)];
      self->presenter->OnMove(x, y);
    }
    return event;
  }];
}

- (void)windowWillMove:(NSNotification *)notification {
  if (self == [notification object]) isMoving = YES;
}

+ (int)translatoPos:(int)y {
  int menuBarHeight = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];
  return [[NSScreen mainScreen] frame].size.height - menuBarHeight - y;
}
@end

@implementation MainWindowController
@end

@implementation MainViewController {
}

-(void)viewDidLoad {
}

@end


void MainPresenterMac::OnLoaded() {
  NSStoryboard *storyBoard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
  // Link window and presenter
  NSWindowController* windowController =
    (NSWindowController*)[storyBoard instantiateControllerWithIdentifier:@"MainWindowController"];
  window_ = (MainWindow*)[windowController window];
  [window_ setPresenter:this];
  // Link view controller and presenter
  // MainViewController* viewController = (MainViewController*)windowController.contentViewController;
  
  [window_ setFrame:CGRectMake(main_->x_, [MainWindow translatoPos:(main_->y_ + main_->h_)], main_->w_, main_->h_)
            display:YES
            animate:YES];
  [window_ windowDidLoad];
  [windowController showWindow:nil];
}

bool MainPresenterMac::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetClassId()) {
    case EventPos::class_id_: {
      EventPos::ptr e(event);
      [window_ setFrameOrigin:CGPointMake(e->x_, [MainWindow translatoPos:(e->y_ + main_->h_)])];
      return false;
    }
    default:
      return aether::Obj::OnEvent(event);
  }
}
AETHER_IMPLEMENTATION(MainPresenterMac);

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

extern void PtrTest();
extern void DomainTest();
extern void SerializeReferences();
extern void SaveToFolder();
extern void EventProcessor();
extern void DevMode();
extern void Versioning();
extern void Graph();
extern void AppRun();
extern void Loadable();

int main(int argc, const char * argv[]) {
  Versioning();
  EventProcessor();
  return 0;
}

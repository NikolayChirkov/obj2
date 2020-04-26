/* Copyright 2016 Aether authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// Example of configuration file that can be specified externally to configure
// aether::obj without changing the original source code.
// A path to the config file can be specified as command line argument as
// -DAETHER_CONFIG='../test/config.h'

#include <aether/mstream/mstream.h>
#ifndef AETHER_OMSTREAM
  #define AETHER_OMSTREAM aether::tomstream
#endif
#ifndef AETHER_IMSTREAM
  #define AETHER_IMSTREAM aether::timstream
#endif

// #include "../../third_party/cppformat/format.h"
// #define AETHER_THROW(F, ...) \
//   throw std::runtime_error(fmt::format(F, __VA_ARGS__));

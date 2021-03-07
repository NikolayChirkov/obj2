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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// Overriding exception handling
#include "../../third_party/cppformat/format.h"
#define AETHER_THROW(F, ...) \
  throw std::runtime_error(fmt::format(F, __VA_ARGS__));

#include "mstream.h"

using aether::omstream;
using aether::imstream;
using aether::tomstream;
using aether::timstream;


TEST_CASE( "type check", "tomstream" ) {
  size_t size = 0;
  tomstream os;
  os << uint8_t{123};
  REQUIRE(os.stream_.size() == (size += 1 + sizeof(uint8_t)));
  os << true;
  REQUIRE(os.stream_.size() == (size += 1 + sizeof(bool)));
  timstream is;
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  REQUIRE(!is.stream_.empty());
  REQUIRE(is.stream_.size() == size);
  uint8_t uint8_t_val;
  REQUIRE_NOTHROW(is >> uint8_t_val);
  REQUIRE(uint8_t_val == uint8_t{123});
  REQUIRE(is.stream_.size() == 1 + sizeof(bool));
}

#define CHECK_TYPE(T, V) {\
  tomstream os; \
  os << (T){V}; \
  timstream is; \
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end()); \
  T v; \
  REQUIRE_NOTHROW(is >> v); }

TEST_CASE( "check types", "tomstream" ) {
  CHECK_TYPE(bool, true);
  CHECK_TYPE(char, 12);
  CHECK_TYPE(signed char, -12);
  CHECK_TYPE(int8_t, -12);
  CHECK_TYPE(unsigned char, 13);
  CHECK_TYPE(uint8_t, 34);
  CHECK_TYPE(int16_t, 12345);
  CHECK_TYPE(short, -12345);
  CHECK_TYPE(uint16_t, 34567);
  CHECK_TYPE(unsigned short, 33333);
  CHECK_TYPE(int32_t, -4556);
  CHECK_TYPE(int, -567567);
  CHECK_TYPE(signed, -66554);
  CHECK_TYPE(uint32_t, 123);
  CHECK_TYPE(unsigned, 4545);
  CHECK_TYPE(unsigned int,77666);
  CHECK_TYPE(float, 12.23f);
  CHECK_TYPE(int64_t, -112211222334);
  CHECK_TYPE(long long, -3434);
  CHECK_TYPE(long long int, -434343);
  CHECK_TYPE(uint64_t, 998877);
  CHECK_TYPE(unsigned long long, 1233212);
  CHECK_TYPE(unsigned long long int, 101010);
  CHECK_TYPE(size_t, 993344);
  CHECK_TYPE(std::size_t, 878787);
  CHECK_TYPE(uintptr_t, 556677);
  CHECK_TYPE(long, -8877);
  CHECK_TYPE(long int, -55555);
  CHECK_TYPE(double, 1e44);
  CHECK_TYPE(long double, -1e144);
  CHECK_TYPE(std::string, "test");
}

TEST_CASE( "trivial types", "omstream" ) {
  size_t size = 0;
  omstream os;

  os << true;
  REQUIRE(os.stream_.size() == (size += sizeof(bool)));
  os << char{1};
  REQUIRE(os.stream_.size() == (size += sizeof(char)));
  os << int{2};
  REQUIRE(os.stream_.size() == (size += sizeof(int)));
  os << uint16_t{3};
  REQUIRE(os.stream_.size() == (size += sizeof(uint16_t)));
  os << size_t{4};
  REQUIRE(os.stream_.size() == (size += sizeof(size_t)));
  long long ll = 5;
  os << ll;
  REQUIRE(os.stream_.size() == (size += sizeof(long long)));
  os << uint64_t{6};
  REQUIRE(os.stream_.size() == (size += sizeof(uint64_t)));
  os << long{7};
  REQUIRE(os.stream_.size() == (size += sizeof(long)));
  os << 8.123f;
  REQUIRE(os.stream_.size() == (size += sizeof(float)));

  imstream is;
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  REQUIRE(!is.stream_.empty());
  REQUIRE(is.stream_.size() == size);
  bool bool_val;
  is >> bool_val;
  REQUIRE(bool_val == true);
  char char_val;
  is >> char_val;
  REQUIRE(char_val == 1);
  int int_val;
  is >> int_val;
  REQUIRE(int_val == 2);
  uint16_t uint16_t_val;
  is >> uint16_t_val;
  REQUIRE(uint16_t_val == 3);
  size_t size_t_val = 4;
  is >> size_t_val;
  REQUIRE(size_t_val == 4);
  long long long_long_val;
  is >> long_long_val;
  REQUIRE(long_long_val == 5);
  uint64_t uint64_t_val;
  is >> uint64_t_val;
  REQUIRE(uint64_t_val == 6);
  long long_val;
  is >> long_val;
  REQUIRE(long_val == 7);
  float float_val;
  is >> float_val;
  REQUIRE(float_val == 8.123f);
}

TEST_CASE( "const char*", "mstream" ) {
  const char* str = "std::string text";
  omstream os;
  os << str;
  REQUIRE(os.stream_.size() == sizeof(uint32_t) + std::string(str).size());
  imstream is;
  is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
  std::string res;
  is >> res;
  REQUIRE(res == str);
}

TEST_CASE( "std", "omstream" ) {
  SECTION( "std::string" ) {
    std::string str{"std::string text"};
    omstream os;
    os << str;
    REQUIRE(os.stream_.size() == sizeof(uint32_t) + str.size());
    imstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::string res;
    is >> res;
    REQUIRE(res == str);
  }
  SECTION( "std::vector<int>" ) {
    std::vector<int> v{1,2,3,4};
    omstream os;
    os << v;
    REQUIRE(os.stream_.size() == sizeof(uint32_t) + sizeof(int) * v.size());
    imstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<int> vres;
    is >> vres;
    REQUIRE(vres == v);
  }
  SECTION( "std::vector<std::vector<int>>" ) {
    std::vector<std::vector<int>> v{{1,2},{3,4}};
    omstream os;
    os << v;
    REQUIRE(os.stream_.size() == sizeof(uint32_t) + 2 * (sizeof(uint32_t) +
            sizeof(int) * 2));
    imstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<std::vector<int>> vres;
    is >> vres;
    REQUIRE(vres == v);
  }
  SECTION( "std::vector<std::string>" ) {
    std::vector<std::string> v{"1","2","3","4"};
    omstream os;
    os << v;
    imstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<std::string> vres;
    is >> vres;
    REQUIRE(vres == v);
  }
}

TEST_CASE( "std2", "tomstream" ) {
  SECTION( "std::string" ) {
    std::string str{"std::string text"};
    tomstream os;
    os << str;
    REQUIRE(os.stream_.size() == 2 + sizeof(uint32_t) + str.size());
    timstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::string res;
    REQUIRE_NOTHROW(is >> res);
    REQUIRE(res == str);
  }
  SECTION( "std::vector<int>" ) {
    std::vector<int> v{1,2,3,4};
    tomstream os;
    os << v;
    // 3 types: std::vector<Trivial>, uint32_t, int
    REQUIRE(os.stream_.size() == 3 + sizeof(uint32_t) + sizeof(int) * v.size());
    timstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<int> vres;
    REQUIRE_NOTHROW(is >> vres);
    REQUIRE(vres == v);
  }
  SECTION( "std::vector<Trivial> incorrect type" ) {
    std::vector<int> v{1,2,3,4};
    tomstream os;
    os << v;
    REQUIRE(os.stream_.size() == 3 + sizeof(uint32_t) + sizeof(int) * v.size());
    timstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<float> vres;
    REQUIRE_THROWS_WITH(is >> vres, "timstream: error reading type 9 as 16");
  }
  SECTION( "std::vector Trivial vs. NonTrivial" ) {
    std::vector<int> v{1,2,3,4};
    tomstream os;
    os << v;
    REQUIRE(os.stream_.size() == 3 + sizeof(uint32_t) + sizeof(int) * v.size());
    timstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<std::string> vres;
    REQUIRE_THROWS_WITH(is >> vres, "timstream: error reading type 254 as 255");
  }
  SECTION( "std::vector<NonTrivial> incorrect type" ) {
    std::vector<std::vector<int>> v{{1,2},{3,4}};
    tomstream os;
    os << v;
    timstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<std::string> vres;
    REQUIRE_THROWS_WITH(is >> vres, "timstream: error reading type 254 as 19");
  }
  SECTION( "std::vector<std::string>" ) {
    std::vector<std::string> v{"1","2","3","4"};
    tomstream os;
    os << v;
    timstream is;
    is.stream_.insert(is.stream_.begin(), os.stream_.begin(), os.stream_.end());
    std::vector<std::string> vres;
    REQUIRE_NOTHROW(is >> vres);
    REQUIRE(vres == v);
  }
}

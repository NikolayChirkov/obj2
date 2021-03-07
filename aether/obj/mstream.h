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

#ifndef AETHER_MSTREAM_H_
#define AETHER_MSTREAM_H_

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include <map>

// If the platform doesn't support uint8_t let's define something close.
#ifndef UINT8_MAX
typedef unsigned char uint8_t;
#endif

// Overriding text for error messages. Obfuscating?
#ifndef AETHER_TEXT
#define AETHER_TEXT(X) (X)
#endif

// Overriding exception throwing. If an exception is not thrown segmentation
// violation should appear.
#ifndef AETHER_THROW
#include <stdexcept>
#define AETHER_THROW(F, ...) throw std::runtime_error(F);
#endif

#ifndef AETHER_OSTREAM_CONTAINER
#define AETHER_OSTREAM_CONTAINER std::vector<uint8_t>
#define AETHER_CONTAINER_WRITE(stream, data, size) \
  stream.insert(stream.end(), reinterpret_cast<const uint8_t*>(data), \
  reinterpret_cast<const uint8_t*>(data) + size);
#endif

#ifndef AETHER_ISTREAM_CONTAINER
#define AETHER_ISTREAM_CONTAINER std::vector<uint8_t>
#define AETHER_CONTAINER_READ(stream, data, size) \
  std::memcpy(data, stream.data(), size); \
  stream.erase(stream.begin(), stream_.begin() + size);
#endif

namespace aether {

// https://en.cppreference.com/w/cpp/language/types
// Undefined types are mapped to 0.
template <typename T> struct TypeToIndexImpl{ enum { value = 0 }; };

template<> struct TypeToIndexImpl<bool> { enum { value = 1 }; };
template<> struct TypeToIndexImpl<char> { enum { value = 2 }; };

template<> struct TypeToIndexImpl<signed char> { enum { value = 3 }; };
template<> struct TypeToIndexImpl<unsigned char> { enum { value = 4 }; };

// 16 bits.
template<> struct TypeToIndexImpl<short int> { enum { value = 5 }; };
template<> struct TypeToIndexImpl<unsigned short int> { enum { value = 6 }; };
template<> struct TypeToIndexImpl<char16_t> { enum { value = 7 }; };
template<> struct TypeToIndexImpl<wchar_t> { enum { value = 8 }; };

// 32 bits, at least 16.
template<> struct TypeToIndexImpl<int> { enum { value = 9 }; };
template<> struct TypeToIndexImpl<unsigned int> { enum { value = 10 }; };

// 32 bits.
template<> struct TypeToIndexImpl<long int> { enum { value = 11 }; };
template<> struct TypeToIndexImpl<unsigned long int> { enum { value = 12 }; };
template<> struct TypeToIndexImpl<char32_t> { enum { value = 13 }; };

// 64 bits.
template<> struct TypeToIndexImpl<long long int> { enum { value = 14 }; };
template<> struct TypeToIndexImpl<unsigned long long int> {
  enum { value = 15 };
};

template<> struct TypeToIndexImpl<float> { enum { value = 16 }; };
template<> struct TypeToIndexImpl<double> { enum { value = 17 }; };
template<> struct TypeToIndexImpl<long double> { enum { value = 18 }; };

// Non-trivial types.
template<> struct TypeToIndexImpl<std::string> { enum { value = 19 }; };
constexpr uint8_t kNonTrivialVectorTypeIndex = 255;
constexpr uint8_t kTrivialVectorTypeIndex = 254;
constexpr uint8_t kMapTypeIndex = 253;

template <typename T>
constexpr uint8_t TypeToIndex() { return TypeToIndexImpl<T>::value; }

// Base classes to simplify std::conditional checks in serialization functions.
class ostream {};
class istream {};

template <bool Typed, typename Custom>
class ostream_impl : public ostream {
 public:
  Custom custom_;

  void write(const void* data, size_t size) {
    AETHER_CONTAINER_WRITE(stream_, data, size);
  }

  struct VoidWriter {
    static void write(ostream_impl<Typed, Custom>*, uint8_t) {}
  };
  struct Writer {
    static void write(ostream_impl<Typed, Custom>* stream, uint8_t type_index) {
      stream->write(&type_index, sizeof(type_index));
    }
  };
  void write_type(uint8_t type_index) {
    std::conditional<
      Typed, Writer, VoidWriter>::type::write(this, type_index);
  }

  AETHER_OSTREAM_CONTAINER stream_;
};
template<typename Custom>
using omstream = ostream_impl<false, Custom>;
template<typename Custom>
using tomstream = ostream_impl<true, Custom>;

template <bool Typed, typename Custom>
class istream_impl : public istream {
 public:
  Custom custom_;

  void read(void* data, size_t size) {
    if (size > stream_.size()) {
      AETHER_THROW(AETHER_TEXT(
        "imstream: reading {0} bytes from stream containing {1} bytes"), size,
        stream_.size());
    }
    AETHER_CONTAINER_READ(stream_, data, size);
  }

  struct VoidReader {
    static void read_type_and_check(istream_impl<Typed, Custom>*, uint8_t) { }
  };
  struct Reader {
    static void read_type_and_check(istream_impl<Typed, Custom>* s, uint8_t type_index) {
      uint8_t index;
      s->read(&index, sizeof(index));
      if (index != type_index) {
        AETHER_THROW(AETHER_TEXT("timstream: error reading type {0} as {1}"),
                     int{index}, int{type_index});
      }
    }
  };
  void readTypeAndCheck(uint8_t type_index) {
    std::conditional<
      Typed, Reader, VoidReader>::type::read_type_and_check(this, type_index);
  }

  AETHER_ISTREAM_CONTAINER stream_;
};
template<typename Custom>
using imstream = istream_impl<false, Custom>;
template<typename Custom>
using timstream = istream_impl<true, Custom>;

// All other operators uses this two operators as end-points of read/write
// operations
template <typename T, typename S>
S& WriteTrivial(S& s, const T& t) {
  static_assert(std::is_trivially_copyable<T>::value,
                "Error: omstream can't automatically serialize non trivial \
                 type. Provide serialization operator.");
  static_assert(!std::is_pointer<T>::value,
                "Error: omstream can't automatically serialize a pointer. \
                 Provide serialization operator.");
  s.write_type(TypeToIndex<T>());
  s.write(&t, sizeof(T));
  return s;
}
template <bool Typed, typename T, typename Custom, typename = typename std::enable_if<
            std::is_trivially_copyable<T>::value && !std::is_pointer<T>::value >::type>
ostream_impl<Typed, Custom>& operator<<(ostream_impl<Typed, Custom>& s, const T& t) {
  return WriteTrivial(s, t);
}


template <typename T, typename S>
S& ReadTrivial(S& s, T& t) {
  static_assert(std::is_trivially_copyable<T>::value,
                "Error: imstream can't automatically deserialize non trivial \
                 type. Provide deserialization operator.");
  static_assert(!std::is_pointer<T>::value,
                "Error: imstream can't automatically deserialize pointer. \
                 Provide deserialization operator.");
  s.readTypeAndCheck(TypeToIndex<T>());
  s.read(&t, sizeof(T));
  return s;
}
template <bool Typed, typename T, typename Custom, typename = typename std::enable_if<
          std::is_trivially_copyable<T>::value && !std::is_pointer<T>::value >::type>
istream_impl<Typed, Custom>& operator>>(istream_impl<Typed, Custom>& s, T& t) {
  return ReadTrivial(s, t);
}

template <typename S>
S& WriteConstChar(S& s, const char* t) {
  s.write_type(TypeToIndex<std::string>());
  uint32_t length = static_cast<uint32_t>(std::strlen(t));
  s << static_cast<uint32_t>(length);
  s.write(t, length);
  return s;
}
template <bool Typed, typename Custom>
ostream_impl<Typed, Custom>& operator<<(ostream_impl<Typed, Custom>& s, const char* t) {
  return WriteConstChar(s, t);
}

// supporting standard data types
template <typename S>
S& WriteString(S& s, const std::string& t) {
  s.write_type(TypeToIndex<std::string>());
  s << static_cast<uint32_t>(t.size());
  s.write(t.data(), t.size());
  return s;
}
template <bool Typed, typename Custom>
ostream_impl<Typed, Custom>& operator<<(ostream_impl<Typed, Custom>& s, const std::string& t) {
  return WriteString(s, t);
}

template <typename S>
S& ReadString(S& s, std::string& t) {
  s.readTypeAndCheck(TypeToIndex<std::string>());
  uint32_t size;
  s >> size;
  if (size > 0) {
    t.resize(size);
    s.read(const_cast<char*>(t.data()), size);
  }
  return s;
}
template <bool Typed, typename Custom>
istream_impl<Typed, Custom>& operator>>(istream_impl<Typed, Custom>& s, std::string& t) {
  return ReadString(s, t);
}

template <typename T, typename S>
struct VectorWriterContinuous {
  static void save(S& s, const std::vector<T>& t) {
    s.write_type(kTrivialVectorTypeIndex);
    s.write_type(TypeToIndex<T>());
    s << static_cast<uint32_t>(t.size());
    if (!t.empty())
      s.write(t.data(), t.size() * sizeof(T));
  }
};
template <typename T, typename S>
struct VectorWriterPerElement {
  static void save(S& s, const std::vector<T>& t) {
    s.write_type(kNonTrivialVectorTypeIndex);
    s << static_cast<uint32_t>(t.size());
    if (!t.empty()) {
      for (const T& v : t) {
        s << v;
      }
    }
  }
};
template <typename T, typename S>
S& WriteVector(S& s, const std::vector<T>& t) {
  std::conditional<std::is_trivially_copyable<T>::value, VectorWriterContinuous<T, S>,
    VectorWriterPerElement<T, S>>::type::save(s, t);
  return s;
}
template <bool Typed, typename T, typename Custom>
ostream_impl<Typed, Custom>& operator <<(ostream_impl<Typed, Custom>& s, const std::vector<T>& t) {
  return WriteVector(s, t);
}

template <typename T, typename S>
struct VectorReaderContinuous {
  static void load(S& s, std::vector<T>& t) {
    s.readTypeAndCheck(kTrivialVectorTypeIndex);
    s.readTypeAndCheck(TypeToIndex<T>());
    uint32_t size;
    s >> size;
    if (size > 0) {
      t.resize(size);
      s.read(t.data(), size * sizeof(T));
    }
  }
};
template <typename T, typename S>
struct VectorReaderPerElement {
  static void load(S& s, std::vector<T>& t) {
    s.readTypeAndCheck(kNonTrivialVectorTypeIndex);
    uint32_t size;
    s >> size;
    if (size > 0) {
      t.resize(size);

      for (T& v : t)
        s >> v;
    }
  }
};
template <typename T, typename S>
S& ReadVector(S& s, std::vector<T>& t) {
  std::conditional<std::is_trivially_copyable<T>::value,
    VectorReaderContinuous<T, S>,
    VectorReaderPerElement<T, S>>::type::load(s, t);
  return s;
}
template <bool Typed, typename T, typename Custom>
istream_impl<Typed, Custom>& operator >>(istream_impl<Typed, Custom>& s, std::vector<T>& t) {
  return ReadVector(s, t);
}

template <bool Typed, typename T1, typename T2, typename Custom>
ostream_impl<Typed, Custom>& operator <<(ostream_impl<Typed, Custom>& s, const std::map<T1, T2>& t) {
  s.write_type(kMapTypeIndex);
  s.write_type(TypeToIndex<T1>());
  s.write_type(TypeToIndex<T2>());
  s << uint32_t(t.size());
  for (const auto& i : t) {
    s << i.first << i.second;
  }
  return s;
}
template <bool Typed, typename T1, typename T2, typename Custom>
istream_impl<Typed, Custom>& operator >>(istream_impl<Typed, Custom>& s, std::map<T1, T2>& t) {
  s.readTypeAndCheck(kMapTypeIndex);
  s.readTypeAndCheck(TypeToIndex<T1>());
  s.readTypeAndCheck(TypeToIndex<T2>());
  uint32_t size;
  s >> size;
  for (int i = 0; i < size; i++) {
    T1 k;
    s >> k;
    T2 v;
    s >> v;
    t[k] = std::move(v);
  }
  return s;
}

// & bi-directional operators
template <bool Typed, typename T, typename Custom>
ostream_impl<Typed, Custom>& operator &(ostream_impl<Typed, Custom>& s, const T& t) {
  return s << t;
}
template <bool Typed, typename T, typename Custom>
istream_impl<Typed, Custom>& operator &(istream_impl<Typed, Custom>& s, T& t) {
  return s >> t;
}

template <bool Typed, typename T, typename Custom>
ostream_impl<Typed, Custom>& operator &(ostream_impl<Typed, Custom>& s, const std::vector<T>& t) {
  return s << t;
}
template <bool Typed, typename T, typename Custom>
istream_impl<Typed, Custom>& operator &(istream_impl<Typed, Custom>& s, std::vector<T>& t) {
  return s >> t;
}

}  // namespace aether

#endif  // AETHER_MSTREAM_H_

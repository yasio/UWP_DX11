//////////////////////////////////////////////////////////////////////////////////////////
// A cross platform socket APIs, support ios & android & wp8 & window store
// universal app
//////////////////////////////////////////////////////////////////////////////////////////
/*
The MIT License (MIT)

Copyright (c) 2012-2020 HALX99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef YASIO__OBSTREAM_HPP
#define YASIO__OBSTREAM_HPP
#include <stddef.h>
#include <string>
#include "yasio/cxx17/string_view.hpp"
#include <sstream>
#include <vector>
#include <stack>
#include "yasio/detail/endian_portable.hpp"
#include "yasio/detail/config.hpp"
namespace yasio
{
class obstream
{
public:
  YASIO__DECL obstream(size_t capacity = 128);
  YASIO__DECL obstream(const obstream& rhs);
  YASIO__DECL obstream(obstream&& rhs);
  YASIO__DECL ~obstream();

  YASIO__DECL void push8();
  YASIO__DECL void pop8();
  YASIO__DECL void pop8(uint8_t);

  YASIO__DECL void push16();
  YASIO__DECL void pop16();
  YASIO__DECL void pop16(uint16_t);

  YASIO__DECL void push24();
  YASIO__DECL void pop24();
  YASIO__DECL void pop24(uint32_t);

  YASIO__DECL void push32();
  YASIO__DECL void pop32();
  YASIO__DECL void pop32(uint32_t);

  YASIO__DECL obstream& operator=(const obstream& rhs);
  YASIO__DECL obstream& operator=(obstream&& rhs);

  /* write 7bit encoded variant integer value
  ** @.net BinaryWriter.Write7BitEncodedInt(Int32)
  */
  YASIO__DECL void write_i(int value);

  YASIO__DECL void write_i24(int32_t value);  // highest bit as sign
  YASIO__DECL void write_u24(uint32_t value); // highest byte ignored

  /* write blob data with '7bit encoded int' length field */
  YASIO__DECL void write_v(cxx17::string_view sv);

  /* 32 bits length field */
  YASIO__DECL void write_v32(cxx17::string_view);
  /* 16 bits length field */
  YASIO__DECL void write_v16(cxx17::string_view);
  /* 8 bits length field */
  YASIO__DECL void write_v8(cxx17::string_view);

  YASIO__DECL void write_v32(const void* v, int size);
  YASIO__DECL void write_v16(const void* v, int size);
  YASIO__DECL void write_v8(const void* v, int size);

  YASIO__DECL void write_byte(uint8_t v);

  YASIO__DECL void write_bytes(cxx17::string_view);
  YASIO__DECL void write_bytes(const void* v, int vl);
  YASIO__DECL void write_bytes(std::streamoff offset, const void* v, int vl);

  bool empty() const { return buffer_.empty(); }
  size_t length() const { return buffer_.size(); }
  const char* data() const { return buffer_.data(); }

  const std::vector<char>& buffer() const { return buffer_; }
  std::vector<char>& buffer() { return buffer_; }

  char* wptr(ptrdiff_t offset = 0) { return &buffer_.front() + offset; }

  template <typename _Nty> inline void write_ix(_Nty value)
  {
    auto nv = yasio::endian::htonv(value);
    write_bytes(&nv, sizeof(nv));
  }

  template <typename _Nty> inline void pwrite_ix(ptrdiff_t offset, const _Nty value)
  {
    swrite_ix(wptr(offset), value);
  }
  template <typename _Nty> static void swrite_ix(void* dst, const _Nty value)
  {
    auto nv = yasio::endian::htonv(value);
    ::memcpy(dst, &nv, sizeof(nv));
  }

  template <typename _LenT> inline void write_vx(const void* v, int size)
  {
    write_ix<_LenT>(static_cast<_LenT>(size));
    if (size > 0)
      write_bytes(v, size);
  }
  YASIO__DECL obstream sub(size_t offset, size_t count = -1);

public:
  YASIO__DECL void save(const char* filename);

protected:
  std::vector<char> buffer_;
  std::stack<size_t> offset_stack_;
}; // CLASS obstream

template <> inline void obstream::write_ix<float>(float value)
{
  auto nv = htonf(value);
  write_bytes(&nv, sizeof(nv));
}

template <> inline void obstream::write_ix<double>(double value)
{
  auto nv = htond(value);
  write_bytes(&nv, sizeof(nv));
}
} // namespace yasio

#if defined(YASIO_HEADER_ONLY)
#  include "yasio/obstream.cpp" // lgtm [cpp/include-non-header]
#endif

#endif

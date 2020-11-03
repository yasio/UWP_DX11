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
#ifndef YASIO__IBSTREAM_HPP
#define YASIO__IBSTREAM_HPP
#include <stddef.h>
#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include "yasio/cxx17/string_view.hpp"
#include "yasio/detail/endian_portable.hpp"
#include "yasio/detail/config.hpp"
namespace yasio
{
class obstream;
class ibstream_view
{
public:
  YASIO__DECL ibstream_view();
  YASIO__DECL ibstream_view(const void* data, size_t size);
  YASIO__DECL ibstream_view(const obstream*);
  YASIO__DECL ibstream_view(const ibstream_view&) = delete;
  YASIO__DECL ibstream_view(ibstream_view&&)      = delete;

  YASIO__DECL ~ibstream_view();

  YASIO__DECL void reset(const void* data, size_t size);

  YASIO__DECL ibstream_view& operator=(const ibstream_view&) = delete;
  YASIO__DECL ibstream_view& operator=(ibstream_view&&) = delete;

  /* write 7bit encoded variant integer value
  ** @.net BinaryReader.Read7BitEncodedInt
  */
  YASIO__DECL int read_i();

  YASIO__DECL int32_t read_i24();
  YASIO__DECL uint32_t read_u24();

  /* read blob data with '7bit encoded int' length field */
  YASIO__DECL cxx17::string_view read_v();

  YASIO__DECL void read_v32(std::string&); // 32 bits length field
  YASIO__DECL void read_v16(std::string&); // 16 bits length field
  YASIO__DECL void read_v8(std::string&);  // 8 bits length field

  YASIO__DECL void read_v32(void*, int);
  YASIO__DECL void read_v16(void*, int);
  YASIO__DECL void read_v8(void*, int);

  YASIO__DECL uint8_t read_byte();

  YASIO__DECL void read_bytes(std::string& oav, int len);
  YASIO__DECL void read_bytes(void* oav, int len);

  YASIO__DECL cxx17::string_view read_v32();
  YASIO__DECL cxx17::string_view read_v16();
  YASIO__DECL cxx17::string_view read_v8();

  YASIO__DECL cxx17::string_view read_bytes(int len);

  const char* data() { return first_; }
  size_t length(void) { return last_ - first_; }

  YASIO__DECL ptrdiff_t seek(ptrdiff_t offset, int whence);

  template <typename _Nty> inline _Nty read_ix() { return sread_ix<_Nty>(consume(sizeof(_Nty))); }

  template <typename _Nty> static _Nty sread_ix(const void* src)
  {
    _Nty value;
    ::memcpy(&value, src, sizeof(value));
    return yasio::endian::ntohv(value);
  }

  template <typename _LenT> inline cxx17::string_view read_vx()
  {
    _LenT n = read_ix<_LenT>();

    if (n > 0)
      return read_bytes(n);

    return {};
  }

protected:
  // will throw std::out_of_range
  YASIO__DECL const char* consume(size_t size);

protected:
  const char* first_;
  const char* last_;
  const char* ptr_;
};

template <> inline float ibstream_view::sread_ix<float>(const void* src)
{
  uint32_t nv;
  ::memcpy(&nv, src, sizeof(nv));
  return ntohf(nv);
}

template <> inline double ibstream_view::sread_ix<double>(const void* src)
{
  uint64_t nv;
  ::memcpy(&nv, src, sizeof(nv));
  return ntohd(nv);
}

/// --------------------- CLASS ibstream ---------------------
class ibstream : public ibstream_view
{
public:
  YASIO__DECL ibstream(std::vector<char> blob);
  YASIO__DECL ibstream(const obstream* obs);

private:
  std::vector<char> blob_;
};

} // namespace yasio

#if defined(YASIO_HEADER_ONLY)
#  include "yasio/ibstream.cpp" // lgtm [cpp/include-non-header]
#endif

#endif

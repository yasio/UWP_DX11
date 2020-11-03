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
#ifndef YASIO__CORE_HPP
#define YASIO__CORE_HPP

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <functional>
#if defined(_WIN32)
#  include <map>
#endif
#include "yasio/detail/sz.hpp"
#include "yasio/detail/config.hpp"
#include "yasio/detail/endian_portable.hpp"
#include "yasio/detail/object_pool.hpp"
#include "yasio/detail/singleton.hpp"
#include "yasio/detail/select_interrupter.hpp"
#include "yasio/detail/concurrent_queue.hpp"
#include "yasio/detail/utils.hpp"
#include "yasio/cxx17/memory.hpp"
#include "yasio/cxx17/string_view.hpp"
#include "yasio/xxsocket.hpp"

#if !defined(YASIO_HAVE_CARES)
#  include "yasio/cxx17/shared_mutex.hpp"
#endif

#if defined(YASIO_HAVE_KCP)
typedef struct IKCPCB ikcpcb;
#endif

#if defined(YASIO_HAVE_SSL)
typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
#endif

#if defined(YASIO_HAVE_CARES)
typedef struct ares_channeldata* ares_channel;
typedef struct ares_addrinfo ares_addrinfo;
#endif

#define yasio__find(cont, v) ::std::find(cont.begin(), cont.end(), v)
#define yasio__find_if(cont, pred) ::std::find_if(cont.begin(), cont.end(), pred)

namespace yasio
{
namespace inet
{
// options
enum
{ // lgtm [cpp/irregular-enum-init]

  // Set with deferred dispatch event, default is: 1
  // params: deferred_event:int(1)
  YOPT_S_DEFERRED_EVENT = 1,

  // Set custom resolve function, native C++ ONLY
  // params: func:resolv_fn_t*
  YOPT_S_RESOLV_FN,

  // Set custom print function, native C++ ONLY, you must ensure thread safe of it.
  // remark:
  // parmas: func:print_fn_t
  YOPT_S_PRINT_FN,

  // Set custom print function, native C++ ONLY, you must ensure thread safe of it.
  // remark:
  // parmas: func:print_fn2_t
  YOPT_S_PRINT_FN2,

  // Set custom print function
  // params: func:event_cb_t*
  YOPT_S_EVENT_CB,

  // Set tcp keepalive in seconds, probes is tries.
  // params: idle:int(7200), interal:int(75), probes:int(10)
  YOPT_S_TCP_KEEPALIVE,

  // Don't start a new thread to run event loop
  // value:int(0)
  YOPT_S_NO_NEW_THREAD,

  // Sets ssl verification cert, if empty, don't verify
  // value:const char*
  YOPT_S_SSL_CACERT,

  // Set connect timeout in seconds
  // params: connect_timeout:int(10)
  YOPT_S_CONNECT_TIMEOUT,

  // Set dns cache timeout in seconds
  // params: dns_cache_timeout : int(600),
  YOPT_S_DNS_CACHE_TIMEOUT,

  // Set dns queries timeout in seconds, default is: 5
  // params: dns_queries_timeout : int(5)
  // remark:
  //         a. this option must be set before 'io_service::start'
  //         b. only works when have c-ares
  //         c. since v3.33.0 it's milliseconds, previous is seconds.
  //         d. the timeout algorithm of c-ares is complicated, usually, by default, dns queries
  //         will failed with timeout after more than 75 seconds.
  //         e. for more detail, please see:
  //         https://c-ares.haxx.se/ares_init_options.html
  YOPT_S_DNS_QUERIES_TIMEOUT,

  // Set dns queries timeout in milliseconds, default is: 5000
  // remark: same with YOPT_S_DNS_QUERIES_TIMEOUT, but in mmilliseconds
  YOPT_S_DNS_QUERIES_TIMEOUTMS,

  // Set dns queries tries when timeout reached, default is: 5
  // params: dns_queries_tries : int(5)
  // remark:
  //        a. this option must be set before 'io_service::start'
  //        b. relative option: YOPT_S_DNS_QUERIES_TIMEOUT
  YOPT_S_DNS_QUERIES_TRIES,

  // Set dns server dirty
  // params: reserved : int(1)
  // remark:
  //        a. this option only works with c-ares enabled
  //        b. you should set this option after your mobile network changed
  YOPT_S_DNS_DIRTY,

  // Set whether ignore udp error, by default is 1: don't trigger handle_close,
  // params: ignored : int(1)
  YOPT_S_IGNORE_UDP_ERROR,

  // Sets channel length field based frame decode function, native C++ ONLY
  // params: index:int, func:decode_len_fn_t*
  YOPT_C_LFBFD_FN = 101,

  // Sets channel length field based frame decode params
  // params:
  //     index:int,
  //     max_frame_length:int(10MBytes),
  //     length_field_offset:int(-1),
  //     length_field_length:int(4),
  //     length_adjustment:int(0),
  YOPT_C_LFBFD_PARAMS,

  // Sets channel length field based frame decode initial bytes to strip, default is 0
  // params:
  //     index:int,
  //     initial_bytes_to_strip:int(0)
  YOPT_C_LFBFD_IBTS,

  // Sets channel remote host
  // params: index:int, ip:const char*
  YOPT_C_REMOTE_HOST,

  // Sets channel remote port
  // params: index:int, port:int
  YOPT_C_REMOTE_PORT,

  // Sets channel remote endpoint
  // params: index:int, ip:const char*, port:int
  YOPT_C_REMOTE_ENDPOINT,

  // Sets local host for client channel only
  // params: index:int, ip:const char*
  YOPT_C_LOCAL_HOST,

  // Sets local port for client channel only
  // params: index:int, port:int
  YOPT_C_LOCAL_PORT,

  // Sets local endpoint for client channel only
  // params: index:int, ip:const char*, port:int
  YOPT_C_LOCAL_ENDPOINT,

  // Sets channl flags
  // params: index:int, flagsToAdd:int, flagsToRemove:int
  YOPT_C_MOD_FLAGS,

  // Enable channel multicast mode
  // params: index:int, multi_addr:const char*, loopback:int
  YOPT_C_ENABLE_MCAST,

  // Disable channel multicast mode
  // params: index:int
  YOPT_C_DISABLE_MCAST,

  // The kcp conv id, must equal in two endpoint from the same connection
  // params: index:int, conv:int
  YOPT_C_KCP_CONV,

  // Change 4-tuple association for io_transport_udp
  // params: transport:transport_handle_t
  // remark: only works for udp client transport
  YOPT_T_CONNECT,

  // Dissolve 4-tuple association for io_transport_udp
  // params: transport:transport_handle_t
  // remark: only works for udp client transport
  YOPT_T_DISCONNECT,

  // Sets io_base sockopt
  // params: io_base*,level:int,optname:int,optval:int,optlen:int
  YOPT_B_SOCKOPT = 201,
};

// channel masks: only for internal use, not for user
enum
{
  YCM_CLIENT = 1,
  YCM_SERVER = 1 << 1,
  YCM_TCP    = 1 << 2,
  YCM_UDP    = 1 << 3,
  YCM_KCP    = 1 << 4,
  YCM_SSL    = 1 << 5,
  YCM_UDS    = 1 << 6, // IPC: posix domain socket
};

// channel kinds: for user to call io_service::open
enum
{
  YCK_TCP_CLIENT = YCM_TCP | YCM_CLIENT,
  YCK_TCP_SERVER = YCM_TCP | YCM_SERVER,
  YCK_UDP_CLIENT = YCM_UDP | YCM_CLIENT,
  YCK_UDP_SERVER = YCM_UDP | YCM_SERVER,
  YCK_KCP_CLIENT = YCM_KCP | YCM_CLIENT | YCM_UDP,
  YCK_KCP_SERVER = YCM_KCP | YCM_SERVER | YCM_UDP,
  YCK_SSL_CLIENT = YCM_SSL | YCM_CLIENT | YCM_TCP,
};

// channel flags
enum
{
  /* Whether setsockopt SO_REUSEADDR and SO_REUSEPORT */
  YCF_REUSEADDR = 1 << 9,

  /* For winsock security issue, see:
     https://docs.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse
  */
  YCF_EXCLUSIVEADDRUSE = 1 << 10,
};

// event kinds
enum
{
  YEK_CONNECT_RESPONSE = 1,
  YEK_CONNECTION_LOST,
  YEK_PACKET,
};

// the network core service log level
enum
{
  YLOG_V,
  YLOG_D,
  YLOG_I,
  YLOG_E,
};

// class fwds
class highp_timer;
class io_send_op;
class io_sendto_op;
class io_event;
class io_channel;
class io_transport;
class io_transport_tcp; // tcp client/server
class io_transport_ssl; // ssl client
class io_transport_udp; // udp client/server
class io_transport_kcp; // kcp client/server
class io_service;

// recommand user always use transport_handle_t, in the future, it's maybe void* or intptr_t
typedef io_transport* transport_handle_t;

// typedefs
typedef std::unique_ptr<io_send_op> send_op_ptr;
typedef std::unique_ptr<io_event> event_ptr;
typedef std::shared_ptr<highp_timer> highp_timer_ptr;

typedef std::function<bool()> timer_cb_t;
typedef std::function<void()> light_timer_cb_t;
typedef std::function<void(event_ptr&&)> event_cb_t;
typedef std::function<void(int, size_t)> completion_cb_t;
typedef std::function<int(void* ptr, int len)> decode_len_fn_t;
typedef std::function<int(std::vector<ip::endpoint>&, const char*, unsigned short)> resolv_fn_t;
typedef std::function<void(const char*)> print_fn_t;
typedef std::function<void(int level, const char*)> print_fn2_t;

typedef std::pair<highp_timer*, timer_cb_t> timer_impl_t;

// alias, for compatible purpose only
typedef highp_timer deadline_timer;
typedef highp_timer_ptr deadline_timer_ptr;
typedef event_cb_t io_event_cb_t;
typedef completion_cb_t io_completion_cb_t;

struct io_hostent {
  io_hostent() = default;
  io_hostent(std::string ip, u_short port) : host_(std::move(ip)), port_(port) {}
  io_hostent(io_hostent&& rhs) : host_(std::move(rhs.host_)), port_(rhs.port_) {}
  io_hostent(const io_hostent& rhs) : host_(rhs.host_), port_(rhs.port_) {}
  void set_ip(const std::string& value) { host_ = value; }
  const std::string& get_ip() const { return host_; }
  void set_port(u_short port) { port_ = port; }
  u_short get_port() const { return port_; }
  std::string host_;
  u_short port_ = 0;
};

class highp_timer {
public:
  ~highp_timer() {}
  highp_timer(io_service& service) : service_(service) {}

  void expires_from_now(const std::chrono::microseconds& duration)
  {
    this->duration_    = duration;
    this->expire_time_ = steady_clock_t::now() + this->duration_;
  }

  void expires_from_now() { this->expire_time_ = steady_clock_t::now() + this->duration_; }

  // Wait timer timeout once.
  void async_wait_once(light_timer_cb_t cb)
  {
#if YASIO__HAS_CXX14
    this->async_wait([cb = std::move(cb)]() {
#else
    this->async_wait([cb]() {
#endif
      cb();
      return true;
    });
  }

  // Wait timer timeout
  // @retval of timer_cb_t:
  //        true: wait once
  //        false: wait again after expired
  YASIO__DECL void async_wait(timer_cb_t);

  // Cancel the timer
  YASIO__DECL void cancel();

  // Check if timer is expired?
  bool expired() const { return wait_duration().count() <= 0; }

  // Gets wait duration of timer.
  std::chrono::microseconds wait_duration() const { return std::chrono::duration_cast<std::chrono::microseconds>(this->expire_time_ - steady_clock_t::now()); }

  io_service& service_;
  std::chrono::microseconds duration_                  = {};
  std::chrono::time_point<steady_clock_t> expire_time_ = {};
};

struct io_base {
  enum class state : u_short
  {
    CLOSED,
    OPENING,
    OPEN,
  };
  io_base() : error_(0), state_(state::CLOSED), opmask_(0) {}
  virtual ~io_base() {}
  void set_last_errno(int error) { error_ = error; }

  std::shared_ptr<xxsocket> socket_;
  int error_; // socket error(>= -1), application error(< -1)

  std::atomic<state> state_;
  u_short opmask_;
};

#if defined(YASIO_HAVE_SSL)
class ssl_auto_handle {
public:
  ssl_auto_handle() : ssl_(nullptr) {}
  ~ssl_auto_handle() { destroy(); }
  ssl_auto_handle(ssl_auto_handle&& rhs) : ssl_(rhs.release()) {}
  ssl_auto_handle& operator=(ssl_auto_handle&& rhs)
  {
    this->reset(rhs.release());
    return *this;
  }
  SSL* release()
  {
    auto tmp = ssl_;
    ssl_     = nullptr;
    return tmp;
  }
  void reset(SSL* ssl)
  {
    if (ssl_)
      destroy();
    ssl_ = ssl;
  }
  operator SSL*() { return ssl_; }
  YASIO__DECL void destroy();

protected:
  SSL* ssl_ = nullptr;
};
#endif

class io_channel : public io_base {
  friend class io_service;
  friend class io_transport;
  friend class io_transport_tcp;
  friend class io_transport_ssl;
  friend class io_transport_udp;
  friend class io_transport_kcp;

public:
  io_service& get_service() { return timer_.service_; }
  int index() const { return index_; }
  u_short remote_port() const { return remote_port_; }

protected:
  YASIO__DECL void enable_multicast_group(const ip::endpoint& ep, int loopback);
  YASIO__DECL int join_multicast_group();
  YASIO__DECL void disable_multicast_group();
  YASIO__DECL int configure_multicast_group(bool onoff);

private:
  YASIO__DECL io_channel(io_service& service, int index);

  void set_address(std::string host, u_short port)
  {
    set_host(host);
    set_port(port);
  }

  YASIO__DECL void set_host(std::string host);
  YASIO__DECL void set_port(u_short port);

  // configure address, check whether needs dns queries
  YASIO__DECL void configure_address();

  // -1 indicate failed, connection will be closed
  YASIO__DECL int __builtin_decode_len(void* ptr, int len);

  /* Since v3.33.0 mask,kind,flags,private_flags are stored to this field
  ** bit[1-8] mask & kinds
  ** bit[9-16] flags
  ** bit[17-24] byte1 of private flags
  ** bit[25~32] byte2 of private flags
  */
  uint32_t properties_ = 0;

  /*
  ** !!! tcp/udp client only, if not zero, will use it as fixed port,
  ** !!! otherwise system will generate a random port for local socket.
  */
  u_short local_port_ = 0;

  /*
  ** !!! tcp/udp client, the port to connect
  ** !!! tcp/udp server, the port to listening
  */
  u_short remote_port_ = 0;

  std::atomic<u_short> dns_queries_state_;

  highp_time_t dns_queries_timestamp_ = 0;

  int index_;
  int socktype_ = 0;

  // The timer for check resolve & connect timeout
  highp_timer timer_;

  struct __unnamed01 {
    int max_frame_length       = YASIO_SZ(10, M); // 10MBytes
    int length_field_offset    = -1;              // -1: directly, >= 0: store as 1~4bytes integer, default value=-1
    int length_field_length    = 4;               // 1,2,3,4
    int length_adjustment      = 0;
    int initial_bytes_to_strip = 0;
  } lfb_;
  decode_len_fn_t decode_len_;

  /*
  !!! for tcp/udp client to bind local specific network adapter, empty for any
  */
  std::string local_host_;

  /*
  !!! for tcp/udp client to connect remote host.
  !!! for tcp/udp server to bind local specific network adapter, empty for any
  !!! for multicast, it's used as multicast address,
      doesn't connect even through recvfrom on packet from remote
  */
  std::string remote_host_;
  std::vector<ip::endpoint> remote_eps_;

  ip::endpoint multiaddr_;

  // Current it's only for UDP
  std::vector<char> buffer_;

#if defined(YASIO_HAVE_KCP)
  int kcp_conv_ = 0;
#endif

#if defined(YASIO_HAVE_SSL)
  ssl_auto_handle ssl_;
#endif

#if defined(YASIO_ENABLE_ARES_PROFILER)
  highp_time_t ares_start_time_;
#endif
};

class io_send_op {
public:
  io_send_op(std::vector<char>&& buffer, completion_cb_t&& handler) : offset_(0), buffer_(std::move(buffer)), handler_(std::move(handler)) {}
  virtual ~io_send_op() {}

  size_t offset_;            // read pos from sending buffer
  std::vector<char> buffer_; // sending data buffer
  completion_cb_t handler_;

  YASIO__DECL virtual int perform(transport_handle_t transport, const void* buf, int n);

#if !defined(YASIO_DISABLE_OBJECT_POOL)
  DEFINE_CONCURRENT_OBJECT_POOL_ALLOCATION(io_send_op, 512)
#endif
};

class io_sendto_op : public io_send_op {
public:
  io_sendto_op(std::vector<char>&& buffer, completion_cb_t&& handler, const ip::endpoint& destination)
      : io_send_op(std::move(buffer), std::move(handler)), destination_(destination)
  {}

  YASIO__DECL int perform(transport_handle_t transport, const void* buf, int n) override;
#if !defined(YASIO_DISABLE_OBJECT_POOL)
  DEFINE_CONCURRENT_OBJECT_POOL_ALLOCATION(io_sendto_op, 512)
#endif
  ip::endpoint destination_;
};

class io_transport : public io_base {
  friend class io_service;
  friend class io_send_op;
  friend class io_sendto_op;
  friend class io_event;

  io_transport(const io_transport&) = delete;

public:
  unsigned int id() const { return id_; }

  ip::endpoint local_endpoint() const { return socket_->local_endpoint(); }
  virtual ip::endpoint remote_endpoint() const { return socket_->peer_endpoint(); }

  io_channel* get_context() const { return ctx_; }
  int cindex() const { return ctx_->index_; }

  virtual ~io_transport() { send_queue_.clear(); }

protected:
  io_service& get_service() const { return ctx_->get_service(); }
  bool is_open() const { return is_valid() && socket_ && socket_->is_open(); }
  std::vector<char> fetch_packet()
  {
    expected_size_ = -1;
    return std::move(expected_packet_);
  }

  // Returns the custom print function
  YASIO__DECL const print_fn2_t& cprint() const;

  // Call at user thread
  YASIO__DECL virtual int write(std::vector<char>&&, completion_cb_t&&);

  // Call at user thread
  virtual int write_to(std::vector<char>&&, const ip::endpoint&, completion_cb_t&&)
  {
    YASIO_LOG("[warning] io_transport doesn't support 'write_to' operation!");
    return 0;
  }

  YASIO__DECL int call_read(void* data, int size, int& error);
  YASIO__DECL int call_write(io_send_op*, int& error);
  YASIO__DECL void complete_op(io_send_op*, int error);

  // Call at io_service
  YASIO__DECL virtual int do_read(int revent, int& error, highp_time_t& wait_duration);

  // Call at io_service, try flush pending packet
  YASIO__DECL virtual bool do_write(highp_time_t& wait_duration);

  // Sets the underlying layer socket io primitives.
  YASIO__DECL virtual void set_primitives();

  YASIO__DECL io_transport(io_channel* ctx, std::shared_ptr<xxsocket>& s);

  bool is_valid() const { return state_ == io_base::state::OPEN; }
  void invalid() { state_ = io_base::state::CLOSED; }

  unsigned int id_;

  char buffer_[YASIO_INET_BUFFER_SIZE]; // recv buffer, 64K
  int wpos_ = 0;                        // recv buffer write pos

  std::vector<char> expected_packet_;
  int expected_size_ = -1;

  io_channel* ctx_;

  std::function<int(const void*, int)> write_cb_;
  std::function<int(void*, int)> read_cb_;

  privacy::concurrent_queue<send_op_ptr> send_queue_;

  // mark whether pollout event registerred.
  bool pollout_registerred_ = false;
#if !defined(YASIO_MINIFY_EVENT)
private:
  // The user data
  union {
    void* ptr;
    int ival;
  } ud_;
#endif
};

class io_transport_tcp : public io_transport {
  friend class io_service;

public:
  io_transport_tcp(io_channel* ctx, std::shared_ptr<xxsocket>& s);
};
#if defined(YASIO_HAVE_SSL)
class io_transport_ssl : public io_transport_tcp {
public:
  YASIO__DECL io_transport_ssl(io_channel* ctx, std::shared_ptr<xxsocket>& s);
  YASIO__DECL void set_primitives() override;

#  if defined(YASIO_HAVE_SSL)
protected:
  ssl_auto_handle ssl_;
#  endif
};
#else
class io_transport_ssl {};
#endif
class io_transport_udp : public io_transport {
  friend class io_service;

public:
  YASIO__DECL io_transport_udp(io_channel* ctx, std::shared_ptr<xxsocket>& s);
  YASIO__DECL ~io_transport_udp();

  YASIO__DECL ip::endpoint remote_endpoint() const override;

protected:
  YASIO__DECL int connect();
  YASIO__DECL int disconnect();

  YASIO__DECL int write(std::vector<char>&&, completion_cb_t&&) override;
  YASIO__DECL int write_to(std::vector<char>&&, const ip::endpoint&, completion_cb_t&&) override;

  YASIO__DECL void set_primitives() override;

  // ensure destination for sendto valid, if not, assign from ctx_->remote_eps_[0]
  YASIO__DECL const ip::endpoint& ensure_destination() const;

  // configure remote with specific endpoint
  YASIO__DECL int confgure_remote(const ip::endpoint& peer);

  // process received data from low level
  YASIO__DECL virtual int handle_input(const char* buf, int bytes_transferred, int& error, highp_time_t& wait_duration);

  ip::endpoint peer_;                // for recv only
  mutable ip::endpoint destination_; // for sendto only
  bool connected_ = false;
};
#if defined(YASIO_HAVE_KCP)
class io_transport_kcp : public io_transport_udp {
public:
  YASIO__DECL io_transport_kcp(io_channel* ctx, std::shared_ptr<xxsocket>& s);
  YASIO__DECL ~io_transport_kcp();
  ikcpcb* internal_object() { return kcp_; }

protected:
  YASIO__DECL int write(std::vector<char>&&, completion_cb_t&&) override;

  YASIO__DECL int do_read(int revent, int& error, highp_time_t& wait_duration) override;
  YASIO__DECL bool do_write(highp_time_t& wait_duration) override;

  YASIO__DECL int handle_input(const char* buf, int len, int& error, highp_time_t& wait_duration) override;

  YASIO__DECL void check_timeout(highp_time_t& wait_duration) const;

  std::vector<char> rawbuf_; // the low level raw buffer
  ikcpcb* kcp_;
  std::recursive_mutex send_mtx_;
};
#else
class io_transport_kcp {};
#endif

class io_event final {
public:
  io_event(int cindex, int kind, int error)
      : cindex_(cindex), kind_(kind), status_(error), transport_(nullptr), packet_({})
#if !defined(YASIO_MINIFY_EVENT)
        ,
        timestamp_(highp_clock()), transport_udata_(nullptr), transport_id_(-1)
#endif
  {}
  io_event(int cindex, int kind, int error, transport_handle_t transport)
      : cindex_(cindex), kind_(kind), status_(error), transport_(transport), packet_({})
#if !defined(YASIO_MINIFY_EVENT)
        ,
        timestamp_(highp_clock()), transport_udata_(transport->ud_.ptr), transport_id_(transport->id_)
#endif
  {}
  io_event(int cindex, int kind, transport_handle_t transport, std::vector<char> packet)
      : cindex_(cindex), kind_(kind), status_(0), transport_(transport), packet_(std::move(packet))
#if !defined(YASIO_MINIFY_EVENT)
        ,
        timestamp_(highp_clock()), transport_udata_(transport->ud_.ptr), transport_id_(transport->id_)
#endif
  {}
  io_event(io_event&& rhs)
      : cindex_(rhs.cindex_), kind_(rhs.kind_), status_(rhs.status_), transport_(rhs.transport_), packet_(std::move(rhs.packet_))
#if !defined(YASIO_MINIFY_EVENT)
        ,
        timestamp_(rhs.timestamp_), transport_udata_(rhs.transport_udata_), transport_id_(rhs.transport_id_)
#endif
  {}
  ~io_event() {}

  int cindex() const { return cindex_; }

  int kind() const { return kind_; }
  int status() const { return status_; }

  std::vector<char>& packet() { return packet_; }
  transport_handle_t transport() const { return transport_; }

#if !defined(YASIO_MINIFY_EVENT)
  /* Gets to transport user data when process this event */
  template <typename _Uty = void*> _Uty transport_udata() const { return (_Uty)(uintptr_t)transport_udata_; }
  /* Sets trasnport user data when process this event */
  template <typename _Uty = void*> void transport_udata(_Uty uval)
  {
    transport_udata_ = (void*)(uintptr_t)uval;
    if (transport_)
      transport_->ud_.ptr = (void*)(uintptr_t)uval;
  }
  unsigned int transport_id() const { return transport_id_; }
  highp_time_t timestamp() const { return timestamp_; }
#endif
#if !defined(YASIO_DISABLE_OBJECT_POOL)
  DEFINE_CONCURRENT_OBJECT_POOL_ALLOCATION(io_event, 512)
#endif
private:
  int cindex_;
  int kind_;
  int status_;
  transport_handle_t transport_;
  std::vector<char> packet_;
#if !defined(YASIO_MINIFY_EVENT)
  highp_time_t timestamp_;
  void* transport_udata_;
  unsigned int transport_id_;
#endif
};

class io_service // lgtm [cpp/class-many-fields]
{
  friend class highp_timer;
  friend class io_transport;
  friend class io_transport_tcp;
  friend class io_transport_udp;
  friend class io_transport_kcp;
  friend class io_channel;

public:
  enum class state
  {
    UNINITIALIZED,
    IDLE,
    RUNNING,
    STOPPING,
  };

  /*
  ** Summary: init global state with custom print function, you must ensure thread safe of it.
  ** @remark:
  **   a. this function is not required, if you don't want print init log to custom console.
  **   b.this function only works once
  **   c. you should call once before call any 'io_servic::start'
  */
  YASIO__DECL static void init_globals(const yasio::inet::print_fn2_t&);

public:
  YASIO__DECL io_service();
  YASIO__DECL io_service(int channel_count);
  YASIO__DECL io_service(const io_hostent& channel_eps);
  YASIO__DECL io_service(const std::vector<io_hostent>& channel_eps);
  YASIO__DECL io_service(const io_hostent* channel_eps, int channel_count);
  YASIO__DECL ~io_service();

  YASIO_OBSOLETE_DEPRECATE(io_service::start)
  void start_service(event_cb_t cb) { this->start(std::move(cb)); }

  YASIO_OBSOLETE_DEPRECATE(io_service::stop)
  void stop_service() { this->stop(); };

  YASIO__DECL void start(event_cb_t cb);
  YASIO__DECL void stop();

  bool is_running() const { return this->state_ == io_service::state::RUNNING; }

  // should call at the thread who care about async io
  // events(CONNECT_RESPONSE,CONNECTION_LOST,PACKET), such cocos2d-x opengl or
  // any other game engines' render thread.
  YASIO__DECL void dispatch(int max_count = 128);

  // set option, see enum YOPT_XXX
  YASIO__DECL void set_option(int opt, ...);
  YASIO__DECL void set_option_internal(int opt, va_list args);

  // open a channel, default: YCK_TCP_CLIENT
  YASIO__DECL void open(size_t index, int kind = YCK_TCP_CLIENT);

  // check whether the channel is open
  YASIO__DECL bool is_open(int index) const;
  // check whether the transport is open
  YASIO__DECL bool is_open(transport_handle_t) const;

  // close transport
  YASIO__DECL void close(transport_handle_t);
  // close channel
  YASIO__DECL void close(int index);

  // the additional API to get rtt of tcp transport
  YASIO__DECL static uint32_t tcp_rtt(transport_handle_t);

  /*
  ** Summary: Write data to a TCP or connected UDP transport with last peer address
  ** @retval: < 0: failed
  ** @params:
  **        'thandle': the transport to write, could be tcp/udp/kcp
  **        'buf': the data to write
  **        'len': the data len
  **        'handler': send finish callback, only works for TCP transport
  ** @remark:
  **        + TCP/UDP: Use queue to store user message, flush at io_service thread
  **        + KCP: Use queue provided by kcp internal, flush at io_service thread
  */
  int write(transport_handle_t thandle, const void* buf, size_t len, completion_cb_t completion_handler = nullptr)
  {
    return write(thandle, std::vector<char>((char*)buf, (char*)buf + len), std::move(completion_handler));
  }
  YASIO__DECL int write(transport_handle_t thandle, std::vector<char> buffer, completion_cb_t completion_handler = nullptr);

  /*
   ** Summary: Write data to unconnected UDP transport with specified address.
   ** @retval: < 0: failed
   ** @remark: This function only for UDP like transport (UDP or KCP)
   **        + UDP: Use queue to store user message, flush at io_service thread
   **        + KCP: Use the queue provided by kcp internal, flush at io_service thread
   */
  int write_to(transport_handle_t thandle, const void* buf, size_t len, const ip::endpoint& to, completion_cb_t completion_handler = nullptr)
  {
    return write_to(thandle, std::vector<char>((char*)buf, (char*)buf + len), to, std::move(completion_handler));
  }
  YASIO__DECL int write_to(transport_handle_t thandle, std::vector<char> buffer, const ip::endpoint& to, completion_cb_t completion_handler = nullptr);

  // The highp_timer support, !important, the callback is called on the thread of io_service
  YASIO__DECL highp_timer_ptr schedule(const std::chrono::microseconds& duration, timer_cb_t);

  YASIO_OBSOLETE_DEPRECATE(io_service::resolve)
  YASIO__DECL int builtin_resolv(std::vector<ip::endpoint>& endpoints, const char* hostname, unsigned short port = 0)
  {
    return this->resolve(endpoints, hostname, port);
  }

  YASIO__DECL int resolve(std::vector<ip::endpoint>& endpoints, const char* hostname, unsigned short port = 0);

  YASIO_OBSOLETE_DEPRECATE(io_service::channel_at)
  io_channel* cindex_to_handle(size_t index) const { return channel_at(index); }

  // Gets channel by index
  YASIO__DECL io_channel* channel_at(size_t index) const;

private:
  YASIO__DECL void schedule_timer(highp_timer*, timer_cb_t&&);
  YASIO__DECL void remove_timer(highp_timer*);

  std::vector<timer_impl_t>::iterator find_timer(highp_timer* key)
  {
    return yasio__find_if(timer_queue_, [=](const timer_impl_t& timer) { return timer.first == key; });
  }
  void sort_timers()
  {
    std::sort(this->timer_queue_.begin(), this->timer_queue_.end(),
              [](const timer_impl_t& lhs, const timer_impl_t& rhs) { return lhs.first->wait_duration() > rhs.first->wait_duration(); });
  }

  // Start a async resolve, It's only for internal use
  YASIO__DECL void start_resolve(io_channel*);

  YASIO__DECL void init(const io_hostent* channel_eps /* could be nullptr */, int channel_count);
  YASIO__DECL void cleanup();

  YASIO__DECL void handle_stop();

  /* Call by stop_service, wait io_service thread exit properly & do cleanup */
  YASIO__DECL void join();

  YASIO__DECL void open_internal(io_channel*);

  YASIO__DECL void process_transports(fd_set* fds_array);
  YASIO__DECL void process_channels(fd_set* fds_array);
  YASIO__DECL void process_timers();

  YASIO__DECL void interrupt();

  YASIO__DECL highp_time_t get_timeout(highp_time_t usec);

  YASIO__DECL int do_select(fd_set* fds_array, highp_time_t wait_duration);

  YASIO__DECL void do_nonblocking_connect(io_channel*);
  YASIO__DECL void do_nonblocking_connect_completion(io_channel*, fd_set* fds_array);

#if defined(YASIO_HAVE_SSL)
  YASIO__DECL void init_ssl_context();
  YASIO__DECL void cleanup_ssl_context();
  YASIO__DECL SSL_CTX* get_ssl_context();
  YASIO__DECL void do_ssl_handshake(io_channel*);
#endif

#if defined(YASIO_HAVE_CARES)
  static void ares_getaddrinfo_cb(void* arg, int status, int timeouts, ares_addrinfo* answerlist);
  void ares_work_started() { ++ares_outstanding_work_; }
  void ares_work_finished()
  {
    if (ares_outstanding_work_ > 0)
      --ares_outstanding_work_;
  }
  YASIO__DECL void process_ares_requests(fd_set* fds_array);
  YASIO__DECL void recreate_ares_channel();
  YASIO__DECL void config_ares_name_servers();
  YASIO__DECL void destroy_ares_channel();
#endif

  void handle_connect_succeed(io_channel* ctx, std::shared_ptr<xxsocket> socket) { handle_connect_succeed(allocate_transport(ctx, std::move(socket))); }
  YASIO__DECL void handle_connect_succeed(transport_handle_t);
  YASIO__DECL void handle_connect_failed(io_channel*, int ec);
  YASIO__DECL void notify_connect_succeed(transport_handle_t);

  YASIO__DECL transport_handle_t allocate_transport(io_channel*, std::shared_ptr<xxsocket>);
  YASIO__DECL void deallocate_transport(transport_handle_t);

  YASIO__DECL void register_descriptor(const socket_native_type fd, int flags);
  YASIO__DECL void unregister_descriptor(const socket_native_type fd, int flags);

  // The major non-blocking event-loop
  YASIO__DECL void run(void);

  YASIO__DECL bool do_read(transport_handle_t, fd_set* fds_array);
  bool do_write(transport_handle_t transport) { return transport->do_write(this->wait_duration_); }
  YASIO__DECL void unpack(transport_handle_t, int bytes_expected, int bytes_transferred, int bytes_to_strip);

  // The op mask will be cleared, the state will be set CLOSED when clear_state is 'true'
  YASIO__DECL bool cleanup_io(io_base* obj, bool clear_state = true);

  YASIO__DECL void handle_close(transport_handle_t);
  YASIO__DECL void handle_event(event_ptr event);

  // new/delete client socket connection channel
  // please call this at initialization, don't new channel at runtime
  // dynmaically: because this API is not thread safe.
  YASIO__DECL void create_channels(const io_hostent* eps, int count);
  // Clear all channels after service exit.
  YASIO__DECL void clear_channels();   // destroy all channels
  YASIO__DECL void clear_transports(); // destroy all transports
  YASIO__DECL bool close_internal(io_channel*);

  // shutdown a tcp-connection if possible
  YASIO__DECL bool shutdown_internal(transport_handle_t);

  /*
  ** Summay: Query async resolve state for new endpoint set
  ** @retval:
  **   YDQS_READY, YDQS_INPRROGRESS, YDQS_FAILED
  ** @remark: will start a async resolv when the state is: YDQS_DIRTY
  */
  YASIO__DECL u_short query_ares_state(io_channel* ctx);

  // supporting server
  YASIO__DECL void do_nonblocking_accept(io_channel*);
  YASIO__DECL void do_nonblocking_accept_completion(io_channel*, fd_set* fds_array);

  YASIO__DECL static const char* strerror(int error);

  /*
  ** Summary: For udp-server only, make dgram handle to communicate with client
  */
  YASIO__DECL transport_handle_t do_dgram_accept(io_channel*, const ip::endpoint& peer, int& error);

  int local_address_family() const { return ((ipsv_ & ipsv_ipv4) || !ipsv_) ? AF_INET : AF_INET6; }

  /* Returns the custom print function */
  inline const print_fn2_t& cprint() const { return options_.print_; }

private:
  state state_ = state::UNINITIALIZED; // The service state
  std::thread worker_;
  std::thread::id worker_id_;

  privacy::concurrent_queue<event_ptr, true> events_;

  std::vector<io_channel*> channels_;

  std::recursive_mutex channel_ops_mtx_;
  std::vector<io_channel*> channel_ops_;

  std::vector<transport_handle_t> transports_;
  std::vector<transport_handle_t> tpool_;

  // select interrupter
  select_interrupter interrupter_;

  // timer support timer_pair
  std::vector<timer_impl_t> timer_queue_;
  std::recursive_mutex timer_queue_mtx_;

  // the next wait duration for socket.select
  highp_time_t wait_duration_;

  // the max nfds for socket.select, must be max_fd + 1
  int max_nfds_;
  enum
  {
    read_op,
    write_op,
    except_op,
    max_ops,
  };
  fd_set fds_array_[max_ops];

  // options
  struct __unnamed_options {
    highp_time_t connect_timeout_     = 10LL * std::micro::den;
    highp_time_t dns_cache_timeout_   = 600LL * std::micro::den;
    highp_time_t dns_queries_timeout_ = 5LL * std::micro::den;
    int dns_queries_tries_            = 5;

    bool dns_dirty_                   = true; // only for c-ares

    bool deferred_event_ = true;

    // tcp keepalive settings
    struct __unnamed01 {
      int onoff    = 0;
      int idle     = 7200;
      int interval = 75;
      int probs    = 10;
    } tcp_keepalive_;

    bool no_new_thread_    = false;
    bool ignore_udp_error_ = true;

    // The resolve function
    resolv_fn_t resolv_;
    // the event callback
    event_cb_t on_event_;
    // The custom debug print function
    print_fn2_t print_;

#if defined(YASIO_HAVE_SSL)
    // The full path cacert(.pem) file for ssl verifaction
    std::string capath_;
#endif
  } options_;

  // The ip stack version supported by localhost
  u_short ipsv_ = 0;
#if defined(YASIO_HAVE_SSL)
  SSL_CTX* ssl_ctx_ = nullptr;
#endif
#if defined(YASIO_HAVE_CARES)
  ares_channel ares_         = nullptr; // the ares handle for non blocking io dns resolve support
  int ares_outstanding_work_ = 0;
#else
  // we need life_token + life_mutex
  struct life_token {};
  std::shared_ptr<life_token> life_token_;
  std::shared_ptr<cxx17::shared_mutex> life_mutex_;
#endif
}; // io_service

} // namespace inet
} /* namespace yasio */

#define yasio_shared_service yasio::gc::singleton<yasio::inet::io_service>::instance

#if defined(YASIO_HEADER_ONLY)
#  include "yasio/yasio.cpp" // lgtm [cpp/include-non-header]
#endif

#endif

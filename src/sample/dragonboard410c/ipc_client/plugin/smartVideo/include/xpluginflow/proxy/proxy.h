//
// Created by zhuoran.rong@horizon.ai on 11/21/2019.
// Copyright (c) 2019 horizon robotics. All rights reserved.
//
#ifndef INCLUDE_XPLUGINFLOW_PROXY_PROXY_H_
#define INCLUDE_XPLUGINFLOW_PROXY_PROXY_H_

#include <nng/nng.h>  // TODO(zhuoran.rong) chang to artifact
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/reqrep0/req.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <atomic>
#include <thread>
#include <vector>

#include "xpluginflow/message/pluginflow/flowmsg.h"
#include "xpluginflow/proxy/proxy_types.h"
#include "xpluginflow/utils/singleton.h"
#include "xpluginflow/proxy/ipc_onboard.h"

#define gettid() syscall(__NR_gettid)

namespace horizon {
namespace vision {
namespace xpluginflow {

extern std::mutex log_output_mutex;

#if 0
#define DUMP(a)                                                                \
  do {                                                                         \
    log_output_mutex.lock();                                                   \
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "," << gettid()         \
              << "]: " << #a << " => " << a << std::endl;                      \
    log_output_mutex.unlock();                                                 \
  } while (0)

#define DUMP_UUID(u)                                                           \
  do {                                                                         \
    log_output_mutex.lock();                                                   \
    fprintf(stderr, "[%s:%d,%d]: %s => ", __FILE__, __LINE__, gettid(), #u);   \
    for (int i = 0; i < 15; i++) {                                             \
      fprintf(stderr, "%02X-", (u)[i]);                                        \
    }                                                                          \
    fprintf(stderr, "%02X\n", (u)[15]);                                        \
    log_output_mutex.unlock();                                                 \
  } while (0)

#define DEBUG(fmt, ...)                                                        \
  do {                                                                         \
    log_output_mutex.lock();                                                   \
    fprintf(stderr, "[%s:%d,%d]: " fmt "\n", __FILE__, __LINE__, gettid(),     \
            ##__VA_ARGS__);                                                    \
    log_output_mutex.unlock();                                                 \
  } while (0)
#else
#define DUMP(a)
#define DUMP_UUID(u)
#define DEBUG(fmt, ...)
#endif

struct router_item;
struct xproxy_session;

extern struct domain_configuration hbipc_domain_config;

#ifndef XPROXY_HBIPC_DOMAIN_ID
#define XPROXY_HBIPC_DOMAIN_ID 0
#endif

// 内部总线回调函数
typedef std::function<void(XPluginFlowMessagePtr)> bus_cb_t;
typedef std::function<XPluginFlowMessagePtr(const std::string &,
                                            const std::string &)>
    unserial_cb_t;
// 本地路由表类型
typedef std::map<std::string, std::vector<std::vector<struct router_item>>>
    router_table_t;
// 本地连接会话表
typedef std::map<uuid_t, std::shared_ptr<struct xproxy_session>>
    session_table_t;

// 控制信令
enum CONTROL_MSG_TYPE {
  // 路由通告
  ROUTER_DECLEAR = 0,
  // 消息分发
  MSG_DISTRIBUTE,
};

enum router_item_type {
  ROUTER_TYPE_PUBLISH = 0,
  ROUTER_TYPE_SUBSCRIBE,
  ROUTER_TYPE_MAX,
};

// 本地订阅表条目类型
struct router_item {
  // subscribe,publish,ap,cp
  uint32_t subscribe : 1;
  uint32_t ap : 1;
  // 进程的pid(目前未用到)
  uint32_t pid;
  uuid_t service_id;
};

// 到对端proxy的会话
// TODO(zhuoran.rong) 支持进程间
struct xproxy_session {
  // hbipc session
  xproxy_session() {
    registed = false;
    exit_flag = false;
  }
  struct session hbipc_session;
  uuid_t service_id;
  std::thread thread;
  bool registed;
  volatile bool exit_flag;
};

// hbipc hooks
union hbipc_hooks {
  struct hbipc_ap ap;
  struct hbipc_cp cp;
};

// 代理
class XProxy : public hobot::CSingleton<XProxy> {
 public:
  XProxy() {
    // 初始化路由表的读写锁
    pthread_rwlock_init(&router_table_lock, NULL);
    pthread_rwlock_init(&session_lock, NULL);
    inited = false;
  }
  ~XProxy() = default;

 public:
  // 连接gateway,创建接收广播线程
  int Init(bus_cb_t, unserial_cb_t);
  // 反初始化
  int Deinit();
  // plugin订阅消息
  int RegisterMsg(std::string);

  // plugin发布消息
  int PublishMsg(XPluginFlowMessagePtr);
  static int IsAP(void);

 public:
  // 判断当前是ap还是cp
  bool is_ap = true;
  // nng超时时间
  int timeout = 1000;

 protected:
  // 接收线程
  int DoIpcOnBoardRecv();   // 板内进程间通信消息接收
  int DoBroadcastReceiver();
  int DoHBIPCAccept();
  int DoSessionRecv(std::shared_ptr<struct xproxy_session>);
  // 将HBIPC收到的数据转发到内部总线
  int ProxyToBus(std::string, const std::string &);
  int BusToProxy(XPluginFlowMessagePtr);

 private:
  int init_hbipc_hooks();
  int bind_symbols(void **, const char *, void *);
  // 处理gateway应答
  int process_gw_response(nng_socket);
  // 处理消息订阅
  int do_msg_regist(uint32_t , gw_register_info_t *);
  // 处理消息取消订阅
  int do_msg_unregist(uint32_t , gw_register_info_t *);
  // 本地路由表操作
  bool is_subscribed(const std::string &);  // 是否订阅了该消息类型
  int insert_router_map(const std::string &, gw_register_info_t *);
  int delete_router_map(const std::string &, gw_register_info_t *);
  int connect_to_gateway(nng_socket *);
  int hbipc_send(struct session *, uint32_t, char *, size_t);
  int hbipc_revc(struct session *, uint32_t *, char **, size_t *);
  // 扫描路由表,创建板内连接
  int check_dialer();
  // 扫描路由表,创建板间连接
  int check_session();
  int check_connect(const struct router_item &);
  // 将路由注册到gateway中
  int router_regist_gateway(std::string, int);
  void random_uuid(unsigned char[16]);
  int hbipc_ap_init(const char *);
  int hbipc_cp_init(const char *);

 private:
  volatile bool exit_flag = false;
  // 本地路由表,记录msg到service的订阅关系
  pthread_rwlock_t router_table_lock;
  router_table_t router_table;

  // session 会话索引,用来通过service id索引HBIPC会话
  pthread_rwlock_t session_lock;
  session_table_t session_table;

  // 发送到内部总线的接口
  bus_cb_t pub_internal;
  // 消息反序列化接口
  unserial_cb_t unserialize_cb;

  std::thread broadcast_receiver;
  std::mutex hbipc_accepter_mutex;
  std::map<const char *, std::thread> hbipc_accepter;

  // 初始化参数
  std::atomic<bool> inited;
  std::mutex inited_lock;
  // hbipc参数
  uuid_t service_id;

  hbipc_hooks hooks;

  IpcOnBoard ipc_onboard;     // 板内进程间通信管理类
  std::thread ipc_receiver;   // 板内进程间通信消息接收线程
};

}  // namespace xpluginflow
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_XPLUGINFLOW_PROXY_PROXY_H_

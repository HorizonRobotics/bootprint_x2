//
// Created by zhuoran.rong@horizon.ai on 11/21/2019.
// Copyright (c) 2019 horizon robotics. All rights reserved.
//
#ifndef INCLUDE_XPLUGINFLOW_PROXY_IPC_ONBOARD_H_
#define INCLUDE_XPLUGINFLOW_PROXY_IPC_ONBOARD_H_

#include <unistd.h>
#include <nng/nng.h>

#include <string>
#include <map>
#include <mutex>

namespace horizon {
namespace vision {
namespace xpluginflow {

// 负责AP或CP板内进程间通信, 使用nng总线模式
// 消息订阅者主动连接消息发布者

// 路由通告, 防止重复连接, 收到该消息表示进程间已经建立连接
struct router_declear_t {
  int control;  // 控制命令
  int pid;      // 当前进程PID
};

class IpcOnBoard {
 public:
  IpcOnBoard();
  ~IpcOnBoard();

 public:
  int init();
  int deinit();
  // 与进程号为pid的进程建立连接
  int connect(int pid);
  // 向nng总线发送消息
  int send_message(std::string type, std::string msg);
  // 收到路由通告后保存连接关系
  int insert_dialer(int pid);
  // 删除与进程号为pid的进程之间的连接
  void remove_dialer(int pid);
  // 获取socket
  nng_socket get_socket();

 private:
  // 生成url，格式为ipc:///tmp/pid.ipc
  std::string generate_url(int pid);

 private:
  // 当前进程PID
  int pid;
  // 当前进程url
  std::string bus_url;
  // 当前进程socket
  nng_socket bus_socket;

  // 板内nng总线连接表, key: pid, value: dialer
  typedef std::map<int, nng_dialer> dialer_table_t;
  dialer_table_t dialer_table;
  std::mutex dialer_mutex;
};

}  // namespace xpluginflow
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_XPLUGINFLOW_PROXY_IPC_ONBOARD_H_

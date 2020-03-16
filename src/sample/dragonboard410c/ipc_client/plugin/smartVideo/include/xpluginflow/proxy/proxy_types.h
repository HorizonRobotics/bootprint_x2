/*
 * proxy_types.h
 *
 *  Created on: 2019-11-26
 *      Author: virusv(zhuoran.rong@horizon.ai)
 *	Copyright (c) 2019 horizon robotics. All rights reserved.
 */

#ifndef INCLUDE_XPLUGINFLOW_PROXY_PROXY_TYPES_H_
#define INCLUDE_XPLUGINFLOW_PROXY_PROXY_TYPES_H_

#include <stdint.h>
#include <string.h>

#include "xpluginflow/proxy/hbipc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 错误码定义
#define MODULE_SYSTEM 0    // 系统相关错误
#define MODULE_BISINESS 1  // 业务相关错误
#define MODULE_HBIPC 2     // HBIPC的错误
#define MODULE_NNG 3       // NNG侧的错误
#define MODULE_MESSAGE 4   // 通用消息

#define RETURN_ERROR_CODE(MODULE, CODE)                                        \
  (int32_t)(0x80000000u | (((MODULE)&0x7) << 28) |                             \
            (uint32_t)((CODE) > 0 ? (CODE) : -(CODE)))
enum {
#define GENERATE_ERROR_CODE(MODULE, NAME, CODE)                                \
  XPROXY_ERR_##NAME = RETURN_ERROR_CODE(MODULE, CODE)
  // 系统错误
  GENERATE_ERROR_CODE(MODULE_SYSTEM, NO_RESOURCE, 1),
  GENERATE_ERROR_CODE(MODULE_SYSTEM, INVAILD_PARAMETER, 2),
  GENERATE_ERROR_CODE(MODULE_SYSTEM, ALREADY_RUNNING, 3),
  GENERATE_ERROR_CODE(MODULE_SYSTEM, INIT_FAILED, 4),
  GENERATE_ERROR_CODE(MODULE_SYSTEM, UNKNOW_HOOK, 5),
  GENERATE_ERROR_CODE(MODULE_SYSTEM, LOAD_DYN_FAILED, 6),

  // 业务相关错误
  GENERATE_ERROR_CODE(MODULE_BISINESS, REQUEST_FAILED, 1),
  GENERATE_ERROR_CODE(MODULE_BISINESS, BAD_RESPONSE, 2),
  GENERATE_ERROR_CODE(MODULE_BISINESS, UNSUBS_MSG, 3),

  // 消息相关
  GENERATE_ERROR_CODE(MODULE_MESSAGE, INVALID_MESSAGE, 1),

#undef GENERATE_ERROR_CODE
};

// gateway连接信息
#define XMSG_GATEWAY_CONNECT "ipc:///tmp/register.ipc"
#define XMSG_GATEWAY_BROADCAST "ipc:///tmp/broadcast.ipc"

// 5KB
#define HBIPC_BUF_SIZE (5 << 10)

// uuid warpper
struct uuid_t {
  uuid val;

 public:
  uuid_t() {}
  explicit uuid_t(const uuid &orig) { memcpy(val, orig, sizeof(uuid)); }
  inline bool operator<(const uuid_t &rhs) const {
    return memcmp(val, rhs.val, sizeof(uuid)) < 0;
  }

  inline uuid_t &operator=(const uuid_t &r) {
    memcpy(val, r.val, sizeof(uuid));
    return *this;
  }

  inline bool operator==(const uuid_t b) const {                  // NOLINT
    return memcmp((void *)val, (void *)b.val, sizeof(uuid)) == 0; // NOLINT
  }

  inline bool operator!=(const uuid_t b) const {
    return memcmp((void *)val, (void *)b.val, sizeof(uuid)) != 0; // NOLINT
  }
};

// 注册信息类型
#define XMSG_SUBSCRIBE 0x01
#define XMSG_PUBLISH 0x02

// 设备类型
#define XMSG_DEVICE_CP 0
#define XMSG_DEVICE_AP 1

// hbipc_ap hooks
struct hbipc_ap {
  int (*init)(const char *, int, int, struct domain_configuration *);
  int (*deinit)(int);
  int (*sendframe)(struct session *, char *, int);
  int (*recvframe)(struct session *, char *, int);
  int (*startserver)(int, uuid, char *, int);
  int (*stopserver)(int, int);
  int (*connect)(int, int, struct session *);
  int (*disconnect)(struct session *);
};

// hbipc_cp hooks
struct hbipc_cp {
  int (*init)(const char *, uuid, int, int, struct domain_configuration *);
  int (*deinit)(int, uuid);
  int (*sendframe)(struct session *, char *, int);
  int (*recvframe)(struct session *, char *, int);
  int (*accept)(int, uuid, struct session *, int);
};

// 操作命令
enum XMSG_COMMOND {
  XMSG_RESPOND_ERROR = 0x81,   // 回复失败
  XMSG_RESPOND_SUCCESS = 0x00,  // 回复成功
  XMSG_REGISTER = 0x01,        // 注册
  XMSG_UNREGISTER = 0x02       // 取消注册
};

// 报文头部
typedef struct {
  uint32_t commond;           // 操作命令
  uint32_t register_info_num;  // 注册信息数量
} gw_header_t;

// 注册信息
typedef struct {
  uint16_t register_type;       // 1: subscribe, 2: publish
  uint16_t device_type;         // 0: CP, 1: AP
  uint32_t domain;              // 通信域名称
  uuid server_id;               // 服务ID, CP填写
  uint32_t process_id;          // 进程ID
  uint32_t message_type_length;  // 消息类型长度
  char message_type[0];         // 消息类型
} gw_register_info_t;

#ifdef __cplusplus
}
#endif
#endif  // INCLUDE_XPLUGINFLOW_PROXY_PROXY_TYPES_H_

/*
 * hbipc_common.h
 *
 *  Created on: 2019-11-28
 *      Author: virusv(zhuoran.rong@horizon.ai)
 *	Copyright (c) 2019 horizon robotics. All rights reserved.
 */

#ifndef INCLUDE_XPLUGINFLOW_PROXY_HBIPC_COMMON_H_
#define INCLUDE_XPLUGINFLOW_PROXY_HBIPC_COMMON_H_


#define VERSION "2.4.0"

#define IN
#define OUT

#define DOMAIN_MAX_NUM (6)
struct domain_info {
    char *domain_name;
    int domain_id;
    char *device_name;
};

struct domain_configuration {
    int domain_number;
    struct domain_info domain_array[DOMAIN_MAX_NUM];
};

#define UUID_LEN (16)
typedef unsigned char uuid[UUID_LEN];

#define UUID_DEFINE(name, u0, u1, u2, u3, u4, \
        u5, u6, u7, u8, u9, u10, u11, u12, u13, u14, u15)\
        uuid name = {u0, u1, u2, u3, u4, u5, u6, u7, \
                u8, u9, u10, u11, u12, u13, u14, u15}

struct session {
    int domain_id;
    int provider_id;
    int client_id;
};

/* transfer feature */
#define TF_NONBLOCK (1 << 0)
#define TF_BLOCK    ~(1 << 0)

#endif /* INCLUDE_XPLUGINFLOW_PROXY_HBIPC_COMMON_H_ */

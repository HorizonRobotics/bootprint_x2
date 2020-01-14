/*
 * @Author: your name
 * @Date: 2020-01-13 22:12:57
 * @LastEditTime : 2020-01-13 22:13:07
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /common/xroc/tutorials/stage1/method_factory.cc
 */
/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      method_factory.cc
 * @brief     MethodFactory class implementation
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-01-03
 */

#include "method_factory.h"
#include "method/b_box_filter.h"

namespace HobotXRoc {

MethodPtr MethodFactory::CreateMethod(const std::string &method_name) {
  if ("BBoxFilter" == method_name) {
    return MethodPtr(new BBoxFilter());
  } else {
    return MethodPtr();
  }
}

} // namespace HobotXRoc

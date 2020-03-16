/*!
 * -------------------------------------------
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     msg_registry.h
 * \Author Songshan Gong
 * \Mail     songshan.gong@horizon.ai
 * \Version  1.0.0.0
 * \Date     2019-07-30
 * \Brief    xplugin message registry
 * \DO NOT MODIFY THIS COMMENT, \
 * \WHICH IS AUTO GENERATED BY EDITOR
 * -------------------------------------------
 */

#ifndef INCLUDE_XPLUGINFLOW_MESSAGE_PLUGINFLOW_MSG_REGISTRY_H_
#define INCLUDE_XPLUGINFLOW_MESSAGE_PLUGINFLOW_MSG_REGISTRY_H_

#include <map>
#include <mutex>
#include <string>
#include "hobotlog/hobotlog.hpp"

namespace horizon {
namespace vision {
namespace xpluginflow {
typedef int32_t XPluginMsgTypeHandle;
#define XPLUGIN_INVALID_MSG_TYPE -1
class XPluginMsgRegistry {
 public:
  inline XPluginMsgTypeHandle RegisterOrGet(const std::string& name) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (fmap_.count(name) == 0) {
      fmap_[name] = counter_++;
      return fmap_[name];
    } else {
      return fmap_.at(name);
    }
  }

  inline XPluginMsgTypeHandle Register(const std::string& name) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (fmap_.count(name)) {
      LOGF << "XPlugin msg type:" << name << " already registered!";
      return XPLUGIN_INVALID_MSG_TYPE;
    }
    fmap_[name] = counter_++;
    return fmap_[name];
  }

  inline XPluginMsgTypeHandle Get(const std::string& name) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (fmap_.count(name)) {
      return fmap_.at(name);
    } else {
      return XPLUGIN_INVALID_MSG_TYPE;
    }
  }
  static XPluginMsgRegistry &Instance() {
    static XPluginMsgRegistry inst;
    return inst;
  }

 private:
  std::mutex mutex_;
  std::map<std::string, XPluginMsgTypeHandle> fmap_;
  int32_t counter_{0};

  XPluginMsgRegistry() {}
  ~XPluginMsgRegistry() {}
};

// #define XPLUGIN_REGISTER_MSG_TYPE(key)                      \
// static horizon::vision::xpluginflow::XPluginMsgTypeHandle   \
//   __make__##key##__xplugin_msg_type__ =                     \
//   horizon::vision::xpluginflow::XPluginMsgRegistry::Instance().Register(#key);

}  // namespace xpluginflow
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_XPLUGINFLOW_MESSAGE_PLUGINFLOW_MSG_REGISTRY_H_
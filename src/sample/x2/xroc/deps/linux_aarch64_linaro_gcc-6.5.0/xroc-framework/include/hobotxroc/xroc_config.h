/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     the config of xsoul framework
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */
#ifndef HOBOTXROC_XROC_CONFIG_H_
#define HOBOTXROC_XROC_CONFIG_H_

#include <limits.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include "json/json.h"
#include "hobotxsdk/xroc_error.h"
#include "hobotlog/hobotlog.hpp"

namespace HobotXRoc
{

enum CONFIG_ERROR
{
    CONFIG_OK = 0,
    INPUT_UNFEED_ERROR,
    OUTPUT_USELESS_ERROR,
    WORKFLOW_CIRCLE_ERROR,
    OUTPUT_REPEATED_ERROR,
    NODE_NAME_ERROR,
};

typedef Json::Value Config;

class XRocConfig {  //  配置模块
 public:
    XRocConfig() = default;

    virtual ~XRocConfig() {}

 public:
    int LoadFile(const std::string &file_path);
    std::string folder_path_;
    Config cfg_jv_;
};

typedef std::shared_ptr<XRocConfig> XRocConfigPtr;

struct XRocSharedConfig {
  int source_num_ = 1;
};
typedef std::shared_ptr<XRocSharedConfig> XRocSharedConfigPtr;
class SchedulerConfig {  //  配置模块
 public:
    explicit SchedulerConfig(XRocConfigPtr config);

    virtual ~SchedulerConfig() {}

 public:
    std::map<std::string, std::vector<std::string>> GetFlowOutputs() const
    {
        return flow_outputs_;
    }
    std::vector<std::string> GetFlowOutputsUnion() const
    {
        return flow_outputs_union_;
    }

    std::vector<std::string> GetNodesName() const { return nodes_names_; }

    std::vector<std::string> GetNodeInputs(const std::string &node_name) const
    {
        auto itr = nodes_inputs_.find(node_name);
        if (itr != nodes_inputs_.end())
        {
            return itr->second;
        }
        return {};
    }

    std::vector<std::string> GetNodeOutputs(const std::string &node_name)
    {
        auto itr = nodes_outputs_.find(node_name);
        if (itr != nodes_outputs_.end())
        {
            return itr->second;
        }
        return {};
    }

    Config &GetNodeConfig(const std::string &node_name)
    {
        return nodes_config_[node_name];
    }

    XRocSharedConfig GetSharedConfg() const {
        XRocSharedConfig config;
        config.source_num_ = source_num_;
        return config;
    }

    int GetMaxRunningCount() const { return max_running_count_; }

    uint32_t GetSourceNumber() const { return source_num_; }

    std::string GetFolderPath() const {
        return config_->folder_path_;
    }

    Config &GetOptionalConfig()
    {
        return optional_config_;
    }


    // int CheckOutputRedundant();
    int CheckInputValid();
    int CheckUniqueName();
    int CheckIsCircle();
    int CheckRepeatedOutput();

    int CheckConfigValid()
    {
        if (CheckUniqueName()) {
            return NODE_NAME_ERROR;
        } else if (CheckInputValid()) {
            return INPUT_UNFEED_ERROR;
        } else if (CheckIsCircle()) {
            return WORKFLOW_CIRCLE_ERROR;
        } else if (CheckRepeatedOutput()) {
            return OUTPUT_REPEATED_ERROR;
        }

        return 0;
    }

 protected:
  XRocConfigPtr config_;
  int max_running_count_ = INT_MAX;
  uint32_t source_num_        = 1;
  std::vector<std::string> nodes_names_;
  std::vector<std::string> flow_inputs_;
  // 支持多路输出
  std::map<std::string, std::vector<std::string>> flow_outputs_;
  // 多路输出并集
  std::vector<std::string> flow_outputs_union_;
  std::map<std::string, std::vector<std::string>> nodes_inputs_;
  std::map<std::string, std::vector<std::string>> nodes_outputs_;
  std::map<std::string, Config> nodes_config_;
  Config optional_config_;
};

typedef std::shared_ptr<SchedulerConfig> SchedulerConfigPtr;

}  //  namespace HobotXRoc

#endif  //  XROC_FRAMEWORK_INCLUDE_HOBOTXROC_XROC_CONFIG_H_

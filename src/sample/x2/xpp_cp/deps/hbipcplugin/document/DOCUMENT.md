# HBIPCPlugin 使用文档

## 介绍
HbipcPlugin（Hbipc Plugin）为跨板传输插件，主要负责实现AP<sup>*1*</sup>与CP<sup>*2*</sup>端的双向通信功能，用于AP与CP间的BIF<sup>*3*</sup>通信功能，CP端通过HbipcPlugin将智能帧等数据发送到AP端，或者通过HbipcPlugin接收AP端发过来的配置数据与命令数据。

## 使用
HBIPCPlugin作为XPluginFlow的插件, 符合XPluginFlow中定义的Plugin标准接口.

### 配置文件
配置文件中记录的是HBIPC的配置参数:
```json
{
  "domain_name": "X2SD001",
  "server_id": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14]
}
```
其中`domain_name`表示HBIPC通信域<sup>*4*</sup>, 可选的值有`X2SD001`和`X2BIF001`.  
`server_id`表示CP侧的服务ID, UUID格式. AP侧在建立连接的时候会用到该UUID.

## 生产消息类型
当HBIPCPlugin收到AP侧发来的配置数据和命令数据时,会将其以`XPLUGIN_HBIPC_MESSAGE`类型的消息发送到XPluginFlow的内部总线.  
消息对应的结构体定义如下:
```c++
struct HbipcMessage : XPluginFlowMessage {
  HbipcMessage() { type_ = "XPLUGIN_HBIPC_MESSAGE"; }
  std::string Serialize() override { return "Default hbipc message"; }
  virtual ~HbipcMessage() = default;
};

struct CustomHbipcMessage : HbipcMessage {
  explicit CustomHbipcMessage(std::string proto) : proto_(proto) {
    type_ = "XPLUGIN_HBIPC_MESSAGE";
  }
  std::string Serialize() override;

 private:
  std::string proto_;
};
```

## 消费消息类型
HBIPCPlugin负责将VioPlugin和SmartPlugin产生的消息发送到AP侧, 所以它需要监听总线上面的`XPLUGIN_SMART_MESSAGE`, `XPLUGIN_IMAGE_MESSAGE`和`XPLUGIN_DROP_MESSAGE`. 
+ `XPLUGIN_SMART_MESSAGE`: 智能数据帧.  
    ```c++
    #define TYPE_SMART_MESSAGE "XPLUGIN_SMART_MESSAGE"

    struct SmartMessage : XPluginFlowMessage {
    uint64_t time_stamp;
    uint64_t frame_id;
    SmartMessage() { type_ = TYPE_SMART_MESSAGE; }
    virtual ~SmartMessage() = default;

    std::string Serialize() override { return "Default smart message"; };
    };
    ```
+ `XPLUGIN_IMAGE_MESSAGE`: 图像数据帧.  
+ `XPLUGIN_DROP_MESSAGE`: 报告VIOPlugin丢弃的帧.
    ```c++
    #define TYPE_IMAGE_MESSAGE "XPLUGIN_IMAGE_MESSAGE"
    #define TYPE_DROP_MESSAGE "XPLUGIN_DROP_MESSAGE"

    struct VioMessage : public XPluginFlowMessage {
    public:
        VioMessage() { type_ = TYPE_IMAGE_MESSAGE; };
        virtual ~VioMessage() = default;

        // image frames number
        uint32_t num_ = 0;
        // sequence id, would increment automatically
        uint64_t sequence_id_ = 0;
        // time stamp
        uint64_t time_stamp_ = 0;
        // is valid uri
        bool is_valid_uri_ = true;
        // image frames
        HorizonVisionImageFrame **image_ = nullptr;
        // free source image
        void FreeImage();
        // serialize proto
        std::string Serialize() override { return "Default vio message"; };
        // multi
        mult_img_info_t *multi_info_ = nullptr;
    };
    ```

## 接口
### 构造函数
#### 定义
#include "hbipcplugin/hbipcplugin.h"

**HbipcPlugin::HbipcPlugin(std::string *path*);**

#### 参数
+ std::string *path*: 配置文件路径

#### 说明
构造HbipcPlugin类对象, 并指定该类对象使用的配置信息.

### 初始化Plugin
#### 定义
#include "hbipcplugin/hbipcplugin.h"

**int HbipcPlugin::Init() override;**

#### 参数
无

#### 返回值
+ 0: 成功
+ 非0: 失败

#### 说明
初始化Plugin.

### 反初始化Plugin
#### 定义
#include "hbipcplugin/hbipcplugin.h"

**int HbipcPlugin::Deinit() override;**

#### 参数
无

#### 返回值
+ 0: 成功
+ 非0: 失败

#### 说明
反初始化Plugin.

### 启动Plugin
#### 定义
#include "hbipcplugin/hbipcplugin.h"

**int HbipcPlugin::Start() override;**

#### 参数
无

#### 返回值
+ 0: 成功
+ 非0: 失败

#### 说明
启动Plugin.

### 停止Plugin
#### 定义
#include "hbipcplugin/hbipcplugin.h"

**int HbipcPlugin::Stop() override;**

#### 参数
无

#### 返回值
+ 0: 成功
+ 非0: 失败

#### 说明
停止Plugin.

### 获得Plugin名称
#### 定义
#include "hbipcplugin/hbipcplugin.h"

**std::string HbipcPlugin::desc() const;**

#### 参数
无

#### 返回值
Plugin描述字符串

#### 说明
获取Plugin描述字符串.

----
#### 术语表
1. **AP** : Application Processor 应用处理器  
  这里指96Boards的高通410C.
2. **CP** : Coprocessor 协处理器  
  这里指X2/J2.
3. **BIF** : AP/CP之间的通信方式  
  BIF包括物理链路和传输协议.
4. **通信域** : Communicate Domain  
  通信域为AP/CP板间的一个物理连接链路, 如SPI或者SDIO.
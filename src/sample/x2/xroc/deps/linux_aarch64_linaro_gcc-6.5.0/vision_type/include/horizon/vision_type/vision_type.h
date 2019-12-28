/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file vision_type.h
 * @brief this c header file defines the vision related data structure that are
 * used in IOT, including face, head
 * @date 2019/4/3
 */

#ifndef VISION_TYPE_VISION_TYPE_H_
#define VISION_TYPE_VISION_TYPE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "horizon/vision_type/vision_type_common.h"

/**
 * @brief 图片
 */
typedef struct HorizonVisionImage_ {
  /// 图片编码方式
  HorizonVisionPixelFormat pixel_format;
  /// 图片
  uint8_t *data;
  /// 图片大小
  uint32_t data_size;
  /// 宽度
  uint32_t width;
  /// 高度
  uint32_t height;
  /// 长度
  uint32_t stride;
  /// uv长度
  uint32_t stride_uv;
} HorizonVisionImage;
/**
 * @brief 图片帧
 */
typedef struct HorizonVisionImageFrame_ {
  /// 图片
  HorizonVisionImage image;
  /// 通道号
  uint32_t channel_id;
  /// 时间戳
  uint64_t time_stamp;
  /// 帧号
  uint64_t frame_id;
} HorizonVisionImageFrame;
/**
 * @brief 2D坐标点
 */
typedef struct HorizonVisionPoint_ {
  /// x坐标
  float x;
  /// y坐标
  float y;
  /// 置信度
  float score;
} HorizonVisionPoint;
/**
 * @brief 3D 坐标点
 */
typedef struct HorizonVisionPoint3d_ {
  /// x坐标
  float x;
  /// y坐标
  float y;
  /// z坐标
  float z;
  /// 置信度
  float score;
} HorizonVisionPoint3d;
/**
 * @brief 2D 坐标点集合
 */
typedef struct HorizonVisionPoints_ {
  /// 点数目
  size_t num;
  /// 指向2D 坐标点集合的指针
  HorizonVisionPoint *points;
  /// 置信度
  float score;
} HorizonVisionPoints;

/// 关键点
typedef HorizonVisionPoints HorizonVisionLandmarks;
/**
 * @brief 检测框
 */
typedef struct HorizonVisionBBox_ {
  /// 左上点x坐标
  float x1;
  /// 左上点y坐标
  float y1;
  /// 右下点x坐标
  float x2;
  /// 右下点y坐标
  float y2;
  /// 置信度
  float score;
  /// ID 号
  int32_t id;
} HorizonVisionBBox;
/**
 * @brief 单精度浮点数组
 */
typedef struct HorizonVisionFloatArray_ {
  /// 数目
  size_t num;
  /// 指向单精度浮点数组数组的指针
  float *values;
} HorizonVisionFloatArray;

/// 人脸特征
typedef HorizonVisionFloatArray HorizonVisionFeature;

/**
 * @brief 字符数组，可用于存储加密后的特征值
 */
typedef struct HorizonVisionCharArray_ {
  /// 数目
  size_t num;
  /// 指向字符数组的指针
  char *values;
} HorizonVisionCharArray;

/// 人脸特征
typedef HorizonVisionCharArray HorizonVisionEncryptedFeature;

/**
 * @brief 人体分割
 */
typedef struct HorizonVisionSegmentation {
  /// 数目
  size_t num;
  /// 指向float数组的指针
  float *values;
  /// 区域宽度
  int32_t width;
  /// 区域高度
  int32_t height;
} HorizonVisionSegmentation;
/**
 * @brief 人脸姿态
 */
typedef struct HorizonVisionPose3D_ {
  /// 俯仰角度
  float pitch;
  /// 左右摇头角度
  float yaw;
  /// 侧头角度
  float roll;
  /// 置信度
  float score;
} HorizonVisionPose3D;
/**
 * @brief 年龄
 */
typedef struct HorizonVisionAge_ {
  /// 年龄分类
  int32_t value;
  /// 年龄段下限
  int32_t min;
  /// 年龄段上限
  int32_t max;
  /// 置信度
  float score;
} HorizonVisionAge;
/**
 * @brief 属性类检测结果
 */
typedef struct HorizonVisionAttribute_ {
  /// 值
  int32_t value;
  /// 置信度
  float score;
} HorizonVisionAttribute;

/// 年龄
typedef HorizonVisionAttribute HorizonVisionGender;
/// 眼镜
typedef HorizonVisionAttribute HorizonVisionGlass;
/// 口罩
typedef HorizonVisionAttribute HorizonVisionBreathingMask;
/// 图片质量
typedef HorizonVisionAttribute HorizonVisionQuality;
/// 活体
typedef HorizonVisionAttribute HorizonVisionAntiSpoofing;

#ifdef __cplusplus
}
#endif
#endif  // VISION_TYPE_VISION_TYPE_H_

/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file vision_type.h
 * @brief this c header file defines the vision related data structure that are
 * used in IOT, including face, head
 * @date 2019/4/3
 */

#ifndef VISION_TYPE_VISION_TYPE_COMMON_H_
#define VISION_TYPE_VISION_TYPE_COMMON_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
/**
 * @brief 图片编码方式
 */
typedef enum HorizonVisionPixelFormat_ {
  /// 普通图片文件的二进制流，默认BGR, 需要与二进制流文件长度一起配套使用
      kHorizonVisionPixelFormatNone = 0,
  /// RGB
      kHorizonVisionPixelFormatRawRGB,
  /// RGB565
      kHorizonVisionPixelFormatRawRGB565,
  /// BGR
      kHorizonVisionPixelFormatRawBGR,
  /// 单通道灰度图
      kHorizonVisionPixelFormatRawGRAY,
  /// YUV420SP:NV21，存储格式为： YYYYYYYY VUVU
      kHorizonVisionPixelFormatRawNV21,
  /// YUV420SP:NV12 存储格式为：YYYYYYYY UVUV
      kHorizonVisionPixelFormatRawNV12,
  /// YUV420P:I420  存储格式为：YYYYYYYY UU VV
      kHorizonVisionPixelFormatRawI420,
  /// X2:SRC
      kHorizonVisionPixelFormatX2SRC,
  /// X2:PYM
      kHorizonVisionPixelFormatX2PYM,
  /// 图片标准格式 ，比如jpeg
      kHorizonVisionPixelFormatImageContainer,
  /// RGBA
      kHorizonVisionPixelFormatRawRGBA,
  /// BGRA
      kHorizonVisionPixelFormatRawBGRA,
} HorizonVisionPixelFormat;

/**
 * @brief 朝向
 */
typedef enum HorizonVisionOrientation_ {
  /// 未知
      Unknown = 0,
  /// 前
      Front = 1,
  /// 后
      Back = 2,
  /// 左
      Left = 3,
  /// 右
      Right = 4
} HorizonVisionOrientation;

#ifdef __cplusplus
}
#endif
#endif  // VISION_TYPE_VISION_TYPE_COMMON_H_

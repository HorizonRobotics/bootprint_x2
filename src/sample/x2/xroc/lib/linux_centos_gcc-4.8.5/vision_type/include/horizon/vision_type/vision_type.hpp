/**
 *  Copyright (c) 2016 Horizon Robotics. All rights reserved.
 *  @file hobot_vision_type_c.h
 *  @brief this c++ header file defines the vision related data structure that are used in
 * IOT, including face, hand
 *
 */
#ifndef VISION_TYPE_VISION_TYPE_HPP_
#define VISION_TYPE_VISION_TYPE_HPP_
#include <iostream>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>
#include <string>
#include <vector>
#include "horizon/vision_type/vision_type_common.h"
#ifdef X2
#include "hb_vio_interface.h"
#endif

namespace hobot {
namespace vision {

struct ImageFrame {
  virtual ~ImageFrame() {}
  /// 图片编码方式
  HorizonVisionPixelFormat pixel_format =
      HorizonVisionPixelFormat::kHorizonVisionPixelFormatNone;
  uint32_t channel_id = 0;
  uint64_t time_stamp = 0;
  uint64_t frame_id = 0;
  std::string type = "";
  /// 图像数据
  virtual uint8_t *Data() = 0;
  /// UV分量数据
  virtual uint8_t *DataUV() = 0;
  /// 图片大小
  virtual uint32_t DataSize() = 0;
  /// UV分量大小
  virtual uint32_t DataUVSize() = 0;
  /// 宽度
  virtual uint32_t Width() = 0;
  /// 高度
  virtual uint32_t Height() = 0;
  /// 长度
  virtual uint32_t Stride() = 0;
  /// uv长度
  virtual uint32_t StrideUV() = 0;
};

struct CVImageFrame : public ImageFrame {
  CVImageFrame() { type = "CVImageFrame"; }
  cv::Mat img;
  /// 图像数据
  uint8_t *Data() override { return img.data; }
  /// uv分量数据
  uint8_t *DataUV() override {
    return nullptr;
  }
  /// 图片大小
  uint32_t DataSize() override { return img.total() * img.channels(); }
  /// uv分量大小
  uint32_t DataUVSize() override {
    return 0;
  }
  /// 宽度
  uint32_t Width() override {
    switch (pixel_format) {
      case kHorizonVisionPixelFormatNone:
      case kHorizonVisionPixelFormatX2SRC:
      case kHorizonVisionPixelFormatX2PYM:
      case kHorizonVisionPixelFormatImageContainer: {
        return 0;
      }
      case kHorizonVisionPixelFormatRawRGB:
      case kHorizonVisionPixelFormatRawRGB565:
      case kHorizonVisionPixelFormatRawBGR:
      case kHorizonVisionPixelFormatRawGRAY:
      case kHorizonVisionPixelFormatRawNV21:
      case kHorizonVisionPixelFormatRawNV12:
      case kHorizonVisionPixelFormatRawRGBA:
      case kHorizonVisionPixelFormatRawBGRA:
      case kHorizonVisionPixelFormatRawI420: {
        return img.cols;
      }
    }
    return 0;
  }
  /// 高度
  uint32_t Height() override {
    switch (pixel_format) {
      case kHorizonVisionPixelFormatNone:
      case kHorizonVisionPixelFormatX2SRC:
      case kHorizonVisionPixelFormatX2PYM:
      case kHorizonVisionPixelFormatImageContainer: {
        return 0;
      }
      case kHorizonVisionPixelFormatRawRGB:
      case kHorizonVisionPixelFormatRawRGB565:
      case kHorizonVisionPixelFormatRawBGR:
      case kHorizonVisionPixelFormatRawRGBA:
      case kHorizonVisionPixelFormatRawBGRA:
      case kHorizonVisionPixelFormatRawGRAY: {
        return img.rows;
      }
      case kHorizonVisionPixelFormatRawNV21:
      case kHorizonVisionPixelFormatRawNV12:
      case kHorizonVisionPixelFormatRawI420: {
        return img.rows / 3 * 2;
      }
    }
    return 0;
  }
  /// 长度
  uint32_t Stride() override {
    switch (pixel_format) {
      case kHorizonVisionPixelFormatNone:
      case kHorizonVisionPixelFormatX2SRC:
      case kHorizonVisionPixelFormatX2PYM:
      case kHorizonVisionPixelFormatImageContainer: {
        return 0;
      }
      case kHorizonVisionPixelFormatRawRGB:
      case kHorizonVisionPixelFormatRawRGB565:
      case kHorizonVisionPixelFormatRawBGR:
      case kHorizonVisionPixelFormatRawGRAY:
      case kHorizonVisionPixelFormatRawNV21:
      case kHorizonVisionPixelFormatRawNV12:
      case kHorizonVisionPixelFormatRawRGBA:
      case kHorizonVisionPixelFormatRawBGRA:
      case kHorizonVisionPixelFormatRawI420: {
        return img.cols * img.channels();
      }
    }
    return 0;
  }
  /// uv长度
  uint32_t StrideUV() override {
    switch (pixel_format) {
      case kHorizonVisionPixelFormatNone:
      case kHorizonVisionPixelFormatX2SRC:
      case kHorizonVisionPixelFormatX2PYM:
      case kHorizonVisionPixelFormatImageContainer:
      case kHorizonVisionPixelFormatRawRGB:
      case kHorizonVisionPixelFormatRawRGB565:
      case kHorizonVisionPixelFormatRawBGR:
      case kHorizonVisionPixelFormatRawRGBA:
      case kHorizonVisionPixelFormatRawBGRA:
      case kHorizonVisionPixelFormatRawGRAY: {
        return 0;
      }
      case kHorizonVisionPixelFormatRawNV21:
      case kHorizonVisionPixelFormatRawNV12: {
        return img.cols;
      }
      case kHorizonVisionPixelFormatRawI420: {
        return img.cols / 2;
      }
    }
    return 0;
  }
};

#ifdef X2
struct PymImageFrame : public ImageFrame {
  PymImageFrame() {
    type = "PymImageFrame";
    pixel_format = HorizonVisionPixelFormat::kHorizonVisionPixelFormatX2PYM;
  }
  /// now only support nv12.
  img_info_t img;
  int pym_layer = 0;
  /// y分量数据
  uint8_t *Data() override {
    assert(pym_layer >= 0 && pym_layer < DOWN_SCALE_MAX);
    return img.down_scale[pym_layer].y_vaddr;
  }
  /// uv分量数据
  uint8_t *DataUV() override {
    assert(pym_layer >= 0 && pym_layer < DOWN_SCALE_MAX);
    return img.down_scale[pym_layer].c_vaddr;
  }
  /// y分量大小
  uint32_t DataSize() override {
    return Stride() * Height();
  }
  uint32_t DataUVSize() override {
    return StrideUV() * Height() / 2;
  }
  /// 宽度
  uint32_t Width() override {
    assert(pym_layer >= 0 && pym_layer < DOWN_SCALE_MAX);
    return img.down_scale[pym_layer].width;
  }
  /// 高度
  uint32_t Height() override {
    assert(pym_layer >= 0 && pym_layer < DOWN_SCALE_MAX);
    return img.down_scale[pym_layer].height;
  }
  /// 长度
  uint32_t Stride() override {
    assert(pym_layer >= 0 && pym_layer < DOWN_SCALE_MAX);
    return img.down_scale[pym_layer].step;
  }
  /// uv长度
  uint32_t StrideUV() override {
    return Stride();
  }
};

#endif

template<typename Dtype>
struct Point_ {
  inline Point_() {}
  inline Point_(Dtype x_, Dtype y_, float score_ = 0.0)
      : x(x_), y(y_), score(score_) {}

  Dtype x = 0;
  Dtype y = 0;
  float score = 0.0;
};
typedef Point_<float> Point;

template<typename Dtype>
struct Point3d_ {
  Dtype x = 0;
  Dtype y = 0;
  Dtype z = 0;
  float score = 0.0;
  inline Point3d_() {}
  inline Point3d_(Dtype x_, Dtype y_, Dtype z_, float score_ = 0.0)
      : x(x_), y(y_), z(z_), score(score_) {}
  inline explicit Point3d_(Point_<Dtype> point)
      : x(point.x), y(point.y), z(0), score(point.score) {}
  inline Point3d_(Point_<Dtype> point, Dtype z_)
      : x(point.x), y(point.y), z(z_), score(point.score) {}
};
typedef Point3d_<float> Point3d;

struct Points {
  std::vector<Point> values;
  /// 置信度
  float score = 0.0;
};

typedef Points Landmarks;

template<typename Dtype>
struct BBox_ {
  inline BBox_() {}
  inline BBox_(Dtype x1_, Dtype y1_, Dtype x2_, Dtype y2_, float score_ = 0.0f,
               int32_t id_ = -1, const std::string &category_name_ = "") {
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
    id = id_;
    score = score_;
    category_name = category_name_;
  }
  inline Dtype Width() const { return (x2 - x1); }
  inline Dtype Height() const { return (y2 - y1); }
  inline Dtype CenterX() const { return (x1 + (x2 - x1) / 2); }
  inline Dtype CenterY() const { return (y1 + (y2 - y1) / 2); }

  inline friend std::ostream &operator<<(std::ostream &out, BBox_ &bbox) {
    out << "( x1: " << bbox.x1 << " y1: " << bbox.y1 << " x2: " << bbox.x2
        << " y2: " << bbox.y2 << " score: " << bbox.score << " )";
    return out;
  }

  Dtype x1 = 0;
  Dtype y1 = 0;
  Dtype x2 = 0;
  Dtype y2 = 0;
  float score = 0.0;
  int32_t id = 0;
  std::string category_name = "";
};
typedef BBox_<float> BBox;

struct FloatArray {
  std::vector<float> values;
  /// 置信度
  float score = 0.0;
};
typedef FloatArray Feature;

struct Segmentation {
  std::vector<float> values;
  int32_t width = 0;
  int32_t height = 0;
  /// 置信度
  float score = 0.0;
};

struct Pose3D {
  /// 俯仰角度
  float pitch = 0.0;
  /// 左右摇头角度
  float yaw = 0.0;
  /// 侧头角度
  float roll = 0.0;
  /// 置信度
  float score = 0.0;
};

struct Age {
  /// 年龄分类
  int32_t value = -1;
  /// 年龄段下限
  int32_t min = -1;
  /// 年龄段上限
  int32_t max = -1;
  /// 置信度
  float score = 0.0;
};
template<typename DType>
struct Attribute {
  DType value = 0;
  /// 置信度
  float score = 0.0;
};

typedef Attribute<int32_t> Gender;
typedef Attribute<int32_t> Glass;
typedef Attribute<int32_t> Quality;
typedef Attribute<int32_t> AntiSpoofing;
typedef Attribute<int32_t> BreathingMask;

template<typename DType>
struct SnapshotInfo {
  SnapshotInfo()
      : track_id(-1),
        select_value(0),
        origin_image_frame(nullptr),
        snap(nullptr) {}

  /**
  * @brief 抓拍图坐标转换接口
  *
  * @param in [in] 相对于原始帧的一组坐标
  * @return Points 相对于抓拍图的一组坐标
  */
  virtual Points PointsToSnap(const Points &in) { return Points(); }

  int32_t track_id;
  /// 优选参考值
  float select_value;
  /// 抓拍发生时原始帧数据
  std::shared_ptr<ImageFrame> origin_image_frame;
  /// 抓拍图数据
  std::shared_ptr<ImageFrame> snap;
  /// 用户数据数组
  std::vector<DType> userdata;
};

}  // namespace vision
}  // namespace hobot

#endif  // VISION_TYPE_VISION_TYPE_HPP_

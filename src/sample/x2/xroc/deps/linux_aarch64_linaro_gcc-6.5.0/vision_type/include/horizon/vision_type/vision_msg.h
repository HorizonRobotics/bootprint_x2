/**
* Copyright (c) 2019 Horizon Robotics. All rights reserved.
* @file vision_msg.h
* @brief this c header file defines the vision related smart and snapshot message that are used in
 * IOT, including face, head
* @date 2019/4/3
*/

#ifndef VISION_TYPE_VISION_MSG_H_
#define VISION_TYPE_VISION_MSG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "horizon/vision_type/vision_type.h"
/**
 * @brief 人脸智能信息
 */
typedef struct HorizonVisionFaceSmartData_ {
  /// 跟踪ID
  uint32_t track_id;
  /// 人脸框
  HorizonVisionBBox face_rect;
  /// 人头框
  HorizonVisionBBox head_rect;
  /// 人脸姿态
  HorizonVisionPose3D pose3d;
  /// 人脸关键点
  HorizonVisionLandmarks *landmarks;
  /// 年龄
  HorizonVisionAge age;
  /// 性别
  HorizonVisionGender gender;
  /// 眼镜
  HorizonVisionGlass glass;
  /// 口罩
  HorizonVisionBreathingMask mask;
  /// 活体信息
  HorizonVisionAntiSpoofing anti_spoofing;
  /// 图片质量
  HorizonVisionQuality quality;
  /// 人脸特征
  HorizonVisionFeature *feature;
  HorizonVisionEncryptedFeature *encrypted_feature;
} HorizonVisionFaceSmartData;
/**
 * @brief 人体智能信息
 */
typedef struct HorizonVisionBodySmartData_ {
  /// 跟踪ID
  uint32_t track_id;
  /// 人体框
  HorizonVisionBBox body_rect;
  /// 人体分割
  HorizonVisionSegmentation *segmentation;
  /// 骨骼点
  HorizonVisionLandmarks *skeleton;
} HorizonVisionBodySmartData;
/**
 * @brief 单个跟踪目标智能信息，包含人脸智能信息、人体智能信息等
 */
typedef struct HorizonVisionSmartData_ {
  /// 跟踪ID
  uint32_t track_id;
  /// 人脸智能信息
  HorizonVisionFaceSmartData *face;
  /// 人体智能信息
  HorizonVisionBodySmartData *body;
} HorizonVisionSmartData;
/**
 * @brief 智能帧
 */
typedef struct HorizonVisionSmartFrame_ {
  /// 视频帧个数
  uint32_t image_num;
  /// 当前智能帧对应视频帧列表
  HorizonVisionImageFrame **image_frame;
  /// smart_data_list中的智能信息个数
  uint32_t smart_data_list_num;
  /// 指向智能信息列表
  HorizonVisionSmartData *smart_data_list;
  /// 时间戳
  uint64_t time_stamp;
  /// 帧ID
  uint64_t frame_id;
} HorizonVisionSmartFrame;
/**
 * @brief 单个抓拍图
 */
typedef struct HorizonVisionSnapshot_ {
  /// 时间戳
  uint64_t time_stamp;
  /// 抓拍图的智能信息，如：跟踪ID、人脸特征
  HorizonVisionSmartData *smart_data;
  /// 抓拍图
  HorizonVisionImage *croped_image;
} HorizonVisionSnapshot;

/**
 * @brief 单个抓拍目标，内含多个抓拍图
 */
typedef struct HorizonVisionSnapshotTarget_ {
  /// 跟踪ID
  uint32_t track_id;
  /// 抓拍图数量
  uint32_t snapshots_num;
  /// 指向抓拍图列表
  HorizonVisionSnapshot *snapshots;
} HorizonVisionSnapshotTarget;

/**
 * @brief 抓拍帧
 */
typedef struct HorizonVisionSnapshotFrame_ {
  /// 抓拍目标数目
  uint32_t targets_num;
  /// 指向抓拍目标列表
  HorizonVisionSnapshotTarget *targets;
} HorizonVisionSnapshotFrame;

#ifdef __cplusplus
}
#endif
#endif  // VISION_TYPE_VISION_MSG_H_

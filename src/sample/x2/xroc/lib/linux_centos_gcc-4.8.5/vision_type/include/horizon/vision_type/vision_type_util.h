/*
 * Copyright (c) 2019 Horizon Robotics
 * @author sunshuhuan
 * @date 2019.03.12
 */

#ifndef VISION_TYPE_VISION_TYPE_UTIL_H_
#define VISION_TYPE_VISION_TYPE_UTIL_H_

#include <stdint.h>
#include <stdbool.h>

#include "vision_msg.h"
#include "vision_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 复制字符串
 *
 * @param str [in] 源字符串
 * @return char* 失败则返回NULL
 * @note 返回的字符串资源需手动调用free释放
 */
char *HorizonVisionStrDup(const char *str);

/**
 * @brief 构造landmarks结构
 *
 * @param pnew_lmks [out] 保存新landmarks指针的地址
 * @return int 0表示成功，<0则为错误码
 * @points资源需单独申请
 */
int HorizonVisionAllocLandmarks(HorizonVisionLandmarks **pnew_lmks);

/**
 * @brief 复制landmarks结构，复制数据到另一个结构体，相当于赋值操作
 *
 * @param lmks [in] 源landmarks
 * @param new_lmks [out] 新的landmarks
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyLandmarks(HorizonVisionLandmarks *lmks,
                               HorizonVisionLandmarks *new_lmks);

/**
 * @brief 复制landmarks结构，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param lmks [in] 源landmarks
 * @param pnew_lmks [out] 用于保存新的landmarks的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupLandmarks(HorizonVisionLandmarks *lmks,
                              HorizonVisionLandmarks **pnew_lmks);

/**
 * @brief 清空landmarks内部指针数据，但保留结构体本身
 *
 * @param lmks [in] 要清空的结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果points不为空，则会一并释放该资源
 */
int HorizonVisionCleanLandmarks(HorizonVisionLandmarks *lmks);

/**
 * @brief 释放landmarks
 *
 * @param lmks [in] 要释放的指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果points不为空，则会一并释放该资源
 */
int HorizonVisionFreeLandmarks(HorizonVisionLandmarks *lmks);

/**
 * @brief 申请HorizonVisionCharArray
 *
 * @param pnew_array [out] 用来存储新申请结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note HorizonVisionCharArray.values资源需单独申请
 */
int HorizonVisionAllocCharArray(HorizonVisionCharArray **pnew_array);

/**
 * @brief 复制HorizonVisionCharArray内容，复制数据到另一个结构体，相当于赋值操作
 *
 * @param array [in] 源HorizonVisionCharArray
 * @param new_array [out] 新的HorizonVisionCharArray
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyCharArray(HorizonVisionCharArray *array,
                               HorizonVisionCharArray *new_array);

/**
 * @brief 复制HorizonVisionCharArray，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param array [in] 源HorizonVisionCharArray
 * @param pnew_array [out] 新的HorizonVisionCharArray
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupCharArray(HorizonVisionCharArray *array,
                              HorizonVisionCharArray **pnew_array);

/**
 * @brief 清理HorizonVisionCharArray内部指针数据，但保留结构体本身
 *
 * @param array [in] 要清理的结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果HorizonVisionCharArray.values不为空，则会一并释放该资源
 */
int HorizonVisionCleanCharArray(HorizonVisionCharArray *array);

/**
 * @brief 释放CharArray
 *
 * @param farray [in] 要释放的指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果HorizonVisionCharArray.values不为空，则会一并释放该资源
 */
int HorizonVisionFreeCharArray(HorizonVisionCharArray *farray);

/**
 * @brief 申请FloatArray
 *
 * @param pnew_farray [out] 用来存储新申请结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note values资源需单独申请
 */
int HorizonVisionAllocFloatArray(HorizonVisionFloatArray **pnew_farray);

/**
 * @brief 复制FloatArray内容，复制数据到另一个结构体，相当于赋值操作
 *
 * @param farray [in] 源Float Array
 * @param new_farray [out] 新的Float Array
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyFloatArray(HorizonVisionFloatArray *farray,
                                HorizonVisionFloatArray *new_farray);

/**
 * @brief 复制FloatArray，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param farray [in] 源FloatArrray
 * @param pnew_farray [out] 新的FloatArray
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupFloatArray(HorizonVisionFloatArray *farray,
                               HorizonVisionFloatArray **pnew_farray);

/**
 * @brief 清理FloatArray内部指针数据，但保留结构体本身
 *
 * @param farray [in] 要清理的结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果values不为空，则会一并释放该资源
 */
int HorizonVisionCleanFloatArray(HorizonVisionFloatArray *farray);

/**
 * @brief 释放FloatArray
 *
 * @param farray [in] 要释放的指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果values不为空，则会一并释放该资源
 */
int HorizonVisionFreeFloatArray(HorizonVisionFloatArray *farray);

/**
 * @brief 申请分割结构体
 *
 * @param pnew_farray [out] 用来保存新构造的结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note values资源需单独申请
 */
int HorizonVisionAllocSegmentation(HorizonVisionSegmentation **pnew_farray);

/**
 * @brief 复制分割结构体，复制数据到另一个结构体，相当于赋值操作
 *
 * @param farray [in] 源结构体指针
 * @param new_farray [out] 新结构体的指针
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopySegmentation(HorizonVisionSegmentation *farray,
                                  HorizonVisionSegmentation *new_farray);

/**
 * @brief 复制分割结构体，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param farray [in] 源结构体指针
 * @param pnew_farray [out] 用来存储新构建的结构体指针
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupSegmentation(HorizonVisionSegmentation *farray,
                                 HorizonVisionSegmentation **pnew_farray);

/**
 * @brief 清空分割结构体内部指针数据，但保留结构体本身
 *
 * @param farray [in] 要清空的数据结构体指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果values不为空，则会一并释放该资源
 */
int HorizonVisionCleanSegmentation(HorizonVisionSegmentation *farray);

/**
 * @brief 释放分割结构体
 *
 * @param farray [in] 结构体指针
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionFreeSegmentation(HorizonVisionSegmentation *farray);

/**
 * @brief 申请smart data
 *
 * @param psmart [out] 用来存储新申请的smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note landmarks 及 feature 需单独申请
 */
int HorizonVisionAllocFaceSmartData(HorizonVisionFaceSmartData **psmart);

/**
 * @brief 复制 smart data，复制数据到另一个结构体，相当于赋值操作
 *
 * @param smart [in] smart data 指针存储地址
 * @param new_smart [out] 新的smart data的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyFaceSmartData(HorizonVisionFaceSmartData *smart_data,
                                   HorizonVisionFaceSmartData *new_smart);

/**
 * @brief 复制 smart data
 *
 * @param smart [in] smart data 指针存储地址
 * @param pnew_smart [out] 用于存储新的smart data的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupFaceSmartData(HorizonVisionFaceSmartData *smart_data,
                                  HorizonVisionFaceSmartData **pnew_smart);

/**
 * @brief 释放smart data内部指针数据，但保留结构体本身
 *
 * @param smart [in] smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果landmarks 或 feature不为空，则会一并释放该资源
 */
int HorizonVisionCleanFaceSmartData(HorizonVisionFaceSmartData *smart);

/**
 * @brief 释放smart data
 *
 * @param smart [in] smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果landmarks 或 feature不为空，则会一并释放该资源
 */
int HorizonVisionFreeFaceSmartData(HorizonVisionFaceSmartData *smart);

/**
 * @brief 申请smart data
 *
 * @param psmart [out] 用来存储新申请的smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note skeleton 及 segmentation 资源需单独申请
 */
int HorizonVisionAllocBodySmartData(HorizonVisionBodySmartData **psmart);

/**
 * @brief 复制 smart data，复制数据到另一个结构体，相当于赋值操作
 *
 * @param smart [in] smart data 指针存储地址
 * @param new_smart [out] 新的smart data的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyBodySmartData(HorizonVisionBodySmartData *smart_data,
                                   HorizonVisionBodySmartData *new_smart);

/**
 * @brief 复制 smart data，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param smart [in] smart data 指针存储地址
 * @param pnew_smart [out] 用于存储新的smart data的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupBodySmartData(HorizonVisionBodySmartData *smart_data,
                                  HorizonVisionBodySmartData **pnew_smart);

/**
 * @brief 清空smart data内部数据，但保留smart data本身
 *
 * @param smart [in] smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果skeleton 或 segmentation不为空，则会一并释放该资源
 */
int HorizonVisionCleanBodySmartData(HorizonVisionBodySmartData *smart);

/**
 * @brief 释放smart data
 *
 * @param smart [in] smart data指针
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionFreeBodySmartData(HorizonVisionBodySmartData *smart);

/**
 * @brief 申请smart data
 *
 * @param psmart [out] 用来存储新申请的smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note face 及 body 资源需单独申请
 */
int HorizonVisionAllocSmartData(HorizonVisionSmartData **psmart, int num);

/**
 * @brief 复制 smart data，复制数据到另一个结构体，相当于赋值操作
 *
 * @param smart [in] smart data 指针存储地址
 * @param new_smart [out] 目标smart data
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopySmartData(HorizonVisionSmartData *smart_data,
                               HorizonVisionSmartData *new_smart);

/**
 * @brief 复制 smart data，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param smart [in] smart data 指针存储地址
 * @param pnew_smart [out] 用于存储新的smart data的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupSmartData(HorizonVisionSmartData *smart_data,
                              HorizonVisionSmartData **pnew_smart);

/**
 * @brief 清空smart data内部数据，但保留smart data本身
 *
 * @param smart [in] smart data指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果face 或 body不为空，则会一并释放该资源
 */
int HorizonVisionCleanSmartData(HorizonVisionSmartData *smart);

/**
 * @brief 释放smart data
 *
 * @param smart [in] smart data指针
 * @param num [in] smart data数量
 * @return int 0表示成功，<0则为错误码
 * @note 如果face 或 body不为空，则会一并释放该资源
 */
int HorizonVisionFreeSmartData(HorizonVisionSmartData *smart, int num);

/**
 * @brief 创建抓拍数组
 *
 * @param psnaps [in] 指向抓拍数组地址的指针
 * @param num  抓拍图数组大小
 * @return int 0表示成功，<0则为错误码
 * @note 每个抓拍的croped_image和smart_data需单独申请
 */
int HorizonVisionAllocSnapshot(HorizonVisionSnapshot **psnaps, int num);

/**
 * @brief 复制 snapshot，复制数据到另一个结构体，相当于赋值操作
 *
 * @param snapshot [in] snapshot指针存储地址
 * @param new_snapshot [out] 目标snapshot
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopySnapshot(HorizonVisionSnapshot *snapshot,
                              HorizonVisionSnapshot *new_snapshot);

/**
 * @brief 复制 smart snapshot，构造新的结构体并复制内容过来，然后返回新的结构体指针
 * **此处会复制抓拍图像帧数据
 *
 * @param snapshot [in] snapshot指针存储地址
 * @param pnew_snapshot [out] new_snapshot的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupSnapshot(HorizonVisionSnapshot *snapshot,
                             HorizonVisionSnapshot **pnew_snapshot);

/**
 * @brief 清空单个snapshot 内部数据，但保留snapshot本身
 * **此处会复制抓拍图像帧数据
 *
 * @param snap [in] 抓拍指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果croped_image或smart_data不为空，则会一并释放该资源
 */
int HorizonVisionCleanSnapshot(HorizonVisionSnapshot *snap);

/**
 * @brief 释放抓拍数组
 *
 * @param snap [in] 抓拍指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果croped_image或smart_data不为空，则会一并释放该资源
 */
int HorizonVisionFreeSnapshot(HorizonVisionSnapshot *snap, int num);

/**
 * @brief 申请创建图像
 * @param pimg [out] 用来保存图像指针的地址
 * @return int 0表示成功，<0则为错误码
 * @note data字段需单独申请
 */
int HorizonVisionAllocImage(HorizonVisionImage **pimg);

/**
 * @brief 复制 image，复制数据到另一个结构体，相当于赋值操作
 *
 * @param image [in] image
 * @param copy_image_data [in] 是否拷贝图像数据
 * @param new_image [out] 目标image
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyImage(HorizonVisionImage *image,
                           bool copy_image_data,
                           HorizonVisionImage *new_image);

/**
 * @brief 复制 image，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param image [in] image指针存储地址
 * @param dup_image_data [in] 是否拷贝图像数据
 * @param pnew_image [out] image的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupImage(HorizonVisionImage *image,
                          bool dup_image_data,
                          HorizonVisionImage **pnew_image);

/**
 * @brief 清空图像中的数据
 *
 * @param img [in] 指向要清空的图像
 * @return int 0表示成功，<0则为错误码
 * @note 如果data字段不为空，则会一并释放该资源
 */
int HorizonVisionCleanImage(HorizonVisionImage *img);

/**
 * @brief 释放图像指针
 *
 * @param img [in] 图像指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果data字段不为空，则会一并释放该资源
 */
int HorizonVisionFreeImage(HorizonVisionImage *img);

/**
 * @brief 申请创建图像帧
 *
 * @param pimage [in] image frame 指针存储地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionAllocImageFrame(HorizonVisionImageFrame **pimage);

/**
 * @brief 复制 image frame，复制数据到另一个结构体，相当于赋值操作
 *
 * @param image_frame [in] image_frame
 * @param copy_image_data [in] 是否拷贝图像数据
 * @param new_image_frame [out] 目标image_frame
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopyImageFrame(HorizonVisionImageFrame *image_frame,
                                bool copy_image_data,
                                HorizonVisionImageFrame *new_image_frame);

/**
 * @brief 复制 image frame，构造新的结构体并复制内容过来，然后返回新的结构体指针
 *
 * @param image_frame [in] image_frame指针存储地址
 * @param dup_image_data [in] 是否拷贝图像数据
 * @param pnew_image_frame [out] image_frame的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupImageFrame(HorizonVisionImageFrame *image_frame,
                               bool dup_image_data,
                               HorizonVisionImageFrame **pnew_image_frame);
/**
 * @brief 释放图像帧资源
 *
 * @param image [in] image frame 指针
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionFreeImageFrame(HorizonVisionImageFrame *image);

/**
 * @brief 申请创建智能帧
 *
 * @param psmart [in] 智能帧指针存储地址
 * @return int 0表示成功，<0则为错误码
 * @note smart_data_list 资源需单独申请
 */
int HorizonVisionAllocSmartFrame(HorizonVisionSmartFrame **psmart);

/**
 * @brief 复制 smart frame，复制数据到另一个结构体，相当于赋值操作
 * **此处不会复制原始图像帧数据
 *
 * @param smart_frame [in] smart_frame指针存储地址
 * @param new_smart_frame [out] 目标smart_frame
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopySmartFrame(HorizonVisionSmartFrame *smart_frame,
                                HorizonVisionSmartFrame *new_smart_frame);

/**
 * @brief 复制 smart frame，构造新的结构体并复制内容过来，然后返回新的结构体指针
 * **此处不会复制原始图像帧数据
 *
 * @param smart_frame [in] smart_frame指针存储地址
 * @param pnew_smart_frame [out] smart_frame的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupSmartFrame(HorizonVisionSmartFrame *smart_frame,
                               HorizonVisionSmartFrame **pnew_smart_frame);

/**
 * @brief 释放智能帧资源
 *
 * @param smart [in] 指向智能帧资源的指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果smart_data_list不为空，会一并释放该资源
 */
int HorizonVisionFreeSmartFrame(HorizonVisionSmartFrame *smart);

/**
 * @brief 申请创建抓拍目标列表
 *
 * @param ptargets [in] 指向HorizonVisionSnapshotTarget列表地址的指针
 * @param num [in] 抓拍目标数量
 * @return int 0表示成功，<0则为错误码
 * @note 每个抓拍目标的snapshots资源需单独申请
 */
int HorizonVisionAllocSnapshotTarget(HorizonVisionSnapshotTarget **ptargets,
                                     int num);

/**
 * @brief 复制 snapshot target，复制数据到另一个结构体，相当于赋值操作
 * **此处会复制抓拍图像帧数据
 *
 * @param target [in] snapshot target 指针存储地址
 * @param new_target [out] 目标snapshot target
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopySnapshotTarget(HorizonVisionSnapshotTarget *target,
                                    HorizonVisionSnapshotTarget *new_target);

/**
 * @brief 复制 snapshot target，构造新的结构体并复制内容过来，然后返回新的结构体指针
 * **此处会复制抓拍图像帧数据
 *
 * @param target [in] snapshot target 指针存储地址
 * @param pnew_target [out] 用于存储新的snapshot target的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupSnapshotTarget(HorizonVisionSnapshotTarget *target,
                                   HorizonVisionSnapshotTarget **pnew_target);

/**
 * @brief 清空单个抓拍目标，但不释放HorizonVisionSnapshotTarget本身
 *
 * @param targets [in] HorizonVisionSnapshotTarget地址
 * @return int 0表示成功，<0则为错误码
 * @note 如果该目标snapshots不为空，则会一并释放抓拍资源
 */
int HorizonVisionCleanSnapshotTarget(HorizonVisionSnapshotTarget *targets);

/**
 * @brief 释放抓拍目标列表
 *
 * @param targets [in] HorizonVisionSnapshotTarget列表地址
 * @param num [in] 抓拍目标数量
 * @return int 0表示成功，<0则为错误码
 * @note 对于每个抓拍目标，如果snapshots不为空，则会一并释放抓拍资源
 */
int HorizonVisionFreeSnapshotTarget(HorizonVisionSnapshotTarget *targets,
                                    int num);

/**
 * @brief 申请创建抓拍帧资源
 *
 * @param psnapshot [in] snapshot list 指针存储地址
 * @return int 0表示成功，<0则为错误码
 * @note targets资源需单独申请
 */
int HorizonVisionAllocSnapshotFrame(HorizonVisionSnapshotFrame **psnapshots);

/**
 * @brief 复制 snapshot frame，复制数据到另一个结构体，相当于赋值操作
 * **此处会复制抓拍图像帧数据
 *
 * @param snapshots [in] snapshot frame 指针存储地址
 * @param new_snapshots [out] 目标snapshot frame
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionCopySnapshotFrame(HorizonVisionSnapshotFrame *snapshots,
                                   HorizonVisionSnapshotFrame *new_snapshots);

/**
 * @brief 复制 snapshot frame，构造新的结构体并复制内容过来，然后返回新的结构体指针
 * **此处会复制抓拍图像帧数据
 *
 * @param snapshots [in] snapshot frame 指针存储地址
 * @param pnew_snapshots [out] 用于存储新的snapshot frame的地址
 * @return int 0表示成功，<0则为错误码
 */
int HorizonVisionDupSnapshotFrame(HorizonVisionSnapshotFrame *snapshots,
                                  HorizonVisionSnapshotFrame **pnew_snapshots);

/**
 * @brief 清空抓拍帧内容，但不释放抓拍帧本身
 *
 * @param snapshots [in] snapshot list 指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果targets不为空，会一并释放该资源
 */
int HorizonVisionCleanSnapshotFrame(HorizonVisionSnapshotFrame *snapshots);

/**
 * @brief 释放 snapshot 列表
 *
 * @param snapshots [in] snapshot list 指针
 * @return int 0表示成功，<0则为错误码
 * @note 如果targets不为空，会一并释放该资源
 */
int HorizonVisionFreeSnapshotFrame(HorizonVisionSnapshotFrame *snapshots);

#ifdef __cplusplus
}
#endif

#endif  // VISION_TYPE_VISION_TYPE_UTIL_H_

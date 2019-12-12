#!/bin/sh

echo "------------------------------------------"
echo "run onto terminal........................."
echo "------------------------------------------"

HOBOT_PATH="./hobot"
#HOBOT_PATH="/lib/modules/4.14.0-qcomlt-arm64/kernel/drivers/hobot"

QCOM_PATH="/lib/modules/4.14.0-qcomlt-arm64/kernel/drivers/media/platform/qcom"

SENSOR_KO_PATH=${HOBOT_PATH}/sensor
SENSOR_KO="ar023x.ko"

CAMSS_KO_PATH=${QCOM_PATH}/camss
CAMSS_KO="qcom-camss.ko"

echo "rmmod ${CAMSS_KO} ${SENSOR_KO}"
sudo rmmod ${CAMSS_KO} ${SENSOR_KO}

echo "insmod ${SENSOR_KO_PATH}/${SENSOR_KO}"
sudo insmod ${SENSOR_KO_PATH}/${SENSOR_KO}

echo "insmod ${CAMSS_KO_PATH}/${CAMSS_KO}"
sudo insmod ${CAMSS_KO_PATH}/${CAMSS_KO}

#1 Setup media link
sudo media-ctl -d /dev/media0 -l '"msm_csiphy0":1->"msm_csid0":0[1],"msm_csid0":1->"msm_ispif0":0[1],"msm_ispif0":1->"msm_vfe0_pix":0[1]'

#2 Format configuration for NV12/NV21 output
sudo media-ctl -d /dev/media0 -V '"ar023x":0[fmt:UYVY8_2X8/1920x1080 field:none],"msm_csiphy0":0[fmt:UYVY8_2X8/1920x1080 field:none],"msm_csid0":0[fmt:UYVY8_2X8/1920x1080 field:none],"msm_ispif0":0[fmt:UYVY8_2X8/1920x1080 field:none],"msm_vfe0_pix":0[fmt:UYVY8_2X8/1920x1080 field:none],"msm_vfe0_pix":1[fmt:UYVY8_1_5X8/1920x1080 field:none]'
  
# pipeline stream
#gst-launch-1.0 -e v4l2src device=/dev/video3 ! video/x-raw,format=NV12,width=1920,height=1080,framerate=25/1 ! glimagesink




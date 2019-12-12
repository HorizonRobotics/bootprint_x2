#!/bin/sh

HOBOT_PATH="./hobot"
#HOBOT_PATH="/lib/modules/4.14.0-qcomlt-arm64/kernel/drivers/hobot"

BISPI_KO_PATH=${HOBOT_PATH}/bif_spi
BIFSPI_KO="bifspi.ko"
BIFSPI_PARAM="spidev_dts=1 rst_gpio=25 bifrst_gpio=35 spics_gpio=18 spispeed=24000000"

BIFBASE_KO_PATH=${HOBOT_PATH}/bif_base
BIFBASE_KO="bifbase.ko"
BIFBASE_PARAM="bifbase_irq_pin=24 bifbase_tri_pin=115"

BIFETH_KO_PATH=${HOBOT_PATH}/bif_ethernet
BIFETH_KO="bifeth.ko"
BIFETH_PARAM=""

BIFLITE_KO_PATH=${HOBOT_PATH}/bif_lite
BIFLITE_CORE_KO="biflite_core.ko"
BIFLITE_CORE_PARAM=""
BIFLITE_SPI_KO="biflite_spi.ko"
BIFLITE_SPI_PARAM=""

echo "rmmod ${BIFLITE_SPI_KO} ${BIFLITE_CORE_KO} ${BIFETH_KO} ${BIFBASE_KO} ${BIFSPI_KO}"
rmmod ${BIFLITE_SPI_KO}
rmmod ${BIFLITE_CORE_KO}
rmmod ${BIFETH_KO}
rmmod ${BIFBASE_KO}
rmmod ${BIFSPI_KO}

#bifspi
echo "insmod ${BISPI_KO_PATH}/${BIFSPI_KO} ${BIFSPI_PARAM}"
insmod ${BISPI_KO_PATH}/${BIFSPI_KO} ${BIFSPI_PARAM}

#bifbase
echo "insmod ${BIFBASE_KO_PATH}/${BIFBASE_KO} ${BIFBASE_PARAM}"
insmod ${BIFBASE_KO_PATH}/${BIFBASE_KO} ${BIFBASE_PARAM}

#bifeth
echo "insmod ${BIFETH_PATH}/${BIFETH_KO} ${BIFETH_PARAM}"
insmod ${BIFETH_KO_PATH}/${BIFETH_KO} ${BIFETH_PARAM}

#biflite
echo "insmod ${BIFLITE_KO_PATH}/${BIFLITE_CORE_KO} ${BIFLITE_CORE_PARAM}"
insmod ${BIFLITE_KO_PATH}/${BIFLITE_CORE_KO} ${BIFLITE_CORE_PARAM}
echo "insmod ${BIFLITE_KO_PATH}/${BIFLITE_SPI_KO} ${BIFLITE_SPI_PARAM}"
insmod ${BIFLITE_KO_PATH}/${BIFLITE_SPI_KO} ${BIFLITE_SPI_PARAM}




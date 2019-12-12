#!/bin/sh
BASEPATH=$(cd `dirname $0`; pwd)
INSTALL_PATH=/home/linaro/ap_base
SETUP_PATH=/etc/init.d/ap_base_setup.sh
echo $BASEPATH
echo $HOME
sudo mkdir $INSTALL_PATH
sudo cp -rf $BASEPATH/* $INSTALL_PATH/
sudo chmod 777 -R $INSTALL_PATH
echo install 410c_ap_base successful on $INSTALL_PATH
sudo echo "#!/bin/sh
echo "1" > /home/linaro/base_log.txt
cd ${INSTALL_PATH}
sudo ./410c_bif_depoly.sh
sudo ./single_pix_stream.sh" > ${SETUP_PATH}
sudo chmod +x ${SETUP_PATH}
sudo insserv ${SETUP_PATH}
#for i in $(seq 0 6)
#do
#sudo rm /etc/rc${i}.d/S03apbasesetup.sh
#sudo ln -s ${SETUP_PATH} /etc/rc${i}.d/S03apbasesetup.sh
#done


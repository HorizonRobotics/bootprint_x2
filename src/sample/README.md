# Horizon Robotics X2 96board Sandard Sample

## x2
CP(Co-Processor) Deploy package

### Full Ability:
Output face detection and tracking id, head detection, body detection, body skeleton.

### Get cp deploy package
> cd src/sample/x2/xpp_cp/smartplugin/deploy/  
> tar czf xppcp_smart.tgz xppcp_smart

### Transfer the CP Side deployment package to any directory on CP and decompress. (Usually we transfer the package to directory: /userdata/)
> tar zxf xppcp_smart.tgz
> cd x2/xppcp_smart  
> sh xpp_start.sh

## dragonboard410c
AP(Application Processor) Deploy package, and partial open-source.

### Default Ability:
Render face/head/body bbox, face trackid, body skeleton.

### Get Ap packages
> cd src/sample/dragonboard410c/  
> tar czf ap_base.tgz ap_base  
> tar czf XppClientMini.tgz XppClientMini  

### How to deploy
Step1: load related drivers.  
> tar zxf ap_base.tgz  
> cd dragonboard410c/ap_base  
> sudo ./410c_bif_depoly.sh  
> sudo ./single_pix_stream.sh 

Step2: start rendering engine.
> tar zxf XppClientMini.tgz  
> cd dragonboard410c/XppClientMini  
> sudo ./start.sh

### How to build
If you modify some codes, you should rebuild the project.
> make



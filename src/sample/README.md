# Horizon Robotics X2 96board Sandard Sample

## x2
CP(Co-Processor) Deploy package

### Full Ability:
Output face detection and tracking id, head detection, body detection, body skeleton.

### How to deploy
> cd x2/xppcp_smart  
> sh xpp_start.sh

## dragonboard410c
AP(Application Processor) Deploy package, and partial open-source.

### Default Ability:
Render face/head/body bbox, face trackid, body skeleton.

### How to deploy
Step1: load related drivers.  
> cd dragonboard410c/ap_base  
> sudo ./410c_bif_depoly.sh  
> sudo ./single_pix_stream.sh 

Step2: start rendering engine.
> cd dragonboard410c/XppClientMini  
> sudo ./start.sh

### How to build
If you modify some codes, you should rebuild the project.
> make



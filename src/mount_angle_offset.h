#ifndef MOUNT_ANGLE_OFFSET_H
#define MOUNT_ANGLE_OFFSET_H

#include <Arduino.h>
#include "config.h"


enum MountAngleOffsetField {
  MOUNT_ANGLE_OFFSET_FIELD_CROSSHAIR = 0, 
  MOUNT_ANGLE_OFFSET_FIELD_SAVE = 1,      
  MOUNT_ANGLE_OFFSET_FIELD_COUNT = 2      
};


struct MountAngleOffsetState {
  int mountAngleOffsetX;          
  int mountAngleOffsetY;          
  int selectedField;              
  bool adjustingCrosshair;        
};


extern MountAngleOffsetState mountAngleOffsetState;


void initMountAngleOffset();
void drawMountAngleOffsetScreen();
void handleMountAngleOffsetInput();

#endif
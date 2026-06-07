#ifndef CROSSHAIR_SETTINGS_H
#define CROSSHAIR_SETTINGS_H

#include <Arduino.h>
#include "config.h"


enum CrosshairType {
  CROSSHAIR_SIMPLE = 0,      
  CROSSHAIR_TSHAPE,             
  CROSSHAIR_DUAL_CIRCLE,     
  CROSSHAIR_RING_DOT         
};


struct CrosshairSettingsState {
  CrosshairType selectedType;    
  int selectedField;             
};


extern CrosshairSettingsState crosshairState;


void initCrosshairSettings();
void drawCrosshairSettingsScreen();
void handleCrosshairSettingsInput();
void drawCrosshair(int centerX, int centerY, CrosshairType type);

#endif 
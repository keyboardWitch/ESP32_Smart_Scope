#ifndef ARC_POINTS_SETTINGS_H
#define ARC_POINTS_SETTINGS_H

#include <Arduino.h>
#include "config.h"


enum ArcPointsMode {
  ARC_POINTS_SOLID = 0,    
  ARC_POINTS_BLINKING,     
  ARC_POINTS_OFF           
};


enum ArcPointsField {
  ARC_POINTS_FIELD_DISPLAY_MODE = 0,  
  ARC_POINTS_FIELD_TILT_MODE = 1,     
  ARC_POINTS_FIELD_SAVE = 2,          
  ARC_POINTS_FIELD_COUNT = 3          
};


struct ArcPointsSettingsState {
  ArcPointsMode selectedMode;  
  bool tiltModeEnabled;        
  int selectedField;           
};


extern ArcPointsSettingsState arcPointsState;


void initArcPointsSettings();
void drawArcPointsSettingsScreen();
void handleArcPointsSettingsInput();

#endif 
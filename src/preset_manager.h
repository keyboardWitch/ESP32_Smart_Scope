#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include <Arduino.h>
#include "config.h"


enum PresetManagerField {
  PRESET_FIELD_PRESET1 = 0,   
  PRESET_FIELD_PRESET2 = 1,   
  PRESET_FIELD_PRESET3 = 2,   
  PRESET_FIELD_SAVE = 3,      
  PRESET_FIELD_COUNT = 4      
};


struct PresetManagerState {
  int selectedPreset;         
  int selectedField;          
};


extern PresetManagerState presetManagerState;


void initPresetManager();
void drawPresetManagerScreen();
void handlePresetManagerInput();

#endif 
#ifndef WIND_COMPENSATION_H
#define WIND_COMPENSATION_H

#include <Arduino.h>
#include "config.h"
#include "menu_system.h"


enum WindField {
  WIND_FIELD_SPEED = 0,
  WIND_FIELD_DIRECTION = 1,
  WIND_FIELD_SAVE = 2,        
  WIND_FIELD_COUNT = 3        
};


struct WindCompensationState {
  float speed;                
  float direction;            
  int selectedField;          
  bool isEditing;             
};


extern WindCompensationState windState;


void initWindCompensation();
void drawWindCompensationScreen();
void handleWindCompensationInput();
bool isWindCompensationMode();

#endif 
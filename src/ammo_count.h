#ifndef AMMO_COUNT_H
#define AMMO_COUNT_H

#include <Arduino.h>
#include "config.h"


enum AmmoCountField {
  AMMO_FIELD_CAPACITY = 0,      
  AMMO_FIELD_SHOT_DETECTION = 1, 
  AMMO_FIELD_SAVE = 2,          
  AMMO_FIELD_COUNT = 3          
};


enum ShotDetectionState {
  SHOT_IDLE = 0,        
  SHOT_DETECTING = 1,   
  SHOT_DETECTED = 2     
};


struct AmmoCountState {
  int capacity;             
  int selectedField;        
  ShotDetectionState shotState; 
  unsigned long detectionStartTime; 
  int consecutiveDetections; 
  float vibrationThreshold; 
};


extern AmmoCountState ammoCountState;


void initAmmoCount();
void drawAmmoCountScreen();
void handleAmmoCountInput();
bool detectShotVibration(); 

#endif 
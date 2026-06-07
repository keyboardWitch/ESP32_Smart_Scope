#ifndef BULLET_VELOCITY_H
#define BULLET_VELOCITY_H

#include <Arduino.h>
#include "config.h"
#include "menu_system.h"


enum BulletVelocityField {
  BULLET_VELOCITY_FIELD_VALUE = 0,
  BULLET_VELOCITY_FIELD_SAVE = 1,
  BULLET_VELOCITY_FIELD_COUNT = 2
};


struct BulletVelocityState {
  int velocity;               
  int selectedField;          
};


extern BulletVelocityState bulletVelocityState;


void initBulletVelocity();
void drawBulletVelocityScreen();
void handleBulletVelocityInput();

#endif 
#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include "config.h"


extern Adafruit_SSD1306 display;
extern Adafruit_MPU6050 mpu;


extern bool mpu6050Working;


void initSystem();
bool initDisplay();
bool initSensors();
void initModules();
void loadSystemState();

#endif 
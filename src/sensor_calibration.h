#ifndef SENSOR_CALIBRATION_H
#define SENSOR_CALIBRATION_H

#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"  


enum SensorField {
  SENSOR_FIELD_GYROSCOPE = 0,  
  SENSOR_FIELD_SAVE = 1,       
  SENSOR_FIELD_COUNT = 2       
};


struct SensorCalibrationState {
  int selectedField;           
  bool isCalibrating;          
  int calibrationProgress;     
};


extern SensorCalibrationState sensorCalibState;


extern MPU6050CalibrationData cachedMPUCalibrationData;


void initSensorCalibration();
void drawSensorCalibrationScreen();
void handleSensorCalibrationInput();

#endif 
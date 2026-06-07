#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "ballistic_calibration.h"


#define PRESET_COUNT 3


#define EEPROM_ACTIVE_PRESET_ADDR 0


#define EEPROM_PRESET_START_ADDR 1


#define EEPROM_MPU6050_START_ADDR (EEPROM_PRESET_START_ADDR + PRESET_COUNT * sizeof(PresetData))


struct PresetData {
  float windSpeed;           
  float windDirection;       
  float bulletVelocity;      
  int crosshairType;         
  int arcPointState;         
  int ammoCount;             
  int rangeMode;             
  BallisticCalibrationData ballisticCalib; 
  
  
  bool shotDetectionEnabled; 
  float shotVibrationThreshold; 
  unsigned long shotDetectionTime; 
  
  
  bool tiltModeEnabled;      
  
  int  mountAngleOffset;      

  
  PresetData() {
    windSpeed = 0.0f;
    windDirection = 0.0f;
    bulletVelocity = 50.0f;  
    crosshairType = 0;       
    arcPointState = 0;       
    ammoCount = 0;           
    rangeMode = 0;           
    
    ballisticCalib.modelParams.physicsDrag.dragCoefficientK = 0.1f;
    ballisticCalib.pitch5m = 0.0f;
    ballisticCalib.pitch10m = 0.0f;
    ballisticCalib.pitch15m = 0.0f;
    ballisticCalib.mountAngleOffset = MOUNT_ANGLE_OFFSET; 
    ballisticCalib.isCalibrated = false;
    ballisticCalib.calibratedPoints = 0;
    ballisticCalib.modelType = MODEL_TYPE_OFF; 
    ballisticCalib.checksum = 0;
    
    
    shotDetectionEnabled = false;
    shotVibrationThreshold = 2.0f; 
    shotDetectionTime = 0;
    
    
    tiltModeEnabled = false;   
  }
};


struct MPU6050CalibrationData {
  bool isCalibrated;         
  float gyroOffsetX;         
  float gyroOffsetY;         
  float gyroOffsetZ;         
  float accelOffsetX;        
  float accelOffsetY;        
  float accelOffsetZ;        
  float pitchAngleOffset;    
  
  
  MPU6050CalibrationData() {
    isCalibrated = false;
    gyroOffsetX = gyroOffsetY = gyroOffsetZ = 0.0f;
    accelOffsetX = accelOffsetY = accelOffsetZ = 0.0f;
    pitchAngleOffset = 0.0f;
  }
};


void initEEPROM();
int loadActivePreset();
void saveActivePreset(int presetIndex);
void loadPresetData(int presetIndex, PresetData& data);
void savePresetData(int presetIndex, const PresetData& data);
void loadMPU6050Calibration(MPU6050CalibrationData& data);
void saveMPU6050Calibration(const MPU6050CalibrationData& data);


void saveCurrentStateToPreset(int presetIndex);

void loadPresetToCurrentState(int presetIndex);


extern int g_activePresetIndex;

#endif 

#include "eeprom_manager.h"
#include "wind_compensation.h"
#include "bullet_velocity.h" 
#include "crosshair_settings.h"
#include "arc_points_settings.h"
#include "ammo_count.h"
#include "range_finder.h"
#include "preset_manager.h"


int g_activePresetIndex = 0;

void initEEPROM() {
  
  EEPROM.begin(1024); 
  
  
  g_activePresetIndex = loadActivePreset();
}

int loadActivePreset() {
  int activePreset = EEPROM.read(EEPROM_ACTIVE_PRESET_ADDR);
  
  
  if (activePreset >= 0 && activePreset < PRESET_COUNT) {
    return activePreset;
  } else {
    
    
    return 0;
  }
}

void saveActivePreset(int presetIndex) {
  
  if (presetIndex >= 0 && presetIndex < PRESET_COUNT) {
    EEPROM.write(EEPROM_ACTIVE_PRESET_ADDR, presetIndex);
    
    
    if (!EEPROM.commit()) {
      Serial.println("ERROR: Failed to commit active preset to EEPROM!");
    } else {
      Serial.println("Active preset committed to EEPROM successfully");
    }
  }
}

void loadPresetData(int presetIndex, PresetData& data) {
  if (presetIndex < 0 || presetIndex >= PRESET_COUNT) {
    
    data = PresetData();
    return;
  }
  
  
  int addr = EEPROM_PRESET_START_ADDR + presetIndex * sizeof(PresetData);
  
  
  byte* pData = (byte*)&data;
  for (int i = 0; i < sizeof(PresetData); i++) {
    pData[i] = EEPROM.read(addr + i);
  }
  
  
  
  if (isnan(data.windSpeed) || isnan(data.windDirection)) {
    data.windSpeed = 0.0f;
    data.windDirection = 0.0f;
  }
  
  
  if (data.windSpeed < 0.0f || data.windSpeed > 20.0f) {
    data.windSpeed = 0.0f;
  }
  if (data.windDirection < 0.0f || data.windDirection > 360.0f) {
    data.windDirection = 0.0f;
  }
  
  
  if (isnan(data.bulletVelocity)) {
    data.bulletVelocity = 50.0f;
  }
  
  
  if (data.bulletVelocity < 1.0f || data.bulletVelocity > 500.0f) {
    data.bulletVelocity = 50.0f;
  }
  
  
  if (isnan(data.mountAngleOffset)) {
    data.mountAngleOffset = MOUNT_ANGLE_OFFSET;
  }
  if (data.mountAngleOffset < -90 || data.mountAngleOffset > 90) {
    data.mountAngleOffset = MOUNT_ANGLE_OFFSET;
  }
  
  
  if (data.rangeMode < 0 || data.rangeMode > 1) {
    data.rangeMode = 0; 
  }
}

void savePresetData(int presetIndex, const PresetData& data) {
  if (presetIndex < 0 || presetIndex >= PRESET_COUNT) {
    return; 
  }
  
  
  int addr = EEPROM_PRESET_START_ADDR + presetIndex * sizeof(PresetData);
  
  
  const byte* pData = (const byte*)&data;
  for (int i = 0; i < sizeof(PresetData); i++) {
    EEPROM.write(addr + i, pData[i]);
  }
  
  
  if (!EEPROM.commit()) {
    Serial.println("ERROR: Failed to commit EEPROM data!");
  } else {
    Serial.println("EEPROM data committed successfully");
  }
}

void loadMPU6050Calibration(MPU6050CalibrationData& data) {
  int addr = EEPROM_MPU6050_START_ADDR;
  
  
  byte* pData = (byte*)&data;
  for (int i = 0; i < sizeof(MPU6050CalibrationData); i++) {
    pData[i] = EEPROM.read(addr + i);
  }
  
  
  
  bool isValid = data.isCalibrated;
  
  
  if (isnan(data.gyroOffsetX) || isnan(data.gyroOffsetY) || isnan(data.gyroOffsetZ) ||
      isnan(data.accelOffsetX) || isnan(data.accelOffsetY) || isnan(data.accelOffsetZ)) {
    isValid = false;
  }
  
  
  if (!isValid) {
    data = MPU6050CalibrationData();
  }
}

void saveMPU6050Calibration(const MPU6050CalibrationData& data) {
  int addr = EEPROM_MPU6050_START_ADDR;
  
  
  const byte* pData = (const byte*)&data;
  for (int i = 0; i < sizeof(MPU6050CalibrationData); i++) {
    EEPROM.write(addr + i, pData[i]);
  }
  
  
  if (!EEPROM.commit()) {
    Serial.println("ERROR: Failed to commit MPU6050 calibration data!");
  } else {
    Serial.println("MPU6050 calibration data committed successfully");
  }
}


void saveCurrentStateToPreset(int presetIndex) {
  if (presetIndex < 0 || presetIndex >= PRESET_COUNT) {
    return;
  }
  
  PresetData preset;
  preset.windSpeed = windState.speed;
  preset.windDirection = windState.direction;
  preset.bulletVelocity = bulletVelocityState.velocity;
  preset.crosshairType = static_cast<int>(crosshairState.selectedType);
  preset.arcPointState = static_cast<int>(arcPointsState.selectedMode);
  preset.ammoCount = ammoCountState.capacity;
  preset.rangeMode = static_cast<int>(rangeFinderState.selectedMode);

  savePresetData(presetIndex, preset);
}


void loadPresetToCurrentState(int presetIndex) {
  if (presetIndex < 0 || presetIndex >= PRESET_COUNT) {
    return;
  }
  
  PresetData preset;
  loadPresetData(presetIndex, preset);
  
  
  windState.speed = preset.windSpeed;
  windState.direction = preset.windDirection;
  bulletVelocityState.velocity = static_cast<int>(preset.bulletVelocity);
  
  
  if (preset.crosshairType < 0 || preset.crosshairType > 3) {
    preset.crosshairType = 0; 
  }
  crosshairState.selectedType = static_cast<CrosshairType>(preset.crosshairType);
  
  
  if (preset.arcPointState < 0 || preset.arcPointState > 2) {
    preset.arcPointState = 0; 
  }
  arcPointsState.selectedMode = static_cast<ArcPointsMode>(preset.arcPointState);
  
  ammoCountState.capacity = preset.ammoCount;
  
  
  if (preset.rangeMode < 0 || preset.rangeMode > 1) {
    preset.rangeMode = 0; 
  }
  
  
  if (isnan(preset.mountAngleOffset)) {
    preset.mountAngleOffset = MOUNT_ANGLE_OFFSET;
  }
  
  
  if (preset.mountAngleOffset < -90 || preset.mountAngleOffset > 90) {
    preset.mountAngleOffset = 0;
  }
}
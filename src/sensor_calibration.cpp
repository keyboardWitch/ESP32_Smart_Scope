#include "sensor_calibration.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "config.h"


const int CALIBRATION_SAMPLES = MPU6050_CALIBRATION_SAMPLES;  
const int CALIBRATION_DELAY = MPU6050_CALIBRATION_DELAY;     


extern Adafruit_MPU6050 mpu;


SensorCalibrationState sensorCalibState = {SENSOR_FIELD_GYROSCOPE, false, 0};

void initSensorCalibration() {
  sensorCalibState.selectedField = SENSOR_FIELD_GYROSCOPE;
  sensorCalibState.isCalibrating = false;
  sensorCalibState.calibrationProgress = 0;
}

void drawSensorCalibrationScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_SENSOR_CALIBRATION) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRoundRect(0, 0, SCREEN_WIDTH, 16, 4, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 4);
  display->print("Sensor Calib");
  
  
  int startY = 30;
  int lineHeight = 20;
  int y = startY;
  
  
  bool isGyroSelected = (sensorCalibState.selectedField == SENSOR_FIELD_GYROSCOPE);
  if (isGyroSelected) {
    display->fillRect(5, y - 2, SCREEN_WIDTH - 10, 12, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  
  display->setCursor(10, y);
  display->print("Gyroscope");
  
  
  if (cachedMPUCalibrationData.isCalibrated && !sensorCalibState.isCalibrating) {
    
    if (isGyroSelected) {
      display->setTextColor(BLACK);  
    } else {
      display->setTextColor(WHITE);  
    }
    display->setCursor(SCREEN_WIDTH - 30, y);
    display->print("[OK]");
  }
  
  
  if (sensorCalibState.isCalibrating) {
    y += lineHeight;
    
    
    display->setTextColor(WHITE);
    display->setCursor(10, y);
    
    
    int progressWidth = (SCREEN_WIDTH - 30) * sensorCalibState.calibrationProgress / 100;
    if (progressWidth > 0) {
      display->fillRect(28, y - 2, progressWidth, 8, WHITE);
    }
    
    
    char percentStr[5];
    itoa(sensorCalibState.calibrationProgress, percentStr, 10);
    display->setCursor(SCREEN_WIDTH - 20, y);
    display->print(percentStr);
    display->print("%");
  }
  
  
  int hintY = 85;
  display->setTextColor(WHITE);
  display->setCursor(15, hintY);
  if (sensorCalibState.isCalibrating) {
    display->print("Collecting...");
  } else if (cachedMPUCalibrationData.isCalibrated) {
    display->print("Calibrated!");
  } else {
    display->print("Keep device still!");
  }
  
  
  int separatorY = SCREEN_HEIGHT - 20;
  display->drawLine(5, separatorY, SCREEN_WIDTH - 5, separatorY, WHITE);
  
  
  int saveY = SCREEN_HEIGHT - 12;
  if (sensorCalibState.selectedField == SENSOR_FIELD_SAVE) {
    display->fillRect(8, saveY, SCREEN_WIDTH - 16, 10, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  display->setCursor(10, saveY + 3);
  display->print("Save & Exit");
  
  display->display();
}


void performGyroCalibration() {
  Serial.println("Starting real Gyroscope and Accelerometer Calibration...");
  
  
  float gyroSumX = 0.0f, gyroSumY = 0.0f, gyroSumZ = 0.0f;
  float accelSumX = 0.0f, accelSumY = 0.0f, accelSumZ = 0.0f;
  
  
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    
    gyroSumX += g.gyro.x;
    gyroSumY += g.gyro.y;
    gyroSumZ += g.gyro.z;
    
    
    accelSumX += a.acceleration.x;
    accelSumY += a.acceleration.y;
    accelSumZ += a.acceleration.z;
    
    
    sensorCalibState.calibrationProgress = (i + 1) * 100 / CALIBRATION_SAMPLES;
    drawSensorCalibrationScreen();
    
    delay(CALIBRATION_DELAY);
  }
  
  
  float gyroOffsetX = gyroSumX / CALIBRATION_SAMPLES;
  float gyroOffsetY = gyroSumY / CALIBRATION_SAMPLES;
  float gyroOffsetZ = gyroSumZ / CALIBRATION_SAMPLES;
  
  float accelOffsetX = accelSumX / CALIBRATION_SAMPLES;
  float accelOffsetY = accelSumY / CALIBRATION_SAMPLES;
  float accelOffsetZ = accelSumZ / CALIBRATION_SAMPLES;
  
  
  
  
  
  
  
  
  
  
  
  
  
  accelOffsetZ -= 9.81f;
  
  
  
  sensors_event_t currentA, currentG, currentTemp;
  mpu.getEvent(&currentA, &currentG, &currentTemp);
  
  
  if (!isnan(currentA.acceleration.y) && !isnan(currentA.acceleration.z) &&
      !(fabs(currentA.acceleration.y) < 0.01f && fabs(currentA.acceleration.z) < 0.01f)) {
    
    
    float currentAccelY = -currentA.acceleration.y;
    float currentAccelZ = -currentA.acceleration.z;
    if (fabs(currentAccelZ) < 0.01f) {
      currentAccelZ = (currentAccelZ >= 0) ? 0.01f : -0.01f;
    }
    float currentPitch = atan2(currentAccelY, currentAccelZ);
    
    
    float pitchAngleOffset = currentPitch;
    
    
    cachedMPUCalibrationData.isCalibrated = true;
    cachedMPUCalibrationData.gyroOffsetX = gyroOffsetX;
    cachedMPUCalibrationData.gyroOffsetY = gyroOffsetY;
    cachedMPUCalibrationData.gyroOffsetZ = gyroOffsetZ;
    cachedMPUCalibrationData.accelOffsetX = accelOffsetX;
    cachedMPUCalibrationData.accelOffsetY = accelOffsetY;
    cachedMPUCalibrationData.accelOffsetZ = accelOffsetZ;
    cachedMPUCalibrationData.pitchAngleOffset = pitchAngleOffset;
  } else {
    
    cachedMPUCalibrationData.isCalibrated = true;
    cachedMPUCalibrationData.gyroOffsetX = gyroOffsetX;
    cachedMPUCalibrationData.gyroOffsetY = gyroOffsetY;
    cachedMPUCalibrationData.gyroOffsetZ = gyroOffsetZ;
    cachedMPUCalibrationData.accelOffsetX = accelOffsetX;
    cachedMPUCalibrationData.accelOffsetY = accelOffsetY;
    cachedMPUCalibrationData.accelOffsetZ = accelOffsetZ;
    cachedMPUCalibrationData.pitchAngleOffset = 0.0f;
  }
  
  
  saveMPU6050Calibration(cachedMPUCalibrationData);
  
  Serial.println("Gyroscope and Accelerometer Calibration Completed!");
  Serial.print("Gyro Offsets: X="); Serial.print(gyroOffsetX);
  Serial.print(", Y="); Serial.print(gyroOffsetY);
  Serial.print(", Z="); Serial.println(gyroOffsetZ);
  Serial.print("Accel Offsets: X="); Serial.print(accelOffsetX);
  Serial.print(", Y="); Serial.print(accelOffsetY);
  Serial.print(", Z="); Serial.println(accelOffsetZ);
  Serial.print("Pitch Angle Offset (rad): "); Serial.println(cachedMPUCalibrationData.pitchAngleOffset);
  
  sensorCalibState.isCalibrating = false;
  sensorCalibState.calibrationProgress = 100;
}

void handleSensorCalibrationInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (sensorCalibState.selectedField > 0) {
      sensorCalibState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (sensorCalibState.selectedField < SENSOR_FIELD_COUNT - 1) {
      sensorCalibState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (sensorCalibState.selectedField == SENSOR_FIELD_GYROSCOPE) {
      
      sensorCalibState.isCalibrating = true;
      sensorCalibState.calibrationProgress = 0;
      Serial.println("Starting Gyroscope Calibration...");
      needRedraw = true;
      
      
      performGyroCalibration();
      
    } else if (sensorCalibState.selectedField == SENSOR_FIELD_SAVE) {
      
      Serial.println("Sensor Calibration Settings Saved");
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  if (needRedraw) {
    drawSensorCalibrationScreen();
  }
}
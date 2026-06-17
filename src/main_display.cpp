#include "main_display.h"
#include <Adafruit_SSD1306.h>
#include "menu_system.h"
#include "crosshair_settings.h"
#include "wind_compensation.h"
#include "range_finder.h"
#include "config.h"
#include "ballistic_calc.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include "arc_points_settings.h"  
#include "ballistic_calibration.h"  
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "system_init.h"  
#include <math.h>         
#include <stdlib.h>       


float getMedianPitch(float newPitch);
float getDeadzonePitch(float rawPitch);
float getMedianRoll(float newRoll);
float getDeadzoneRoll(float rawRoll);


extern Adafruit_MPU6050 mpu;
extern bool mpu6050Working;


extern MPU6050CalibrationData cachedMPUCalibrationData;


PresetData currentActivePreset;


extern HardwareSerial tofSerial;


unsigned long lastMeasurementTime = 0;


unsigned long lastBlinkTime = 0;
bool dropPointVisible = true;


float getIMUPitchForBallistics();


static float mainDisplayFilteredIMUPitch = 0.0f;


static float filteredDistance = 0.0f;
static float smoothDropPointX = 64.0f;  
static float smoothDropPointY = 32.0f;  



static unsigned long lastShotDetectionTime = 0;
static int consecutiveShotDetections = 0;
static unsigned long shotDetectionStartTime = 0;


float cachedIMUPitchForBallistics = 0.0f;


float cachedIMURollForBallistics = 0.0f;


static int currentStability = 0.0f;


#define STABILITY_WINDOW_SIZE 8
static float gyroXBuffer[STABILITY_WINDOW_SIZE] = {0.0f};
static float gyroYBuffer[STABILITY_WINDOW_SIZE] = {0.0f};
static float gyroZBuffer[STABILITY_WINDOW_SIZE] = {0.0f};
static float accelXBuffer[STABILITY_WINDOW_SIZE] = {0.0f};
static float accelYBuffer[STABILITY_WINDOW_SIZE] = {0.0f};
static float accelZBuffer[STABILITY_WINDOW_SIZE] = {0.0f};
static int stabilityBufferIndex = 0;
static unsigned long lastStabilityUpdateTime = 0;



int getCrosshairCenterX() {
    return SCREEN_WIDTH / 2 + currentActivePreset.mountAngleOffsetX;
}

int getCrosshairCenterY() {
    return SCREEN_HEIGHT / 2 + currentActivePreset.mountAngleOffset;
}


float calculateStandardDeviation(float* buffer, int size) {
    if (size <= 1) return 0.0f;
    
    
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += buffer[i];
    }
    float mean = sum / size;
    
    
    float varianceSum = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = buffer[i] - mean;
        varianceSum += diff * diff;
    }
    float variance = varianceSum / (size - 1); 
    
    return sqrt(variance);
}


void updateStabilityCalculation() {
    static unsigned long lastUpdate = 0;
    unsigned long currentTime = millis();
    
    
    if (currentTime - lastUpdate >= STABILITY_UPDATE_INTERVAL) {
        if (mpu6050Working) {
            sensors_event_t a, g, temp;
            mpu.getEvent(&a, &g, &temp);
            
            
            if (!isnan(a.acceleration.x) && !isnan(a.acceleration.y) && !isnan(a.acceleration.z) &&
                !isnan(g.gyro.x) && !isnan(g.gyro.y) && !isnan(g.gyro.z)) {
                
                
                int idx = stabilityBufferIndex % STABILITY_WINDOW_SIZE;
                gyroXBuffer[idx] = g.gyro.x;
                gyroYBuffer[idx] = g.gyro.y;
                gyroZBuffer[idx] = g.gyro.z;
                accelXBuffer[idx] = a.acceleration.x;
                accelYBuffer[idx] = a.acceleration.y;
                accelZBuffer[idx] = a.acceleration.z;
                stabilityBufferIndex++;
                
                
                if (stabilityBufferIndex >= STABILITY_WINDOW_SIZE) {
                    
                    float gyroStdX = calculateStandardDeviation(gyroXBuffer, STABILITY_WINDOW_SIZE);
                    float gyroStdY = calculateStandardDeviation(gyroYBuffer, STABILITY_WINDOW_SIZE);
                    float gyroStdZ = calculateStandardDeviation(gyroZBuffer, STABILITY_WINDOW_SIZE);
                    float accelStdX = calculateStandardDeviation(accelXBuffer, STABILITY_WINDOW_SIZE);
                    float accelStdY = calculateStandardDeviation(accelYBuffer, STABILITY_WINDOW_SIZE);
                    float accelStdZ = calculateStandardDeviation(accelZBuffer, STABILITY_WINDOW_SIZE);
                    
                    
                    
                    float maxGyroStd = fmax(fmax(gyroStdX, gyroStdY), gyroStdZ);
                    float maxAccelStd = fmax(fmax(accelStdX, accelStdY), accelStdZ);
                    
                    
                    currentStability = 0.7f * maxGyroStd + 0.3f * maxAccelStd;
                }
            }
        }
        lastUpdate = currentTime;
    }
}


float calculateMainDisplayAccelMagnitude(sensors_event_t& accelEvent) {
  float ax = accelEvent.acceleration.x;
  float ay = accelEvent.acceleration.y;
  float az = accelEvent.acceleration.z;
  return sqrt(ax * ax + ay * ay + az * az);
}


void updateIMUCache() {
  static unsigned long lastUpdateTime = 0;
  
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= MAIN_DISPLAY_UPDATE_INTERVAL) {
    if (mpu6050Working) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      
      
      if (!isnan(a.acceleration.x) && !isnan(a.acceleration.y) && !isnan(a.acceleration.z) &&
          !(fabs(a.acceleration.x) < 0.01f && fabs(a.acceleration.y) < 0.01f && fabs(a.acceleration.z) < 0.01f)) {
        

        float accelY = a.acceleration.y;  
        float accelZ = a.acceleration.z;   
        if (fabs(accelZ) < 0.01f) {
          accelZ = (accelZ >= 0) ? 0.01f : -0.01f;
        }
        float rawPitch = atan2(accelY, accelZ);
        
        
        if (cachedMPUCalibrationData.isCalibrated && 
            !(fabs(a.acceleration.y) < 0.01f && fabs(a.acceleration.z) < 0.01f)) {
          
          float correctedAccelY = a.acceleration.y - cachedMPUCalibrationData.accelOffsetY;
          float correctedAccelZ = a.acceleration.z - cachedMPUCalibrationData.accelOffsetZ;
          
          if (!isnan(correctedAccelY) && !isnan(correctedAccelZ) && 
              fabs(correctedAccelZ) >= 0.01f) {
            rawPitch = atan2(correctedAccelY, correctedAccelZ);
          }
        }
        
        
        
        if (cachedMPUCalibrationData.isCalibrated) {
          rawPitch -= cachedMPUCalibrationData.pitchAngleOffset;
        }
        
        
        
        float medPitch = getMedianPitch(rawPitch);
        
        float finalPitch = getDeadzonePitch(medPitch);
        
        cachedIMUPitchForBallistics = finalPitch;
        
        
        
        
        float accelX = a.acceleration.x;
        float accelMagnitudeYZ = sqrt(accelY * accelY + accelZ * accelZ);
        if (accelMagnitudeYZ < 0.01f) {
          accelMagnitudeYZ = 0.01f;  
        }
        float rawRoll = atan2(-accelX, accelMagnitudeYZ);
        
        
        if (cachedMPUCalibrationData.isCalibrated && 
            !(fabs(a.acceleration.x) < 0.01f && fabs(a.acceleration.y) < 0.01f && fabs(a.acceleration.z) < 0.01f)) {
          float correctedAccelX = a.acceleration.x - cachedMPUCalibrationData.accelOffsetX;
          float correctedAccelY = a.acceleration.y - cachedMPUCalibrationData.accelOffsetY;
          float correctedAccelZ = a.acceleration.z - cachedMPUCalibrationData.accelOffsetZ;
          float correctedMagnitudeYZ = sqrt(correctedAccelY * correctedAccelY + correctedAccelZ * correctedAccelZ);
          
          if (!isnan(correctedAccelX) && !isnan(correctedMagnitudeYZ) && 
              correctedMagnitudeYZ >= 0.01f) {
            rawRoll = atan2(-correctedAccelX, correctedMagnitudeYZ);
          }
        }
        
        
        
        
        
        
        float medRoll = getMedianRoll(rawRoll);
        float finalRoll = getDeadzoneRoll(medRoll);
        cachedIMURollForBallistics = finalRoll;

      }
    }
    lastUpdateTime = currentTime;
  }
}


bool detectShotAndDecrementAmmo() {
  
  unsigned long currentTime = millis();
  if (currentTime - lastShotDetectionTime < SHOT_DETECTION_COOLDOWN) {
    return false;
  }
  
  
  if (!mpu6050Working) {
    return false;
  }
  
  
  if (!currentActivePreset.shotDetectionEnabled) {
    return false; 
  }
  
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  
  if (isnan(a.acceleration.x) || isnan(a.acceleration.y) || isnan(a.acceleration.z)) {
    return false;
  }
  
  
  if (fabs(a.acceleration.x) < 0.01f && fabs(a.acceleration.y) < 0.01f && fabs(a.acceleration.z) < 0.01f) {
    return false;
  }
  
  
  float accelMagnitude = calculateMainDisplayAccelMagnitude(a);
  float vibrationLevel = fabs(accelMagnitude - 9.81f); 
  
  
  bool isVibrationDetected = vibrationLevel > currentActivePreset.shotVibrationThreshold;
  
  if (isVibrationDetected) {
    
    if (consecutiveShotDetections == 0) {
      shotDetectionStartTime = currentTime;
    }
    
    consecutiveShotDetections++;
    
    
    if (currentTime - shotDetectionStartTime > SHOT_DETECTION_WINDOW) {
      
      consecutiveShotDetections = 0;
      return false;
    }
    
    
    if (consecutiveShotDetections >= CONSECUTIVE_DETECTIONS_REQUIRED) {
      
      if (currentAmmoCount > 0) {
        currentAmmoCount--;
        lastShotDetectionTime = currentTime;
        consecutiveShotDetections = 0;
        
        return true;
      } else {
        
        consecutiveShotDetections = 0;
        return false;
      }
    }
  } else {
    
    if (currentTime - shotDetectionStartTime <= SHOT_DETECTION_WINDOW) {
      
      consecutiveShotDetections = 0;
    } else {
      
      consecutiveShotDetections = 0;
    }
  }
  
  return false;
}



void drawDropPoint(Adafruit_SSD1306* display, float distance) {
  if (distance <= 0.0f) {
    return; 
  }
  
  // 计算准星中心位置，考虑mount angle offset的X和Y方向
  int crosshairCenterX = SCREEN_WIDTH / 2 + currentActivePreset.mountAngleOffsetX;
  int crosshairCenterY = SCREEN_HEIGHT / 2 + currentActivePreset.mountAngleOffset;
  
  // 使用新的像素偏移计算方式
  int pixelOffsetY = calculateDropPointPixelOffset((int)distance, currentActivePreset.ballisticCalib);
  
  // X方向：先使用弹道修正（风偏等），然后根据需要添加IMU滚转补偿
  float pixelOffsetX = 0.0f;
  
  // 如果有风偏，仍然使用原有的风偏计算
  if (windState.speed > 0.5f && currentActivePreset.bulletVelocity > 0.0f) {
    BallisticResult result = calculateBallisticCorrection(
      distance, 
      currentActivePreset.bulletVelocity,
      windState.speed,
      windState.direction,
      currentActivePreset.ballisticCalib
    );
    if (result.isValid) {
      pixelOffsetX = result.correctionYaw * DROP_POINT_PX_PER_RADIAN;
    }
  }
  
  // 添加IMU滚转补偿（如果启用）
  if (currentActivePreset.tiltModeEnabled) {
    float currentIMURoll = cachedIMURollForBallistics;
    if (isnan(currentIMURoll)) {
      currentIMURoll = 0.0f;
    }
    float tiltCorrectionAngle = currentIMURoll * TILT_CORRECTION_SCALE;  
    float tiltPixelOffsetX = tiltCorrectionAngle * DROP_POINT_PX_PER_RADIAN;
    pixelOffsetX += tiltPixelOffsetX;
  }
  
  if (isnan(pixelOffsetX)) {
    pixelOffsetX = 0.0f;
  }
  
  // 应用像素偏移限制
  float maxYOffset = SCREEN_HEIGHT / 2.0f - 2;
  float maxXOffset = SCREEN_WIDTH / 2.0f - 2;
  
  if (pixelOffsetY < -maxYOffset) pixelOffsetY = -maxYOffset;
  if (pixelOffsetY > maxYOffset) pixelOffsetY = maxYOffset;
  if (pixelOffsetX < -maxXOffset) pixelOffsetX = -maxXOffset;
  if (pixelOffsetX > maxXOffset) pixelOffsetX = maxXOffset;
  
  // 计算最终的弹着点坐标
  float rawDropPointX = crosshairCenterX + pixelOffsetX;
  float rawDropPointY = crosshairCenterY + pixelOffsetY;  
  
  int dropPointX = (int)round(rawDropPointX);
  int dropPointY = (int)round(rawDropPointY);
  
  // 边界检查
  if (dropPointX < 0) dropPointX = 0;
  if (dropPointX >= SCREEN_WIDTH) dropPointX = SCREEN_WIDTH - 1;
  if (dropPointY < 0) dropPointY = 0;
  if (dropPointY >= SCREEN_HEIGHT) dropPointY = SCREEN_HEIGHT - 1;
  
  // 显示弹着点（根据闪烁模式）
  ArcPointsMode arcMode = static_cast<ArcPointsMode>(currentActivePreset.arcPointState);
  bool shouldBlink = (arcMode == ARC_POINTS_BLINKING);
  
  unsigned long currentTime = millis();
  if (shouldBlink) {
    if (currentTime - lastBlinkTime >= DROP_POINT_BLINK_INTERVAL) {
      dropPointVisible = !dropPointVisible;
      lastBlinkTime = currentTime;
    }
  } else {
    dropPointVisible = true;
    lastBlinkTime = currentTime;
  }
  
  if (dropPointVisible) {
    int size = 3;
    int startX = dropPointX - size / 2;
    int startY = dropPointY - size / 2;
    
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (startX + size > SCREEN_WIDTH) startX = SCREEN_WIDTH - size;
    if (startY + size > SCREEN_HEIGHT) startY = SCREEN_HEIGHT - size;
    
    display->fillRect(startX, startY, size, size, WHITE);
  }
  
  // 显示超出范围指示器
  if (pixelOffsetY < -8.0f) {
    int arrowX = SCREEN_WIDTH - 10;
    int arrowY = 10;
    display->drawLine(arrowX, arrowY + 3, arrowX, arrowY - 3, WHITE);
    display->drawLine(arrowX, arrowY - 3, arrowX - 2, arrowY - 1, WHITE);
    display->drawLine(arrowX, arrowY - 3, arrowX + 2, arrowY - 1, WHITE);
  } else if (pixelOffsetY > 8.0f) {
    int arrowX = SCREEN_WIDTH - 10;
    int arrowY = 10;
    display->drawLine(arrowX, arrowY - 3, arrowX, arrowY + 3, WHITE);
    display->drawLine(arrowX, arrowY + 3, arrowX - 2, arrowY + 1, WHITE);
    display->drawLine(arrowX, arrowY + 3, arrowX + 2, arrowY + 1, WHITE);
  }
}



int currentAmmoCount = 0;
int totalAmmoCapacity = 30;


void drawAmmoInfo(Adafruit_SSD1306* display) {
  display->setTextSize(1);
  display->setTextColor(WHITE);
  
  char ammoStr[10];
  sprintf(ammoStr, "%d/%d", currentAmmoCount, totalAmmoCapacity);
  
  
  int x = 2; 
  int y = SCREEN_HEIGHT - 10; 
  
  display->setCursor(x, y);
  display->print(ammoStr);
}


void drawStabilityWaveform(Adafruit_SSD1306* display) {
  
  
  
  
  float stabilityValue = currentStability;
  float maxExpectedStability = 0.1f; 
  
  
  if (stabilityValue > maxExpectedStability) {
    stabilityValue = maxExpectedStability;
  }
  
  
  float amplitude = (stabilityValue / maxExpectedStability) * 8.0f;
  
  
  int waveformWidth = 30; 
  int waveformHeight = 12; 
  int startX = SCREEN_WIDTH - waveformWidth - 2; 
  int centerY = SCREEN_HEIGHT - 8; 
  
  
  display->fillRect(startX - 2, centerY - waveformHeight/2 - 2, 
                   waveformWidth + 4, waveformHeight + 4, BLACK);
  
  
  display->drawLine(startX, centerY, startX + waveformWidth, centerY, WHITE);
  
  
  unsigned long currentTime = millis();
  float timeFactor = currentTime * 0.01f; 
  
  for (int i = 0; i < waveformWidth; i++) {
    float x = i;
    
    float baseWave = sin(timeFactor + x * 0.3f) * amplitude;
    float randomPerturbation = ((float)rand() / RAND_MAX - 0.5f) * amplitude * 0.3f;
    float y = baseWave + randomPerturbation;
    
    int pixelY = centerY - (int)y; 
    
    
    if (pixelY >= 0 && pixelY < SCREEN_HEIGHT) {
      display->drawPixel(startX + i, pixelY, WHITE);
    }
  }
  
  
  char statusChar = ' ';
  if (stabilityValue < 0.02f) {
    statusChar = 'S'; 
  } else if (stabilityValue < 0.05f) {
    statusChar = 'M'; 
  } else {
    statusChar = 'U'; 
  }
  
  display->setTextSize(1);
  display->setTextColor(WHITE);
  display->setCursor(startX - 8, centerY - 4);
  display->print(statusChar);
}


void initMainDisplay() {
  
  loadPresetData(presetManagerState.selectedPreset, currentActivePreset);
  
  
  if (currentActivePreset.tiltModeEnabled != true && currentActivePreset.tiltModeEnabled != false) {
    currentActivePreset.tiltModeEnabled = false; 
  }
  
  
  totalAmmoCapacity = currentActivePreset.ammoCount;
  if (totalAmmoCapacity <= 0) {
    totalAmmoCapacity = 30; 
  }
  currentAmmoCount = totalAmmoCapacity; 
}


void drawWindDirectionArrow(Adafruit_SSD1306* display, int x, int y, float direction) {
  
  
  
  float arrowDirection = direction + 90.0f;
  
  while (arrowDirection >= 360.0f) arrowDirection -= 360.0f;
  while (arrowDirection < 0.0f) arrowDirection += 360.0f;
  
  
  float rad = arrowDirection * PI / 180.0;
  
  
  int arrowLength = 8;
  
  
  int endX = x + (int)(arrowLength * cos(rad));
  int endY = y + (int)(arrowLength * sin(rad));
  
  
  display->fillRect(x - 5, y - 5, 11, 11, BLACK);
  
  
  display->drawLine(x, y, endX, endY, WHITE);
  
  
  int headSize = 3;
  float headAngle1 = rad + 2.6; 
  float headAngle2 = rad - 2.6;
  
  int headX1 = endX + (int)(headSize * cos(headAngle1));
  int headY1 = endY + (int)(headSize * sin(headAngle1));
  int headX2 = endX + (int)(headSize * cos(headAngle2));
  int headY2 = endY + (int)(headSize * sin(headAngle2));
  
  display->drawLine(endX, endY, headX1, headY1, WHITE);
  display->drawLine(endX, endY, headX2, headY2, WHITE);
}


void drawDistanceInfo(Adafruit_SSD1306* display, float distance) {
  if (distance < 0.0f) return; 
  
  display->setTextSize(1);
  display->setTextColor(WHITE);
  
  char distanceStr[10];
  dtostrf(distance, 4, 1, distanceStr);
  strcat(distanceStr, "m");
  
  
  int textWidth = strlen(distanceStr) * 6; 
  int x = SCREEN_WIDTH - textWidth - 2; 
  int y = SCREEN_HEIGHT / 2 - 4; 
  
  display->setCursor(x, y);
  display->print(distanceStr);
}

void drawMainDisplay() {
  if (!g_display) return;
  
  
  updateIMUCache();
  
  
  updateStabilityCalculation();
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->setTextSize(1); 
  display->setTextColor(WHITE);
  display->setCursor(2, 2);
  
  
  float displayWindSpeed = windState.speed;
  float displayWindDirection = windState.direction;
  
  if (isnan(displayWindSpeed)) {
    displayWindSpeed = 0.0f;
  }
  if (isnan(displayWindDirection)) {
    displayWindDirection = 0.0f;
  }
  
  char windSpeedStr[10];
  dtostrf(displayWindSpeed, 3, 1, windSpeedStr);
  display->print(windSpeedStr);
  display->print("m/s");
  
  
  display->setCursor(2, 12); 
  char presetStr[10];
  int presetNum = presetManagerState.selectedPreset + 1; 
  if (presetNum >= 1 && presetNum <= 3) {
    sprintf(presetStr, "set %d", presetNum);
  } else {
    strcpy(presetStr, "set -"); 
  }
  display->print(presetStr);
  
  
  int arrowX = 2 + strlen(windSpeedStr) * 6 + 24; 
  int arrowY = 6; 
  
  drawWindDirectionArrow(display, arrowX, arrowY, displayWindDirection);
  
  
  // 计算准星中心位置（考虑安装偏移）
  int crosshairCenterX = SCREEN_WIDTH / 2 + currentActivePreset.mountAngleOffsetX;
  int crosshairCenterY = SCREEN_HEIGHT / 2 + currentActivePreset.mountAngleOffset;
  
  // 处理测距和弹着点显示
  bool hasValidDistance = false;
  float displayDistance = rangeFinderState.distance;
  
  if (rangeFinderState.selectedMode == RANGE_MODE_SINGLE || 
      rangeFinderState.selectedMode == RANGE_MODE_CONTINUOUS) {
    drawDistanceInfo(display, displayDistance);
    
    if (displayDistance > 0.5f) { // 距离大于0.5米才认为有效
      hasValidDistance = true;
    }
  }
  
  // 根据距离情况绘制弹着点
  if (hasValidDistance) {
    // 有有效距离：绘制包含弹道下坠的弹着点
    drawDropPoint(display, displayDistance);
  } else {
    // 无有效距离或距离很近：在准星中央绘制弹着点
    int size = 3;
    int startX = crosshairCenterX - size / 2;
    int startY = crosshairCenterY - size / 2;
    
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (startX + size > SCREEN_WIDTH) startX = SCREEN_WIDTH - size;
    if (startY + size > SCREEN_HEIGHT) startY = SCREEN_HEIGHT - size;
    
    display->fillRect(startX, startY, size, size, WHITE);
  }
  

  
  // 绘制准星
  drawCrosshair(crosshairCenterX, crosshairCenterY, crosshairState.selectedType);
  
  
  
  drawAmmoInfo(display);
  
  
  drawStabilityWaveform(display);
  
  display->display();
}

void handleMainDisplayInput() {
  
  if (rangeFinderState.selectedMode == RANGE_MODE_CONTINUOUS) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastMeasurementTime >= 100) {
      float measuredDistance = readTOFDistance(); 
      
      if (measuredDistance >= 0.0f) {
        rangeFinderState.distance = measuredDistance;
        lastMeasurementTime = currentTime;
      }
      
      else {
        lastMeasurementTime = currentTime;
      }
    }
  }
  
  
  if (rangeFinderState.selectedMode == RANGE_MODE_SINGLE) {
    static bool downButtonPressed = false;
    static unsigned long lastDownButtonTime = 0;
    const unsigned long DOWN_BUTTON_DEBOUNCE = 50;
    
    if (digitalRead(BTN_DOWN) == LOW) {
      unsigned long currentTime = millis();
      if (currentTime - lastDownButtonTime > DOWN_BUTTON_DEBOUNCE) {
        downButtonPressed = true;
        lastDownButtonTime = currentTime;
      }
    } else {
      downButtonPressed = false;
    }
    
    if (downButtonPressed) {
      float measuredDistance = readTOFDistance();
      if (measuredDistance >= 0.0f) {
        rangeFinderState.distance = measuredDistance;
        downButtonPressed = false;
      }
    }
  }

  
  if (digitalRead(BTN_UP) == LOW) {
    unsigned long currentTime = millis();
    static unsigned long lastReloadTime = 0;
    const unsigned long RELOAD_DEBOUNCE = 200; 
    
    if (currentTime - lastReloadTime > RELOAD_DEBOUNCE) {
      
      currentAmmoCount = totalAmmoCapacity;
      lastReloadTime = currentTime;
    }
  }
  
  
  detectShotAndDecrementAmmo();
}


static float medianPitchBuffer[5] = {0.0f};


#define PITCH_HYSTERESIS_DEADZONE_DEG 0.15f


float getMedianPitch(float newPitch) {
    
    medianPitchBuffer[0] = medianPitchBuffer[1];
    medianPitchBuffer[1] = medianPitchBuffer[2];
    medianPitchBuffer[2] = medianPitchBuffer[3];
    medianPitchBuffer[3] = medianPitchBuffer[4];
    medianPitchBuffer[4] = newPitch;
    
    
    float sorted[5];
    for (int i = 0; i < 5; i++) {
        sorted[i] = medianPitchBuffer[i];
    }
    
    
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (sorted[i] > sorted[j]) {
                float tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }
    
    return sorted[2]; 
}


float getDeadzonePitch(float rawPitch) {
    const float DEADZONE = PITCH_HYSTERESIS_DEADZONE_DEG * PI / 180.0f; 
    
    if (rawPitch > DEADZONE) {
        return rawPitch - DEADZONE;
    } else if (rawPitch < -DEADZONE) {
        return rawPitch + DEADZONE;
    } else {
        return 0.0f; 
    }
}


static float medianRollBuffer[5] = {0.0f};

float getMedianRoll(float newRoll) {
    
    medianRollBuffer[0] = medianRollBuffer[1];
    medianRollBuffer[1] = medianRollBuffer[2];
    medianRollBuffer[2] = medianRollBuffer[3];
    medianRollBuffer[3] = medianRollBuffer[4];
    medianRollBuffer[4] = newRoll;
    
    
    float sorted[5];
    for (int i = 0; i < 5; i++) {
        sorted[i] = medianRollBuffer[i];
    }
    
    
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (sorted[i] > sorted[j]) {
                float tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }
    
    return sorted[2]; 
}


#define ROLL_HYSTERESIS_DEADZONE_DEG 0.15f

float getDeadzoneRoll(float rawRoll) {
    const float DEADZONE = ROLL_HYSTERESIS_DEADZONE_DEG * PI / 180.0f; 
    
    if (rawRoll > DEADZONE) {
        return rawRoll - DEADZONE;
    } else if (rawRoll < -DEADZONE) {
        return rawRoll + DEADZONE;
    } else {
        return 0.0f; 
    }
}

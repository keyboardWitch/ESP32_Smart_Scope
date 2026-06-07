#include "ballistic_calibration.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include <cmath>


extern Adafruit_MPU6050 mpu;


extern MPU6050CalibrationData cachedMPUCalibrationData;


BallisticCalibrationState ballisticState;

void initBallisticCalibration() {
  ballisticState.selectedField = BALLISTIC_FIELD_MODEL_TYPE;
  ballisticState.currentPoint = CALIB_POINT_5M;
  
  
  for (int i = 0; i < 3; i++) {
    ballisticState.recordedPitch[i] = 0.0f;
    ballisticState.pointRecorded[i] = false;
  }
  
  
  PresetData currentPreset;
  loadPresetData(presetManagerState.selectedPreset, currentPreset);
  
  
  ballisticState.result = currentPreset.ballisticCalib;
  
  
  if (ballisticState.result.modelType < MODEL_TYPE_OFF || 
      ballisticState.result.modelType > MODEL_TYPE_POLYNOMIAL_FIT) {
    ballisticState.result.modelType = MODEL_TYPE_OFF;
  }
  
  
  if (!ballisticState.result.isCalibrated) {
    ballisticState.result.modelParams.physicsDrag.dragCoefficientK = 0.1f;
    ballisticState.result.pitch5m = 0.0f;
    ballisticState.result.pitch10m = 0.0f;
    ballisticState.result.pitch15m = 0.0f;
    ballisticState.result.mountAngleOffset = MOUNT_ANGLE_OFFSET;
    ballisticState.result.calibratedPoints = 0;
    ballisticState.result.checksum = 0;
    
  }
  
  ballisticState.calibrationComplete = false;
}


float getIMUPitch() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  
  
  
  
  
  
  
  if (isnan(a.acceleration.y) || isnan(a.acceleration.z)) {
    
    return 0.0f;
  }
  
  
  
  float accelY = a.acceleration.y; 
  float accelZ = a.acceleration.z;  
  
  
  if (cachedMPUCalibrationData.isCalibrated) {
    
    accelY -= cachedMPUCalibrationData.accelOffsetY;
    accelZ -= cachedMPUCalibrationData.accelOffsetZ;
  }

  
  if (fabs(accelZ) < 0.01f) {
    accelZ = (accelZ >= 0) ? 0.01f : -0.01f;
  }
  
  
  float pitch = atan2(accelY, accelZ);
  
  
  
  if (cachedMPUCalibrationData.isCalibrated) {
    pitch -= cachedMPUCalibrationData.pitchAngleOffset;
  }
  
  
  if (isnan(pitch)) {
    
    return 0.0f;
  }

  return pitch;
}


float calcPhysicsDrop(float distance, float bulletVelocity, float dragCoefficientK) {
  float g = 9.8f;
  
  
  float kd_v0 = dragCoefficientK * distance / bulletVelocity;
  float exp_term = exp(kd_v0);
  float drop = (g / (dragCoefficientK * dragCoefficientK)) * (exp_term - 1.0f) - (g * distance) / (dragCoefficientK * bulletVelocity);
  return drop;
}


float calibrateKByPitch(float pitch5, float pitch10, float pitch15, float bulletVelocity) {
  
  
  float h5  =  5.0f * tan(pitch5);
  float h10 = 10.0f * tan(pitch10);
  float h15 = 15.0f * tan(pitch15);

  float low_k = 0.05f;   
  float high_k = 50.0f;   //原来最大为2.5，放宽范围以适应更大阻力的子弹（如重弹头或低速弹）
  float best_k = 0.1f;
  float min_error = 999999.0f;

  
  for(float test_k = low_k; test_k <= high_k; test_k += 0.01f) {
    float c5  = calcPhysicsDrop(5.0f,  bulletVelocity, test_k);
    float c10 = calcPhysicsDrop(10.0f, bulletVelocity, test_k);
    float c15 = calcPhysicsDrop(15.0f, bulletVelocity, test_k);

    
    float error = pow(c5 - h5, 2) + pow(c10 - h10, 2) + pow(c15 - h15, 2);

    if(error < min_error) {
      min_error = error;
      best_k = test_k;
    }
  }
  
  
  
  if (best_k < low_k) {
    best_k = low_k; 
  }
  if (best_k > high_k) {
    best_k = high_k; 
  }
  
  
  return best_k; 
}




bool calibratePolynomialCoefficients(float pitch5, float pitch10, float pitch15, PolynomialFitModelParams& params) {
  
  
  float h5 = 5.0f * tan(pitch5);
  float h10 = 10.0f * tan(pitch10);
  float h15 = 15.0f * tan(pitch15);
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  float denominator = 50.0f;
  if (fabs(denominator) < 1e-6f) {
    
    params.coefficientA = 0.0f;
    params.coefficientB = 0.0f;
    params.coefficientC = 0.0f;
    return false;
  }
  
  params.coefficientA = (h15 - 2.0f * h10 + h5) / denominator;
  
  
  
  
  params.coefficientB = (h10 - h5 - 75.0f * params.coefficientA) / 5.0f;
  
  
  
  params.coefficientC = h5 - 25.0f * params.coefficientA - 5.0f * params.coefficientB;
  
  
  if (isnan(params.coefficientA) || isnan(params.coefficientB) || isnan(params.coefficientC) ||
      isinf(params.coefficientA) || isinf(params.coefficientB) || isinf(params.coefficientC)) {
    params.coefficientA = 0.0f;
    params.coefficientB = 0.0f;
    params.coefficientC = 0.0f;
    return false;
  }
  
  return true;
}


bool calculateBallisticCoefficients() {
  
  if (!ballisticState.pointRecorded[0] || 
      !ballisticState.pointRecorded[1] || 
      !ballisticState.pointRecorded[2]) {
    return false;
  }
  
  
  PresetData currentPreset;
  loadPresetData(presetManagerState.selectedPreset, currentPreset);
  float bulletVelocity = currentPreset.bulletVelocity;
  if (bulletVelocity <= 0.0f) {
    bulletVelocity = 50.0f; 
  }
  
  
  float pitch5 = ballisticState.recordedPitch[0];
  float pitch10 = ballisticState.recordedPitch[1];
  float pitch15 = ballisticState.recordedPitch[2];
  
  
  if (ballisticState.result.modelType == MODEL_TYPE_PHYSICS_DRAG) {
    
    float bestK = calibrateKByPitch(pitch5, pitch10, pitch15, bulletVelocity);
    
    
    ballisticState.result.modelParams.physicsDrag.dragCoefficientK = bestK;
  } else if (ballisticState.result.modelType == MODEL_TYPE_POLYNOMIAL_FIT) {
    
    if (!calibratePolynomialCoefficients(pitch5, pitch10, pitch15, 
                                       ballisticState.result.modelParams.polynomialFit)) {
      return false;
    }
  }
  
  
  
  ballisticState.result.pitch5m = pitch5;
  ballisticState.result.pitch10m = pitch10;
  ballisticState.result.pitch15m = pitch15;
  ballisticState.result.isCalibrated = true;
  ballisticState.result.calibratedPoints = 3;
  
  
  uint32_t sum = 0;
  
  if (ballisticState.result.modelType == MODEL_TYPE_PHYSICS_DRAG) {
    sum += *((uint32_t*)&ballisticState.result.modelParams.physicsDrag.dragCoefficientK);
  } else if (ballisticState.result.modelType == MODEL_TYPE_POLYNOMIAL_FIT) {
    sum += *((uint32_t*)&ballisticState.result.modelParams.polynomialFit.coefficientA);
    sum += *((uint32_t*)&ballisticState.result.modelParams.polynomialFit.coefficientB);
    sum += *((uint32_t*)&ballisticState.result.modelParams.polynomialFit.coefficientC);
  }
  
  sum += *((uint32_t*)&pitch5);
  sum += *((uint32_t*)&pitch10);
  sum += *((uint32_t*)&pitch15);
  sum += *((uint32_t*)&ballisticState.result.mountAngleOffset);
  ballisticState.result.checksum = (uint16_t)(sum & 0xFFFF);
  
  
  if (ballisticState.result.modelType == MODEL_TYPE_OFF) {
    
  } else if (ballisticState.result.modelType == MODEL_TYPE_PHYSICS_DRAG) {
    
    
  } else {
    
    
    
    
    
    
  }
  
  
  
  
  return true;
}

void drawBallisticCalibrationScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_BALLISTIC_CALIBRATION) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  static const char* modelNames[] = {"OFF", "Phys", "Poly"};
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  if (ballisticState.calibrationComplete) {
    display->print("Calibration Done");
  } else {
    display->print("Ballistic Calib");
  }
  
  
  char debugBuffer[20];
  sprintf(debugBuffer, "SF:%d", ballisticState.selectedField);
  display->setCursor(SCREEN_WIDTH - 30, 2);
  display->print(debugBuffer);
  
  
  #define BALLISTIC_TOTAL_ITEMS 4
  
  #define BALLISTIC_ITEMS_PER_PAGE 4
  
  
  int scrollOffset = 0;
  if (ballisticState.selectedField >= BALLISTIC_ITEMS_PER_PAGE) {
    scrollOffset = ballisticState.selectedField - BALLISTIC_ITEMS_PER_PAGE + 1;
    if (scrollOffset > BALLISTIC_TOTAL_ITEMS - BALLISTIC_ITEMS_PER_PAGE) {
      scrollOffset = BALLISTIC_TOTAL_ITEMS - BALLISTIC_ITEMS_PER_PAGE;
    }
  }
  
  
  int startIdx = scrollOffset;
  int endIdx = min(startIdx + BALLISTIC_ITEMS_PER_PAGE, BALLISTIC_TOTAL_ITEMS);
  
  
  display->setTextSize(1);
  
  
  int startY = 18;
  int lineHeight = 10;
  
  for (int i = startIdx; i < endIdx; i++) {
    int y = startY + (i - startIdx) * lineHeight;
    
    if (ballisticState.calibrationComplete) {
      
      if (i == BALLISTIC_FIELD_MODEL_TYPE) {
        
        display->setTextColor(WHITE);
        char buffer[30];
        int currentModel = ballisticState.result.modelType;
        if (currentModel >= MODEL_TYPE_COUNT) currentModel = 0;
        
        
        const char* modelName = modelNames[currentModel];
        
        if (currentModel == MODEL_TYPE_OFF) {
          
          display->setCursor(10, y);
          display->print(modelName);
        }
        else if (currentModel == MODEL_TYPE_PHYSICS_DRAG) {
          
          dtostrf(ballisticState.result.modelParams.physicsDrag.dragCoefficientK, 4, 2, buffer);
          char fullBuffer[30];
          sprintf(fullBuffer, "%s k:%s", modelName, buffer);
          display->setCursor(10, y);
          display->print(fullBuffer);
        } else {
          
          dtostrf(ballisticState.result.modelParams.polynomialFit.coefficientA, 4, 2, buffer);
          char fullBuffer[30];
          sprintf(fullBuffer, "%s a:%s", modelName, buffer);
          display->setCursor(10, y);
          display->print(fullBuffer);
        }
      }
      else if (i == BALLISTIC_FIELD_CALIBRATION) {
        
        display->setTextColor(WHITE);
        
        char buffer[30];
        float pitch5_rad = ballisticState.result.pitch5m;
        float pitch5_deg = pitch5_rad * 180.0f / M_PI;
        
        if (pitch5_deg > 99.9f) pitch5_deg = 99.9f;
        if (pitch5_deg < -99.9f) pitch5_deg = -99.9f;
        sprintf(buffer, "5m:%.1f Deg", pitch5_deg);
        display->setCursor(10, y);
        display->print(buffer);
      }
      else if (i == BALLISTIC_FIELD_SHOOT_ADJUST) {
        
        display->setTextColor(WHITE);
        char buffer[30];
        float pitch10_rad = ballisticState.result.pitch10m;
        float pitch10_deg = pitch10_rad * 180.0f / M_PI;
        if (pitch10_deg > 99.9f) pitch10_deg = 99.9f;
        if (pitch10_deg < -99.9f) pitch10_deg = -99.9f;
        sprintf(buffer, "10m:%.1f Deg", pitch10_deg);
        display->setCursor(10, y);
        display->print(buffer);
      }
      else if (i == BALLISTIC_FIELD_SAVE) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        char buffer[30];
        float pitch15_rad = ballisticState.result.pitch15m;
        float pitch15_deg = pitch15_rad * 180.0f / M_PI;
        if (pitch15_deg > 99.9f) pitch15_deg = 99.9f;
        if (pitch15_deg < -99.9f) pitch15_deg = -99.9f;
        sprintf(buffer, "15m:%.1f Deg", pitch15_deg);
        display->setCursor(10, y);
        display->print(buffer);
      }
    } else {
      
      if (i == BALLISTIC_FIELD_MODEL_TYPE) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        char buffer[30];
        int currentModel = ballisticState.result.modelType;
        if (currentModel >= MODEL_TYPE_COUNT) currentModel = 0;
        
        sprintf(buffer, "Mdl:%s", modelNames[currentModel]);  
        display->setCursor(10, y);
        display->print(buffer);
      }
      else if (i == BALLISTIC_FIELD_CALIBRATION) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        char buffer[30];
        const char* pointNames[] = {"5m", "10m", "15m"};  
        sprintf(buffer, "Cal:%s", pointNames[ballisticState.currentPoint]);  
        display->setCursor(10, y);
        display->print(buffer);
        
        
        if (ballisticState.pointRecorded[ballisticState.currentPoint]) {
          display->setCursor(SCREEN_WIDTH - 20, y);
          display->print("OK");
        }
      }
      else if (i == BALLISTIC_FIELD_SHOOT_ADJUST) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        display->setCursor(10, y);
        display->print("Calc");  
      }
      else if (i == BALLISTIC_FIELD_SAVE) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        display->setCursor(10, y);
        display->print("Save");  
      }
    }
  }
  
  
  if (BALLISTIC_TOTAL_ITEMS > BALLISTIC_ITEMS_PER_PAGE) {
    
    if (scrollOffset > 0) {
      display->drawTriangle(SCREEN_WIDTH - 8, 14, SCREEN_WIDTH - 4, 10, SCREEN_WIDTH - 12, 10, WHITE);
    }
    
    if (scrollOffset < BALLISTIC_TOTAL_ITEMS - BALLISTIC_ITEMS_PER_PAGE) {
      display->drawTriangle(SCREEN_WIDTH - 8, SCREEN_HEIGHT - 6, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 2, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 2, WHITE);
    }
  }
  
  display->display();
}

void handleBallisticCalibrationInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (ballisticState.selectedField > 0) {
      ballisticState.selectedField--;
      needRedraw = true;
    }
  }
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (ballisticState.selectedField < BALLISTIC_FIELD_COUNT - 1) {
      ballisticState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (ballisticState.selectedField == BALLISTIC_FIELD_MODEL_TYPE) {
    
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      if (ballisticState.result.modelType > 0) {
        ballisticState.result.modelType--;
        needRedraw = true;
      }
    }
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      if (ballisticState.result.modelType < 2) { 
        ballisticState.result.modelType++;
        needRedraw = true;
      }
    }
  }
  else if (!ballisticState.calibrationComplete && ballisticState.selectedField == BALLISTIC_FIELD_CALIBRATION) {
    
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      if (ballisticState.currentPoint > 0) {
        ballisticState.currentPoint--;
        needRedraw = true;
      }
    }
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      if (ballisticState.currentPoint < 2) { 
        ballisticState.currentPoint++;
        needRedraw = true;
      }
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (ballisticState.calibrationComplete) {
      if (ballisticState.selectedField == BALLISTIC_FIELD_SAVE) {
        PresetData currentPreset;
        loadPresetData(presetManagerState.selectedPreset, currentPreset);
        currentPreset.ballisticCalib = ballisticState.result;
        savePresetData(presetManagerState.selectedPreset, currentPreset);
        currentMode = MODE_MENU;
        drawMenu();
        return;
      }
      else if (ballisticState.selectedField == BALLISTIC_FIELD_SHOOT_ADJUST) {
        bool allPointsRecorded = ballisticState.pointRecorded[0] && 
                               ballisticState.pointRecorded[1] && 
                               ballisticState.pointRecorded[2];
        if (allPointsRecorded && calculateBallisticCoefficients()) {
          needRedraw = true;
        }
      }
    } else {
      if (ballisticState.selectedField == BALLISTIC_FIELD_CALIBRATION) {
        float currentPitch = getIMUPitch();
        ballisticState.recordedPitch[ballisticState.currentPoint] = currentPitch;
        ballisticState.pointRecorded[ballisticState.currentPoint] = true;
        needRedraw = true;
        if (ballisticState.currentPoint < CALIB_POINT_COUNT - 1) {
          ballisticState.currentPoint++;
        }
      }
      else if (ballisticState.selectedField == BALLISTIC_FIELD_SHOOT_ADJUST) {
        if (calculateBallisticCoefficients()) {
          ballisticState.calibrationComplete = true;
          ballisticState.selectedField = BALLISTIC_FIELD_SAVE;
          needRedraw = true;
        }
      }
      else if (ballisticState.selectedField == BALLISTIC_FIELD_SAVE) {
        PresetData currentPreset;
        loadPresetData(presetManagerState.selectedPreset, currentPreset);
        currentPreset.ballisticCalib = ballisticState.result;
        savePresetData(presetManagerState.selectedPreset, currentPreset);
        currentMode = MODE_MENU;
        drawMenu();
        return;
      }
    }
  }
  
  if (needRedraw) {
    drawBallisticCalibrationScreen();
  }
}

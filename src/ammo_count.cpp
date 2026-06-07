#include "ammo_count.h"
#include "config.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "system_init.h"


extern Adafruit_MPU6050 mpu;
extern bool mpu6050Working;


AmmoCountState ammoCountState = {30, 0, SHOT_IDLE, 0, 0, SHOT_DETECTION_THRESHOLD}; 

void initAmmoCount() {
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  ammoCountState.capacity = preset.ammoCount;
  ammoCountState.selectedField = AMMO_FIELD_CAPACITY; 
  ammoCountState.shotState = SHOT_IDLE;
  ammoCountState.vibrationThreshold = preset.shotVibrationThreshold;
}


float calculateAccelerationMagnitude(sensors_event_t& accelEvent) {
  float ax = accelEvent.acceleration.x;
  float ay = accelEvent.acceleration.y;
  float az = accelEvent.acceleration.z;
  return sqrt(ax * ax + ay * ay + az * az);
}


bool detectShotVibration() {
  if (!mpu6050Working) {
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
  
  
  float accelMagnitude = calculateAccelerationMagnitude(a);
  float vibrationLevel = fabs(accelMagnitude - 9.81f); 
  
  
  
  
  
  return vibrationLevel > ammoCountState.vibrationThreshold;
}

void drawAmmoCountScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_AMMO_COUNT) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Ammo Count");
  
  
  display->setTextSize(1);
  int y = 18;
  
  
  if (ammoCountState.selectedField == AMMO_FIELD_CAPACITY) {
    display->fillRect(5, y - 2, SCREEN_WIDTH - 10, 10, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  
  display->setCursor(8, y);
  display->print("Capacity: ");
  
  char capacityStr[10];
  itoa(ammoCountState.capacity, capacityStr, 10);
  display->setCursor(80, y);
  display->print(capacityStr);
  
  
  y = 28;
  if (ammoCountState.selectedField == AMMO_FIELD_SHOT_DETECTION) {
    display->fillRect(5, y - 2, SCREEN_WIDTH - 10, 10, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  
  display->setCursor(8, y);
  display->print("Shot Detect: ");
  
  
  char statusStr[20];
  switch (ammoCountState.shotState) {
    case SHOT_IDLE:
      strcpy(statusStr, "Idle");
      break;
    case SHOT_DETECTING:
      strcpy(statusStr, "Detecting...");
      break;
    case SHOT_DETECTED:
      strcpy(statusStr, "OK!");
      break;
    default:
      strcpy(statusStr, "Unknown");
      break;
  }
  display->setCursor(80, y);
  display->print(statusStr);
  
  
  int separatorY = SCREEN_HEIGHT - 20;
  display->drawLine(5, separatorY, SCREEN_WIDTH - 5, separatorY, WHITE);
  
  
  int saveY = SCREEN_HEIGHT - 12;
  if (ammoCountState.selectedField == AMMO_FIELD_SAVE) {
    display->fillRect(8, saveY, SCREEN_WIDTH - 16, 10, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  display->setCursor(10, saveY + 2);
  display->print("Save & Exit");
  
  display->display();
}

void handleAmmoCountInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  unsigned long currentTime = millis();
  
  
  if (ammoCountState.shotState == SHOT_DETECTING) {
    
    if (currentTime - ammoCountState.detectionStartTime > SHOT_DETECTION_WINDOW) {
      
      ammoCountState.shotState = SHOT_IDLE;
      ammoCountState.consecutiveDetections = 0;
      needRedraw = true;
    } else {
      
      if (detectShotVibration()) {
        ammoCountState.consecutiveDetections++;
        
        
        
        if (ammoCountState.consecutiveDetections >= CONSECUTIVE_DETECTIONS_REQUIRED) {
          
          ammoCountState.shotState = SHOT_DETECTED;
          needRedraw = true;
          
          
          PresetData preset;
          loadPresetData(presetManagerState.selectedPreset, preset);
          preset.shotDetectionEnabled = true;
          preset.shotVibrationThreshold = ammoCountState.vibrationThreshold;
          preset.shotDetectionTime = currentTime;
          savePresetData(presetManagerState.selectedPreset, preset);
          
          
        }
      }
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (ammoCountState.selectedField > 0) {
      ammoCountState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (ammoCountState.selectedField < AMMO_FIELD_COUNT - 1) {
      ammoCountState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (ammoCountState.selectedField == AMMO_FIELD_CAPACITY) {
    
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      if (ammoCountState.capacity > 1) {
        ammoCountState.capacity--;
        needRedraw = true;
      }
    }
    
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      if (ammoCountState.capacity < 999) {
        ammoCountState.capacity++;
        needRedraw = true;
      }
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (ammoCountState.selectedField == AMMO_FIELD_SHOT_DETECTION) {
      
      if (ammoCountState.shotState == SHOT_IDLE || ammoCountState.shotState == SHOT_DETECTED) {
        ammoCountState.shotState = SHOT_DETECTING;
        ammoCountState.detectionStartTime = currentTime;
        ammoCountState.consecutiveDetections = 0;
        needRedraw = true;
        
      }
    } else if (ammoCountState.selectedField == AMMO_FIELD_SAVE) {
      
      PresetData preset;
      loadPresetData(presetManagerState.selectedPreset, preset);
      preset.ammoCount = ammoCountState.capacity;
      
      savePresetData(presetManagerState.selectedPreset, preset);
      
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  
  if (ammoCountState.shotState == SHOT_DETECTED) {
    if (currentTime - ammoCountState.detectionStartTime > SHOT_DETECTION_COOLDOWN) {
      ammoCountState.shotState = SHOT_IDLE;
      needRedraw = true;
    }
  }
  
  if (needRedraw) {
    drawAmmoCountScreen();
  }
}
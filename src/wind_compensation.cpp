#include "wind_compensation.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include "config.h"


WindCompensationState windState = {DEFAULT_WIND_SPEED, DEFAULT_WIND_DIRECTION, 0}; 

void initWindCompensation() {
  
  
  windState.speed = DEFAULT_WIND_SPEED;
  windState.direction = DEFAULT_WIND_DIRECTION;
  
  
  if (isnan(windState.speed)) {
    windState.speed = 0.0f;
  }
  if (isnan(windState.direction)) {
    windState.direction = 0.0f;
  }
  
  windState.selectedField = WIND_FIELD_SPEED; 
}

void drawWindCompensationScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_WIND_COMP) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Wind Comp");
  
  
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(10, 18);
  display->print("Spd:");
  
  
  if (windState.selectedField == WIND_FIELD_SPEED) {
    display->fillRect(45, 16, 70, 10, WHITE);
    display->setTextColor(BLACK);
  }
  display->setCursor(50, 18);
  char speedStr[10];
  dtostrf(windState.speed, 4, 1, speedStr);
  display->print(speedStr);
  display->print("m/s");
  
  
  display->setTextColor(WHITE);
  display->setCursor(10, 28);
  display->print("Dir:");
  
  
  if (windState.selectedField == WIND_FIELD_DIRECTION) {
    display->fillRect(45, 26, 70, 10, WHITE);
    display->setTextColor(BLACK);
  }
  display->setCursor(50, 28);
  char dirStr[10];
  dtostrf(windState.direction, 4, 0, dirStr);
  display->print(dirStr);
  display->print((char)247);  
  
  
  int saveY = SCREEN_HEIGHT - 12;
  if (windState.selectedField == WIND_FIELD_SAVE) {
    display->fillRect(8, saveY, SCREEN_WIDTH - 16, 10, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  display->setCursor(10, saveY + 2);
  display->print("Save & Exit");
  
  
  
  display->display();
}

void handleWindCompensationInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (windState.selectedField > 0) {
      windState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (windState.selectedField < WIND_FIELD_COUNT - 1) {
      windState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (windState.selectedField == WIND_FIELD_SPEED || windState.selectedField == WIND_FIELD_DIRECTION) {
    
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      if (windState.selectedField == WIND_FIELD_SPEED) {
        windState.speed -= 0.5;
        if (windState.speed < 0.0) windState.speed = 0.0;
      } else {
        windState.direction -= 5.0;
        if (windState.direction < 0.0) windState.direction = 0.0;
      }
      needRedraw = true;
    }
    
    
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      if (windState.selectedField == WIND_FIELD_SPEED) {
        windState.speed += 0.5;
        if (windState.speed > 20.0) windState.speed = 20.0;
      } else {
        windState.direction += 5.0;
        if (windState.direction > 360.0) windState.direction = 360.0;
      }
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (windState.selectedField == WIND_FIELD_SAVE) {
      Serial.print("Wind Compensation Saved - Speed: ");
      Serial.print(windState.speed);
      Serial.print(" m/s, Direction: ");
      Serial.print(windState.direction);
      Serial.println((char)247);
      
      
      PresetData preset;
      loadPresetData(presetManagerState.selectedPreset, preset);
      preset.windSpeed = windState.speed;
      preset.windDirection = windState.direction;
      savePresetData(presetManagerState.selectedPreset, preset);
      Serial.print("Updated wind data in active Preset ");
      Serial.println(presetManagerState.selectedPreset + 1);
      
      currentMode = MODE_MENU;  
      drawMenu();               
      return;
    }
  }
  
  
  
  
  if (isButtonPressed(BTN_INDEX_LEFT)) {
    if (windState.selectedField == WIND_FIELD_SAVE) {
      
      Serial.println("Wind Compensation: Cancelled");
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  if (needRedraw) {
    drawWindCompensationScreen();
  }
}

bool isWindCompensationMode() {
  
  
  return false;
}
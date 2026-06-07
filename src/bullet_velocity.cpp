#include "bullet_velocity.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include "config.h"


BulletVelocityState bulletVelocityState = {BULLET_VELOCITY_DEFAULT, 0}; 

void initBulletVelocity() {
  
  
  bulletVelocityState.velocity = BULLET_VELOCITY_DEFAULT;
  bulletVelocityState.selectedField = BULLET_VELOCITY_FIELD_VALUE; 
  
  
  if (bulletVelocityState.velocity < BULLET_VELOCITY_MIN || bulletVelocityState.velocity > BULLET_VELOCITY_MAX) {
    bulletVelocityState.velocity = BULLET_VELOCITY_DEFAULT;
  }
}

void drawBulletVelocityScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_BULLET_VELOCITY) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Bullet Velocity");
  
  
  display->setTextSize(1);
  int y = 22; 
  
  
  if (bulletVelocityState.selectedField == BULLET_VELOCITY_FIELD_VALUE) {
    display->fillRect(5, y - 2, SCREEN_WIDTH - 10, 10, WHITE); 
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  
  display->setCursor(8, y);
  display->print("Velocity: ");
  
  char velocityStr[10];
  itoa(bulletVelocityState.velocity, velocityStr, 10);
  display->setCursor(80, y);
  display->print(velocityStr);
  display->print(" m/s");
  
  
  int saveY = SCREEN_HEIGHT - 12;
  if (bulletVelocityState.selectedField == BULLET_VELOCITY_FIELD_SAVE) {
    display->fillRect(8, saveY, SCREEN_WIDTH - 16, 10, WHITE);
    display->setTextColor(BLACK);
  } else {
    display->setTextColor(WHITE);
  }
  display->setCursor(10, saveY + 2);
  display->print("Save & Exit");
  
  display->display();
}

void handleBulletVelocityInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (bulletVelocityState.selectedField == BULLET_VELOCITY_FIELD_SAVE) {
      bulletVelocityState.selectedField = BULLET_VELOCITY_FIELD_VALUE;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (bulletVelocityState.selectedField == BULLET_VELOCITY_FIELD_VALUE) {
      bulletVelocityState.selectedField = BULLET_VELOCITY_FIELD_SAVE;
      needRedraw = true;
    }
  }
  
  
  if (bulletVelocityState.selectedField == BULLET_VELOCITY_FIELD_VALUE) {
    
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      if (bulletVelocityState.velocity > 1) {
        bulletVelocityState.velocity -= 5;
        if (bulletVelocityState.velocity < 1) bulletVelocityState.velocity = 1;
        needRedraw = true;
      }
    }
    
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      if (bulletVelocityState.velocity < 500) {
        bulletVelocityState.velocity += 5;
        if (bulletVelocityState.velocity > 500) bulletVelocityState.velocity = 500;
        needRedraw = true;
      }
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (bulletVelocityState.selectedField == BULLET_VELOCITY_FIELD_SAVE) {
      
      PresetData preset;
      loadPresetData(presetManagerState.selectedPreset, preset);
      preset.bulletVelocity = static_cast<float>(bulletVelocityState.velocity);
      savePresetData(presetManagerState.selectedPreset, preset);
      
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  if (needRedraw) {
    drawBulletVelocityScreen();
  }
}
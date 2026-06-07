#include "mount_angle_offset.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"


MountAngleOffsetState mountAngleOffsetState = {0, MOUNT_ANGLE_OFFSET_FIELD_CROSSHAIR};


extern int g_activePresetIndex;

void initMountAngleOffset() {
  
  PresetData currentPreset;
  loadPresetData(g_activePresetIndex, currentPreset);
  mountAngleOffsetState.mountAngleOffset = currentPreset.mountAngleOffset;
  mountAngleOffsetState.selectedField = MOUNT_ANGLE_OFFSET_FIELD_CROSSHAIR;
  mountAngleOffsetState.adjustingCrosshair = false;
}

void drawMountAngleOffsetScreen() {
  if (!g_display) return;
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Mount Angle Offset");
  
  
  if (mountAngleOffsetState.adjustingCrosshair) {
    
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2 + mountAngleOffsetState.mountAngleOffset;
    
    
    centerY = constrain(centerY, 0, SCREEN_HEIGHT - 1);
    
    
    int size = 10;
    display->drawLine(centerX - size, centerY, centerX + size, centerY, WHITE); 
    display->drawLine(centerX, centerY - size, centerX, centerY + size, WHITE); 
    
    
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(5, SCREEN_HEIGHT - 10);
    display->print("Offset: ");
    display->print(mountAngleOffsetState.mountAngleOffset);
    display->print(" px");
  } else {
    
    display->setTextSize(1);
    
    int startY = 18;
    int lineHeight = 10;
    
    
    if (mountAngleOffsetState.selectedField == MOUNT_ANGLE_OFFSET_FIELD_CROSSHAIR) {
      display->fillRect(5, startY - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    display->setCursor(10, startY);
    display->print("Adjust Crosshair");
    
    
    if (mountAngleOffsetState.selectedField == MOUNT_ANGLE_OFFSET_FIELD_SAVE) {
      display->fillRect(5, startY + lineHeight - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    display->setCursor(10, startY + lineHeight);
    display->print("Save & Exit");
  }
  
  display->display();
}

void handleMountAngleOffsetInput() {
  bool needRedraw = false;
  
  if (mountAngleOffsetState.adjustingCrosshair) {
    
    if (isButtonPressed(BTN_INDEX_UP)) {
      mountAngleOffsetState.mountAngleOffset--;
      needRedraw = true;
    }
    
    if (isButtonPressed(BTN_INDEX_DOWN)) {
      mountAngleOffsetState.mountAngleOffset++;
      needRedraw = true;
    }
    
    if (isButtonPressed(BTN_INDEX_ENTER)) {
      
      mountAngleOffsetState.adjustingCrosshair = false;
      mountAngleOffsetState.selectedField = MOUNT_ANGLE_OFFSET_FIELD_CROSSHAIR;
      needRedraw = true;
    }
  } else {
    
    if (isButtonPressed(BTN_INDEX_UP)) {
      if (mountAngleOffsetState.selectedField > 0) {
        mountAngleOffsetState.selectedField--;
        needRedraw = true;
      }
    }
    
    if (isButtonPressed(BTN_INDEX_DOWN)) {
      if (mountAngleOffsetState.selectedField < MOUNT_ANGLE_OFFSET_FIELD_COUNT - 1) {
        mountAngleOffsetState.selectedField++;
        needRedraw = true;
      }
    }
    
    if (isButtonPressed(BTN_INDEX_ENTER)) {
      if (mountAngleOffsetState.selectedField == MOUNT_ANGLE_OFFSET_FIELD_CROSSHAIR) {
        
        mountAngleOffsetState.adjustingCrosshair = true;
        needRedraw = true;
      } else if (mountAngleOffsetState.selectedField == MOUNT_ANGLE_OFFSET_FIELD_SAVE) {
        
        PresetData currentPreset;
        loadPresetData(g_activePresetIndex, currentPreset);
        currentPreset.mountAngleOffset = mountAngleOffsetState.mountAngleOffset;
        savePresetData(g_activePresetIndex, currentPreset);
        
        
        currentMode = MODE_MENU;
        drawMenu();
        return;
      }
    }
  }
  
  if (needRedraw) {
    drawMountAngleOffsetScreen();
  }
}


void mountAngleOffset() {
  currentMode = MODE_MOUNT_ANGLE;
  initMountAngleOffset();
  drawMountAngleOffsetScreen();
}
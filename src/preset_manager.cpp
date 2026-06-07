#include "preset_manager.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"


PresetManagerState presetManagerState = {-1, 0}; 


const char* presetNames[] = {
  "Preset 1",
  "Preset 2", 
  "Preset 3"
};


#define PRESET_MANAGER_TOTAL_ITEMS 4

#define PRESET_MANAGER_ITEMS_PER_PAGE 4

void initPresetManager() {
  
  presetManagerState.selectedPreset = loadActivePreset();
  
  presetManagerState.selectedField = PRESET_FIELD_PRESET1;
}

void drawPresetManagerScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_PRESET_MANAGER) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Preset Manager");
  
  
  int scrollOffset = 0;
  if (presetManagerState.selectedField >= PRESET_MANAGER_ITEMS_PER_PAGE) {
    
    scrollOffset = presetManagerState.selectedField - PRESET_MANAGER_ITEMS_PER_PAGE + 1;
    
    if (scrollOffset > PRESET_MANAGER_TOTAL_ITEMS - PRESET_MANAGER_ITEMS_PER_PAGE) {
      scrollOffset = PRESET_MANAGER_TOTAL_ITEMS - PRESET_MANAGER_ITEMS_PER_PAGE;
    }
  }
  
  
  int startIdx = scrollOffset;
  int endIdx = min(startIdx + PRESET_MANAGER_ITEMS_PER_PAGE, PRESET_MANAGER_TOTAL_ITEMS);
  
  
  display->setTextSize(1);
  
  
  int startY = 18;
  int lineHeight = 10;
  
  for (int i = startIdx; i < endIdx; i++) {
    int y = startY + (i - startIdx) * lineHeight;
    
    
    if (i == presetManagerState.selectedField) {
      display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    
    if (i < 3) {
      
      
      if (i == presetManagerState.selectedPreset) {
        display->setCursor(8, y);
        display->print("*");  
      } else {
        display->setCursor(8, y);
        display->print(" ");  
      }
      
      display->setCursor(20, y);
      display->print(presetNames[i]);
    } else {
      
      
      
      
      
      
      
      
      
      
      display->setCursor(10, y);
      display->print("Save & Exit");
    }
  }
  
  
  if (PRESET_MANAGER_TOTAL_ITEMS > PRESET_MANAGER_ITEMS_PER_PAGE) {
    display->setTextColor(WHITE);
    if (startIdx > 0) {
      display->setCursor(SCREEN_WIDTH - 10, 2); 
      display->print("^");
    }
    if (endIdx < PRESET_MANAGER_TOTAL_ITEMS) {
      display->setCursor(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
      display->print("v");
    }
  }
  
  display->display();
}

void handlePresetManagerInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (presetManagerState.selectedField > 0) {
      presetManagerState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (presetManagerState.selectedField < PRESET_MANAGER_TOTAL_ITEMS - 1) {
      presetManagerState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (presetManagerState.selectedField < PRESET_FIELD_SAVE) {
      
      presetManagerState.selectedPreset = presetManagerState.selectedField;
      needRedraw = true;
    } else if (presetManagerState.selectedField == PRESET_FIELD_SAVE) {
      
      saveActivePreset(presetManagerState.selectedPreset);
      
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  if (needRedraw) {
    drawPresetManagerScreen();
  }
}
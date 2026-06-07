#include "arc_points_settings.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"


ArcPointsSettingsState arcPointsState;


const char* displayModeNames[] = {
  "Solid",
  "Blink", 
  "Off"
};


#define ARC_POINTS_TOTAL_ITEMS 3

#define ARC_POINTS_ITEMS_PER_PAGE 4

void initArcPointsSettings() {
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  arcPointsState.selectedMode = static_cast<ArcPointsMode>(preset.arcPointState);
  arcPointsState.tiltModeEnabled = preset.tiltModeEnabled;
  arcPointsState.selectedField = 0; 
}

void drawArcPointsSettingsScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_ARC_POINTS_SETTINGS) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Arc Points");
  
  
  int scrollOffset = 0;
  if (arcPointsState.selectedField >= ARC_POINTS_ITEMS_PER_PAGE) {
    scrollOffset = arcPointsState.selectedField - ARC_POINTS_ITEMS_PER_PAGE + 1;
    if (scrollOffset > ARC_POINTS_TOTAL_ITEMS - ARC_POINTS_ITEMS_PER_PAGE) {
      scrollOffset = ARC_POINTS_TOTAL_ITEMS - ARC_POINTS_ITEMS_PER_PAGE;
    }
  }
  
  
  int startIdx = scrollOffset;
  int endIdx = min(startIdx + ARC_POINTS_ITEMS_PER_PAGE, ARC_POINTS_TOTAL_ITEMS);
  
  
  display->setTextSize(1);
  
  
  int startY = 18;
  int lineHeight = 10;
  
  for (int i = startIdx; i < endIdx; i++) {
    int y = startY + (i - startIdx) * lineHeight;
    
    
    if (i == arcPointsState.selectedField) {
      display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    
    if (i == 0) {
      
      display->setCursor(10, y);
      display->print("Display Mode:");
      
      
      display->setCursor(SCREEN_WIDTH - 6 * strlen(displayModeNames[arcPointsState.selectedMode]) - 5, y);
      display->print(displayModeNames[arcPointsState.selectedMode]);
      
    } else if (i == 1) {
      
      display->setCursor(10, y);
      display->print("Tilt Mode:");
      
      
      if (arcPointsState.tiltModeEnabled) {
        display->setCursor(SCREEN_WIDTH - 25, y);
        display->print("ON");
      } else {
        display->setCursor(SCREEN_WIDTH - 30, y);
        display->print("OFF");
      }
      
    } else if (i == 2) {
      
      display->setCursor(10, y);
      display->print("Save & Exit");
    }
  }
  
  display->display();
}

void handleArcPointsSettingsInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (arcPointsState.selectedField > 0) {
      arcPointsState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (arcPointsState.selectedField < ARC_POINTS_TOTAL_ITEMS - 1) {
      arcPointsState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (arcPointsState.selectedField == 0) {
    
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      if (arcPointsState.selectedMode > ARC_POINTS_SOLID) {
        arcPointsState.selectedMode = static_cast<ArcPointsMode>(arcPointsState.selectedMode - 1);
        needRedraw = true;
      }
    }
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      if (arcPointsState.selectedMode < ARC_POINTS_OFF) {
        arcPointsState.selectedMode = static_cast<ArcPointsMode>(arcPointsState.selectedMode + 1);
        needRedraw = true;
      }
    }
  } else if (arcPointsState.selectedField == 1) {
    
    if (isButtonPressed(BTN_INDEX_LEFT) || isButtonPressed(BTN_INDEX_RIGHT)) {
      arcPointsState.tiltModeEnabled = !arcPointsState.tiltModeEnabled;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (arcPointsState.selectedField == 2) {
      
      PresetData preset;
      loadPresetData(presetManagerState.selectedPreset, preset);
      preset.arcPointState = arcPointsState.selectedMode;
      preset.tiltModeEnabled = arcPointsState.tiltModeEnabled;
      savePresetData(presetManagerState.selectedPreset, preset);
      
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  if (needRedraw) {
    drawArcPointsSettingsScreen();
  }
}
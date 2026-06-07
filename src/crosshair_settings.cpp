#include "crosshair_settings.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"


CrosshairSettingsState crosshairState = {CROSSHAIR_SIMPLE, 0}; 


const char* crosshairTypeNames[] = {
  "Simple",
  "T-Shape", 
  "Dual Circle",
  "Ring Dot"
};


#define CROSSHAIR_TOTAL_ITEMS 5

#define CROSSHAIR_ITEMS_PER_PAGE 4

void initCrosshairSettings() {
  
  
  crosshairState.selectedType = CROSSHAIR_SIMPLE;
  crosshairState.selectedField = 0; 
}

void drawCrosshairSettingsScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_CROSSHAIR_SETTINGS) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Crosshair Type");
  
  
  int scrollOffset = 0;
  if (crosshairState.selectedField >= CROSSHAIR_ITEMS_PER_PAGE) {
    
    scrollOffset = crosshairState.selectedField - CROSSHAIR_ITEMS_PER_PAGE + 1;
    
    if (scrollOffset > CROSSHAIR_TOTAL_ITEMS - CROSSHAIR_ITEMS_PER_PAGE) {
      scrollOffset = CROSSHAIR_TOTAL_ITEMS - CROSSHAIR_ITEMS_PER_PAGE;
    }
  }
  
  
  int startIdx = scrollOffset;
  int endIdx = min(startIdx + CROSSHAIR_ITEMS_PER_PAGE, CROSSHAIR_TOTAL_ITEMS);
  
  
  display->setTextSize(1);
  
  
  int startY = 16;
  int lineHeight = 10;
  
  for (int i = startIdx; i < endIdx; i++) {
    int y = startY + (i - startIdx) * lineHeight;
    
    
    if (i == crosshairState.selectedField) {
      display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    
    if (i < 4) {
      
      
      if (static_cast<CrosshairType>(i) == crosshairState.selectedType) {
        display->setCursor(8, y);
        display->print("*");  
      } else {
        display->setCursor(8, y);
        display->print(" ");  
      }
      
      display->setCursor(20, y);
      display->print(crosshairTypeNames[i]);
    } else {
      
      
      
      
      
      
      
      
      
      
      display->setCursor(10, y);
      display->print("Save & Exit");
    }
  }
  
  
  if (CROSSHAIR_TOTAL_ITEMS > CROSSHAIR_ITEMS_PER_PAGE) {
    display->setTextColor(WHITE);
    if (startIdx > 0) {
      display->setCursor(SCREEN_WIDTH - 10, 2);
      display->print("^");
    }
    if (endIdx < CROSSHAIR_TOTAL_ITEMS) {
      display->setCursor(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
      display->print("v");
    }
  }
  
  display->display();
}

void handleCrosshairSettingsInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (crosshairState.selectedField > 0) {
      crosshairState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (crosshairState.selectedField < CROSSHAIR_TOTAL_ITEMS - 1) {
      crosshairState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (crosshairState.selectedField == 4) {
      
      Serial.print("Crosshair Settings Saved: ");
      Serial.println(crosshairTypeNames[crosshairState.selectedType]);
      
      
      PresetData preset;
      loadPresetData(presetManagerState.selectedPreset, preset);
      preset.crosshairType = static_cast<int>(crosshairState.selectedType);
      savePresetData(presetManagerState.selectedPreset, preset);
      Serial.print("Updated crosshair type in active Preset ");
      Serial.println(presetManagerState.selectedPreset + 1);
      
      
      
      currentMode = MODE_MENU;  
      drawMenu();               
      return;
    } else {
      
      
      crosshairState.selectedType = static_cast<CrosshairType>(crosshairState.selectedField);
      needRedraw = true;
    }
  }
  
  if (needRedraw) {
    drawCrosshairSettingsScreen();
  }
}


void drawCrosshair(int centerX, int centerY, CrosshairType type) {
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  int size = 10;
  int gap = 4;

  switch (type) {
    case CROSSHAIR_SIMPLE:
      
      display->drawLine(centerX - size, centerY, centerX + size, centerY, WHITE);
      display->drawLine(centerX, centerY - size, centerX, centerY + size, WHITE);
      break;
    case CROSSHAIR_TSHAPE:
      
      display->drawLine(centerX - size, centerY, centerX - gap, centerY, WHITE);
      
      display->drawLine(centerX + gap, centerY, centerX + size, centerY, WHITE);
      
      display->drawLine(centerX, centerY + gap, centerX, centerY + size, WHITE);
      break;
    case CROSSHAIR_DUAL_CIRCLE:
      
      display->drawCircle(centerX, centerY, 8, WHITE);
      display->drawCircle(centerX, centerY, 16, WHITE);
      break;
    case CROSSHAIR_RING_DOT:
      
      display->drawCircle(centerX, centerY, 12, WHITE);
      display->drawPixel(centerX, centerY, WHITE);
      break;
  }
}
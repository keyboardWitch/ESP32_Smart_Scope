#include "bullet_velocity.h"
#include "menu_system.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "wind_compensation.h"
#include "crosshair_settings.h"
#include "arc_points_settings.h"
#include "sensor_calibration.h"
#include "ammo_count.h"
#include "preset_manager.h"
#include "range_finder.h"
#include "eeprom_manager.h"
#include "main_display.h"
#include "mount_angle_offset.h"

void* g_display = nullptr;

MenuState menuStack[MAX_SUBMENU_DEPTH];
int stackDepth = 0;
SystemMode currentMode = MODE_MAIN_DISPLAY;

MenuItem exitConfirmItems[] = {
  {"NO", returnToMainMenu, NULL, 0},
  {"YES", exitToMainDisplay, NULL, 0}
};

MenuItem mainMenuItems[] = {
  {"Wind Compensation", windCompensation, NULL, 0},
  {"Bullet Velocity", bulletVelocity, NULL, 0},
  {"Crosshair Settings", crosshairSettings, NULL, 0},
  {"Arc Points Settings", arcPointsSettings, NULL, 0},
  {"Sensor Calibration", sensorCalibration, NULL, 0},
  {"Ammo Count", ammoCount, NULL, 0},
  {"Preset Manager", presetManager, NULL, 0},
  {"Range Finder", rangeFinder, NULL, 0},
  {"Balistic Calib", ballisticCalibration, NULL, 0},
  {"Mount Angle", mountAngleOffset, NULL, 0},
  {"Exit", NULL, exitConfirmItems, 2}
};

void initMenuSystem() {
  stackDepth = 0;
  currentMode = MODE_MAIN_DISPLAY;
}

void checkStartupMode() {
  bool enterPressed = (digitalRead(BTN_ENTER) == LOW);
  
  if (enterPressed) {
    currentMode = MODE_MENU;
    Serial.println("Enter key pressed at startup - Entering Menu Mode");
    showMainMenu();
  } else {
    currentMode = MODE_MAIN_DISPLAY;
    Serial.println("Enter key not pressed at startup - Entering Main Display Mode");
    drawMainDisplay();
  }
  
  delay(500);
}

void initMenu(MenuItem* menu, int count) {
  if (stackDepth < MAX_SUBMENU_DEPTH) {
    menuStack[stackDepth].currentMenu = menu;
    menuStack[stackDepth].itemCount = count;
    menuStack[stackDepth].selectedIndex = 0;
    menuStack[stackDepth].scrollOffset = 0;
    stackDepth++;
  }
}

void showMainMenu() {
  stackDepth = 0;
  initMenu(mainMenuItems, 11);
  drawMenu();
}

void showExitConfirmMenu() {
  initMenu(exitConfirmItems, 2);
  drawMenu();
}

void drawMenu() {
  if (!g_display) return;
  
  if (currentMode != MODE_MENU) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  MenuState& current = menuStack[stackDepth - 1];
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  
  if (stackDepth == 1) {
    display->print("Main Menu");
  } else if (stackDepth == 2) {
    if (current.currentMenu == exitConfirmItems) {
      display->print("Exit Confirm");
    }
  }
  
  int startIdx = current.scrollOffset;
  int endIdx = min(startIdx + ITEMS_PER_PAGE, current.itemCount);
  
  display->setTextColor(WHITE);
  display->setTextSize(1);
  
  for (int i = startIdx; i < endIdx; i++) {
    int y = 16 + (i - startIdx) * 10;
    
    if (i == current.selectedIndex) {
      display->fillRect(2, y - 2, SCREEN_WIDTH - 4, 10, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    
    if (i == current.selectedIndex) {
      display->setCursor(4, y);
      display->print(">");
    }
    
    int textX = (i == current.selectedIndex) ? 14 : 10;
    display->setCursor(textX, y);
    display->print(current.currentMenu[i].text);
    
    if (current.currentMenu[i].submenu != NULL) {
      display->setCursor(SCREEN_WIDTH - 12, y);
      display->print(">>");
    }
  }
  
  if (current.itemCount > ITEMS_PER_PAGE) {
    display->setTextColor(WHITE);
    if (startIdx > 0) {
      display->setCursor(SCREEN_WIDTH - 10, 2);
      display->print("^");
    }
    if (endIdx < current.itemCount) {
      display->setCursor(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
      display->print("v");
    }
  }
  
  display->display();
}

void handleMenuNavigation() {
  MenuState& current = menuStack[stackDepth - 1];
  bool needRedraw = false;
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (current.selectedIndex > 0) {
      current.selectedIndex--;
      if (current.selectedIndex < current.scrollOffset) {
        current.scrollOffset = current.selectedIndex;
      }
      needRedraw = true;
    }
  }
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (current.selectedIndex < current.itemCount - 1) {
      current.selectedIndex++;
      if (current.selectedIndex >= current.scrollOffset + ITEMS_PER_PAGE) {
        current.scrollOffset = current.selectedIndex - ITEMS_PER_PAGE + 1;
      }
      needRedraw = true;
    }
  }
  
  if (isButtonPressed(BTN_INDEX_LEFT)) {
    if (stackDepth > 1) {
      goBack();
      needRedraw = true;
    }
  }
  
  if (isButtonPressed(BTN_INDEX_RIGHT)) {
    MenuItem& item = current.currentMenu[current.selectedIndex];
    if (item.submenu != NULL) {
      enterSubmenu(&item);
      needRedraw = true;
    } else if (item.callback != NULL) {
      executeCallback(&item);
      needRedraw = true;
    }
  }
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    MenuItem& item = current.currentMenu[current.selectedIndex];
    if (item.submenu != NULL) {
      enterSubmenu(&item);
      needRedraw = true;
    } else if (item.callback != NULL) {
      executeCallback(&item);
      needRedraw = true;
    }
  }
  
  if (needRedraw) {
    drawMenu();
  }
}

void enterSubmenu(MenuItem* item) {
  initMenu(item->submenu, item->itemCount);
  drawMenu();
}

void goBack() {
  if (stackDepth > 1) {
    stackDepth--;
    drawMenu();
  }
}

void executeCallback(MenuItem* item) {
  if (item->callback != NULL) {
    item->callback();
  }
}

void windCompensation() {
  Serial.println("Wind Compensation selected");
  currentMode = MODE_WIND_COMP;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  windState.speed = preset.windSpeed;
  windState.direction = preset.windDirection;
  
  windState.selectedField = WIND_FIELD_SPEED;
  drawWindCompensationScreen();
}

void bulletVelocity() {
  Serial.println("Bullet Velocity selected");
  currentMode = MODE_BULLET_VELOCITY;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  bulletVelocityState.velocity = static_cast<int>(preset.bulletVelocity);
  
  bulletVelocityState.selectedField = BULLET_VELOCITY_FIELD_VALUE;
  drawBulletVelocityScreen();
}

void crosshairSettings() {
  currentMode = MODE_CROSSHAIR_SETTINGS;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  crosshairState.selectedType = static_cast<CrosshairType>(preset.crosshairType);
  
  if (crosshairState.selectedType < CROSSHAIR_SIMPLE || crosshairState.selectedType > CROSSHAIR_RING_DOT) {
    crosshairState.selectedType = CROSSHAIR_SIMPLE;
  }
  
  crosshairState.selectedField = static_cast<int>(crosshairState.selectedType);
  if (crosshairState.selectedField < 0 || crosshairState.selectedField >= 4) {
    crosshairState.selectedField = 0;
  }
  
  drawCrosshairSettingsScreen();
}

void arcPointsSettings() {
  currentMode = MODE_ARC_POINTS_SETTINGS;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  arcPointsState.selectedMode = static_cast<ArcPointsMode>(preset.arcPointState);
  
  if (arcPointsState.selectedMode < ARC_POINTS_SOLID || arcPointsState.selectedMode > ARC_POINTS_OFF) {
    arcPointsState.selectedMode = ARC_POINTS_SOLID;
  }
  
  arcPointsState.selectedField = static_cast<int>(arcPointsState.selectedMode);
  if (arcPointsState.selectedField < 0 || arcPointsState.selectedField >= 3) {
    arcPointsState.selectedField = 0;
  }
  
  drawArcPointsSettingsScreen();
}

void sensorCalibration() {
  Serial.println("Sensor Calibration selected");
  currentMode = MODE_SENSOR_CALIBRATION;
  initSensorCalibration();
  drawSensorCalibrationScreen();
}

void ammoCount() {
  currentMode = MODE_AMMO_COUNT;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  ammoCountState.capacity = preset.ammoCount;
  
  ammoCountState.selectedField = AMMO_FIELD_CAPACITY;
  drawAmmoCountScreen();
}

void presetManager() {
  currentMode = MODE_PRESET_MANAGER;
  presetManagerState.selectedField = PRESET_FIELD_PRESET1;
  drawPresetManagerScreen();
}

void rangeFinder() {
  Serial.println("Range Finder selected");
  currentMode = MODE_RANGE_FINDER;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  rangeFinderState.selectedMode = static_cast<RangeFinderMode>(preset.rangeMode);
  
  if (rangeFinderState.selectedMode < RANGE_MODE_SINGLE || rangeFinderState.selectedMode > RANGE_MODE_CONTINUOUS) {
    rangeFinderState.selectedMode = RANGE_MODE_SINGLE;
  }
  
  rangeFinderState.selectedField = static_cast<int>(rangeFinderState.selectedMode);
  if (rangeFinderState.selectedField < 0 || rangeFinderState.selectedField >= 2) {
    rangeFinderState.selectedField = 0;
  }
  
  drawRangeFinderScreen();
}

void ballisticCalibration() {
  Serial.println("Ballistic Calibration selected");
  currentMode = MODE_BALLISTIC_CALIBRATION;
  initBallisticCalibration();
  drawBallisticCalibrationScreen();
}

void mountAngleSettings() {
  currentMode = MODE_MOUNT_ANGLE;
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);

}

void exitToMainDisplay() {
  currentMode = MODE_MAIN_DISPLAY;
  drawMainDisplay();
}

void returnToMainMenu() {
  if (stackDepth > 1) {
    stackDepth--;
  }
  currentMode = MODE_MENU;
  drawMenu();
}

#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include "config.h"


enum SystemMode {
  MODE_MAIN_DISPLAY,      
  MODE_MENU,              
  MODE_WIND_COMP,         
  MODE_BULLET_VELOCITY,   
  MODE_CROSSHAIR_SETTINGS,
  MODE_ARC_POINTS_SETTINGS,
  MODE_SENSOR_CALIBRATION, 
  MODE_AMMO_COUNT,        
  MODE_PRESET_MANAGER,    
  MODE_RANGE_FINDER,      
  MODE_BALLISTIC_CALIBRATION, 
  MODE_MOUNT_ANGLE        
};


struct MenuItem {
  const char* text;           
  void (*callback)();         
  struct MenuItem* submenu;   
  int itemCount;              
};


struct MenuState {
  MenuItem* currentMenu;      
  int itemCount;              
  int selectedIndex;          
  int scrollOffset;           
};


extern void* g_display;


extern MenuItem mainMenuItems[];
extern MenuItem sensorCalibrationItems[];
extern MenuItem exitConfirmItems[];


extern MenuState menuStack[MAX_SUBMENU_DEPTH];
extern int stackDepth;
extern SystemMode currentMode;


void initMenuSystem();
void checkStartupMode();
void showMainMenu();
void showSensorCalibrationMenu();
void showExitConfirmMenu();
void drawMenu();
void handleMenuNavigation();
void enterSubmenu(MenuItem* item);
void goBack();
void executeCallback(MenuItem* item);


void windCompensation();
void bulletVelocity();
void crosshairSettings();
void arcPointsSettings();
void sensorCalibration();
void gyroscopeCalibration();
void accelerometerCalibration();
void magnetometerCalibration();
void sensorCalibrationDone();
void ammoCount();
void presetManager();
void rangeFinder();
void ballisticCalibration();
void mountAngleOffset();  
void exitToMainDisplay();
void returnToMainMenu();

#endif 
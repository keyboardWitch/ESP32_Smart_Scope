#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include "system_init.h"
#include "button_handler.h"
#include "menu_system.h"
#include "wind_compensation.h"
#include "bullet_velocity.h"
#include "crosshair_settings.h"
#include "arc_points_settings.h"
#include "sensor_calibration.h"
#include "ammo_count.h"
#include "preset_manager.h"
#include "range_finder.h"
#include "ballistic_calc.h"
#include "eeprom_manager.h"
#include "main_display.h"
#include "ballistic_calibration.h"
#include "mount_angle_offset.h"

void setup() {
  initSystem();
}

void loop() {
  updateButtons();
  
  if (!g_display) {
    currentMode = MODE_MAIN_DISPLAY;
    delay(10);
    return;
  }
  
  if (currentMode == MODE_MENU) {
    handleMenuNavigation();
  } else if (currentMode == MODE_WIND_COMP) {
    handleWindCompensationInput();
  } else if (currentMode == MODE_BULLET_VELOCITY) {
    handleBulletVelocityInput();
  } else if (currentMode == MODE_CROSSHAIR_SETTINGS) {
    handleCrosshairSettingsInput();
  } else if (currentMode == MODE_ARC_POINTS_SETTINGS) {
    handleArcPointsSettingsInput();
  } else if (currentMode == MODE_SENSOR_CALIBRATION) {
    handleSensorCalibrationInput();
  } else if (currentMode == MODE_AMMO_COUNT) {
    handleAmmoCountInput();
  } else if (currentMode == MODE_PRESET_MANAGER) {
    handlePresetManagerInput();
  } else if (currentMode == MODE_RANGE_FINDER) {
    handleRangeFinderInput();
  } else if (currentMode == MODE_BALLISTIC_CALIBRATION) {
    handleBallisticCalibrationInput();
  } else if (currentMode == MODE_MOUNT_ANGLE) {
    handleMountAngleOffsetInput();
  } else if (currentMode == MODE_MAIN_DISPLAY) {
    handleMainDisplayInput();
    drawMainDisplay();
  }
  
  delay(10);
}
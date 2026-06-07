#ifndef MAIN_DISPLAY_H
#define MAIN_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "eeprom_manager.h"
#include "ammo_count.h"

extern PresetData currentActivePreset;
extern int currentAmmoCount;
extern int totalAmmoCapacity;
extern float cachedIMUPitchForBallistics;

void initMainDisplay();
void drawMainDisplay();
void handleMainDisplayInput();
bool detectShotAndDecrementAmmo();
void drawStabilityWaveform(Adafruit_SSD1306* display);
void updateStabilityCalculation();

#endif
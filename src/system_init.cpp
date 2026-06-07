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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MPU6050 mpu;

bool mpu6050Working = false;

MPU6050CalibrationData cachedMPUCalibrationData;

void showBootAnimation() {
  Adafruit_SSD1306* display = &::display;
  
  const int animationSteps = 20;
  const int centerX = SCREEN_WIDTH / 2;
  const int centerY = SCREEN_HEIGHT / 2;
  
  for (int step = 0; step <= animationSteps; step++) {
    display->clearDisplay();
    
    float progress = (float)step / animationSteps;
    
    if (progress > 0.2f) {
      int outerRadius = (int)(15 * progress);
      if (outerRadius > 15) outerRadius = 15;
      
      for (int angle = 0; angle < 360; angle += 10) {
        float rad = angle * PI / 180.0f;
        int x1 = centerX + (int)(outerRadius * cos(rad));
        int y1 = centerY + (int)(outerRadius * sin(rad));
        int x2 = centerX + (int)((outerRadius - 1) * cos(rad));
        int y2 = centerY + (int)((outerRadius - 1) * sin(rad));
        display->drawLine(x1, y1, x2, y2, WHITE);
      }
    }
    
    if (progress > 0.1f) {
      int crossSize = (int)(8 * progress);
      if (crossSize > 8) crossSize = 8;
      
      display->drawLine(centerX, centerY - crossSize, centerX, centerY + crossSize, WHITE);
      display->drawLine(centerX - crossSize, centerY, centerX + crossSize, centerY, WHITE);
    }
    
    if (progress > 0.3f) {
      display->drawPixel(centerX, centerY, WHITE);
    }
    
    if (progress > 0.5f) {
      display->setTextSize(1);
      display->setTextColor(WHITE);
      display->setCursor(centerX - 40, centerY + 20);
      
      if ((int)(progress * 10) % 2 == 0 || progress > 0.8f) {
        display->print("SmartScope v1.0");
      }
    }
    
    display->display();
    delay(50);
  }
  
  delay(300);
}

void initSystem() {
  Serial.begin(115200);
  Serial.println("System Initialization Starting...");
  
  Wire.begin(MPU6050_SDA, MPU6050_SCL);
  
  if (!initDisplay()) {
    Serial.println(F("Display initialization failed!"));
    for (;;);
  }
  
  showBootAnimation();
  
  mpu6050Working = initSensors();
  
  initEEPROM();
  
  initModules();
  
  loadSystemState();
  
  checkStartupMode();
  
  Serial.println("System Initialization Complete!");
}

bool initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  }
  
  display.setRotation(0);
  display.ssd1306_command(0xA0); 
  display.clearDisplay();
  display.display();
  
  g_display = &display;
  
  Serial.println("Display initialized successfully");
  return true;
}

bool initSensors() {
  Serial.println("Scanning I2C bus for devices...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(addr, HEX);
    }
  }
  
  if (!mpu.begin(MPU6050_ADDR)) {
    Serial.println("Failed to find MPU6050 chip");
    return false;
  } else {
    Serial.println("MPU6050 Found!");
    
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    return true;
  }
}

void initModules() {
  initButtons();
  initMenuSystem();
  initWindCompensation();
  initBulletVelocity();
  initCrosshairSettings();
  initArcPointsSettings();
  initSensorCalibration();
  initAmmoCount();
  initPresetManager();
  initRangeFinder();
  initMainDisplay();
  
  Serial.println("All modules initialized");
}

void loadSystemState() {
  int activePreset = loadActivePreset();
  Serial.print("Loading active preset: ");
  Serial.println(activePreset + 1);
  loadPresetToCurrentState(activePreset);
  presetManagerState.selectedPreset = activePreset;
  
  loadMPU6050Calibration(cachedMPUCalibrationData);
  Serial.print("MPU6050 calibration loaded - isCalibrated: ");
  Serial.println(cachedMPUCalibrationData.isCalibrated ? "true" : "false");
}
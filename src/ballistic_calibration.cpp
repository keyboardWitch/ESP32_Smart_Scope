#include "ballistic_calibration.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include <cmath>

// 外部变量声明
extern PresetManagerState presetManagerState;
extern Adafruit_MPU6050 mpu;
extern MPU6050CalibrationData cachedMPUCalibrationData;
extern void* g_display;
extern SystemMode currentMode;

BallisticCalibrationState ballisticState;

void initBallisticCalibration() {
  ballisticState.selectedField = BALLISTIC_FIELD_CALIBRATION;
  ballisticState.currentPoint = CALIB_POINT_5M;
  ballisticState.calibrationMode = CALIB_MODE_MENU;
  
  
  for (int i = 0; i < 3; i++) {
    ballisticState.pointRecorded[i] = false;
    ballisticState.tempPixelOffsets[i] = 0;
  }
  
  
  PresetData currentPreset;
  loadPresetData(presetManagerState.selectedPreset, currentPreset);
  
  
  ballisticState.result = currentPreset.ballisticCalib;
  
  
  if (!ballisticState.result.isCalibrated) {
    ballisticState.result.pixelOffset5m = 0;
    ballisticState.result.pixelOffset10m = 0;
    ballisticState.result.pixelOffset15m = 0;
    ballisticState.result.calibratedPoints = 0;
    ballisticState.result.checksum = 0;
  }
  
  ballisticState.calibrationComplete = false;
  
  // 初始化手动校准相关字段
  ballisticState.centerX = SCREEN_WIDTH / 2;
  ballisticState.centerY = SCREEN_HEIGHT / 2;
  ballisticState.impactX = ballisticState.centerX;
  ballisticState.impactY = ballisticState.centerY;
}


// 将像素Y坐标转换为下坠高度（米）
float pixelToDropHeight(int pixelY, float distance) {
  // 屏幕中心为参考点，Y轴向下为正
  int centerY = SCREEN_HEIGHT / 2;
  int deltaY = pixelY - centerY;
  
  // 使用配置中的转换因子
  float angleRadians = (float)deltaY / DROP_POINT_PX_PER_RADIAN;
  float dropHeight = distance * tan(angleRadians);
  
  return dropHeight;
}


bool calculateBallisticCoefficients() {
  // 这个函数现在不需要了，逻辑已移到handleCalibrationAdjustInput中
  return true;
}


void drawCalibrationAdjustScreen() {
  if (!g_display) return;
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  // 获取当前距离
  float distances[] = {5.0f, 10.0f, 15.0f};
  float currentDistance = distances[ballisticState.currentPoint];
  
  // 显示标题
  display->setTextSize(1);
  display->setTextColor(WHITE);
  char title[20];
  sprintf(title, "Cal %dm", (int)currentDistance);
  display->setCursor(5, 2);
  display->print(title);
  
  // 绘制十字准星（屏幕中心）
  int centerX = ballisticState.centerX;
  int centerY = ballisticState.centerY;
  
  // 十字准星
  display->drawLine(centerX - 5, centerY, centerX + 5, centerY, WHITE);
  display->drawLine(centerX, centerY - 5, centerX, centerY + 5, WHITE);
  
  // 绘制3x3的弹着点方块
  int impactX = ballisticState.impactX;
  int impactY = ballisticState.impactY;
  display->fillRect(impactX - 1, impactY - 1, 3, 3, WHITE);
  
  // 显示当前偏移值
  int deltaX = impactX - centerX;
  int deltaY = impactY - centerY;
  char offsetInfo[30];
  sprintf(offsetInfo, "X:%d Y:%d", deltaX, deltaY);
  display->setCursor(SCREEN_WIDTH - 40, 2);
  display->print(offsetInfo);
  
  display->display();
}


void handleCalibrationAdjustInput() {
  bool needRedraw = false;
  
  // 上下调整弹着点位置（只允许Y轴调整，X轴保持在中心）
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (ballisticState.impactY > 0) {
      ballisticState.impactY--;
      needRedraw = true;
    }
  }
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (ballisticState.impactY < SCREEN_HEIGHT - 1) {
      ballisticState.impactY++;
      needRedraw = true;
    }
  }
  
  // ENTER键记录当前点并切换到下一个距离
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    // 记录当前Y轴偏移值（相对于中心）
    ballisticState.tempPixelOffsets[ballisticState.currentPoint] = ballisticState.impactY - ballisticState.centerY;
    ballisticState.pointRecorded[ballisticState.currentPoint] = true;
    
    // 自动切换到下一个距离点
    if (ballisticState.currentPoint < CALIB_POINT_COUNT - 1) {
      ballisticState.currentPoint++;
      // 重置弹着点到中心位置
      ballisticState.impactX = ballisticState.centerX;
      ballisticState.impactY = ballisticState.centerY;
      needRedraw = true;
    } else {
      // 所有点都校准完成，保存数据并退出手动校准模式
      ballisticState.result.pixelOffset5m = ballisticState.tempPixelOffsets[0];
      ballisticState.result.pixelOffset10m = ballisticState.tempPixelOffsets[1];
      ballisticState.result.pixelOffset15m = ballisticState.tempPixelOffsets[2];
      ballisticState.result.isCalibrated = true;
      ballisticState.result.calibratedPoints = 3;
      
      // 计算校验和
      uint32_t sum = 0;
      sum += ballisticState.result.pixelOffset5m;
      sum += ballisticState.result.pixelOffset10m;
      sum += ballisticState.result.pixelOffset15m;
      ballisticState.result.checksum = (uint16_t)(sum & 0xFFFF);
      
      // 保存到EEPROM
      PresetData currentPreset;
      loadPresetData(presetManagerState.selectedPreset, currentPreset);
      currentPreset.ballisticCalib = ballisticState.result;
      savePresetData(presetManagerState.selectedPreset, currentPreset);
      
      ballisticState.calibrationComplete = true;
      ballisticState.calibrationMode = CALIB_MODE_MENU;
      // 返回到弹道校准菜单界面
      currentMode = MODE_BALLISTIC_CALIBRATION;
      drawBallisticCalibrationScreen();
      return;
    }
  }
  
  if (needRedraw) {
    drawCalibrationAdjustScreen();
  }
}


void drawBallisticCalibrationScreen() {
  if (!g_display) return;
  
  // 如果处于手动调整模式，绘制调整界面
  if (ballisticState.calibrationMode == CALIB_MODE_ADJUSTING) {
    drawCalibrationAdjustScreen();
    return;
  }
  
  if (currentMode != MODE_BALLISTIC_CALIBRATION) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  // 标题：Ball Calib（不可选中）
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Ball Calib");
  
  
  #define BALLISTIC_TOTAL_ITEMS 2
  #define BALLISTIC_ITEMS_PER_PAGE 2
  
  
  int startY = 18;
  int lineHeight = 10;
  
  // 校准完成后的特殊显示逻辑
  if (ballisticState.calibrationComplete) {
    // 显示校准完成信息
    char buffer[30];
    strcpy(buffer, "Calibration Done");
    display->setTextColor(WHITE);
    display->setCursor(10, startY);
    display->print(buffer);
    
    // 第二行：Exit 选项（始终高亮，因为这是唯一可操作的选项）
    int exitY = startY + lineHeight;
    display->fillRect(5, exitY - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
    display->setTextColor(BLACK);
    display->setCursor(10, exitY);
    display->print("Exit");
    
  } else {
    // 未校准完成的正常显示逻辑
    for (int i = 0; i < BALLISTIC_TOTAL_ITEMS; i++) {
      int y = startY + i * lineHeight;
      
      if (i == BALLISTIC_FIELD_CALIBRATION) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        display->setCursor(10, y);
        display->print("Calibration");  
      }
      else if (i == BALLISTIC_FIELD_EXIT) {
        
        if (i == ballisticState.selectedField) {
          display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
          display->setTextColor(BLACK);
        } else {
          display->setTextColor(WHITE);
        }
        display->setCursor(10, y);
        display->print("Exit");  
      }
    }
  }
  
  display->display();
}

void handleBallisticCalibrationInput() {
  if (!g_display) return;
  
  // 如果处于手动调整模式，处理调整输入
  if (ballisticState.calibrationMode == CALIB_MODE_ADJUSTING) {
    handleCalibrationAdjustInput();
    return;
  }
  
  bool needRedraw = false;
  
  // 校准完成后的特殊输入处理
  if (ballisticState.calibrationComplete) {
    // 在校准完成后，只允许Exit操作
    // 默认选中Exit（索引1）
    ballisticState.selectedField = BALLISTIC_FIELD_EXIT;
    
    // 任何按钮按下都返回菜单（简化操作）
    if (isButtonPressed(BTN_INDEX_ENTER) || 
        isButtonPressed(BTN_INDEX_LEFT) || 
        isButtonPressed(BTN_INDEX_RIGHT) ||
        isButtonPressed(BTN_INDEX_UP) ||
        isButtonPressed(BTN_INDEX_DOWN)) {
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  } else {
    // 未校准完成的正常输入处理
    
    if (isButtonPressed(BTN_INDEX_UP)) {
      if (ballisticState.selectedField > 0) {
        ballisticState.selectedField--;
        needRedraw = true;
      }
    }
    if (isButtonPressed(BTN_INDEX_DOWN)) {
      if (ballisticState.selectedField < BALLISTIC_FIELD_COUNT - 1) {
        ballisticState.selectedField++;
        needRedraw = true;
      }
    }
    
    
    if (isButtonPressed(BTN_INDEX_ENTER)) {
      if (ballisticState.selectedField == BALLISTIC_FIELD_CALIBRATION) {
        // ENTER键进入手动校准模式
        ballisticState.calibrationMode = CALIB_MODE_ADJUSTING;
        ballisticState.currentPoint = CALIB_POINT_5M; // 从5m开始
        ballisticState.impactX = ballisticState.centerX;
        ballisticState.impactY = ballisticState.centerY;
        drawCalibrationAdjustScreen();
        return;
      }
      else if (ballisticState.selectedField == BALLISTIC_FIELD_EXIT) {
        currentMode = MODE_MENU;
        drawMenu();
        return;
      }
    }
    
    // LEFT/RIGHT也应该作为确认操作
    if (isButtonPressed(BTN_INDEX_LEFT) || isButtonPressed(BTN_INDEX_RIGHT)) {
      if (ballisticState.selectedField == BALLISTIC_FIELD_CALIBRATION && !ballisticState.calibrationComplete) {
        // LEFT/RIGHT在校准字段上也进入手动校准模式
        ballisticState.calibrationMode = CALIB_MODE_ADJUSTING;
        ballisticState.currentPoint = CALIB_POINT_5M;
        ballisticState.impactX = ballisticState.centerX;
        ballisticState.impactY = ballisticState.centerY;
        drawCalibrationAdjustScreen();
        return;
      }
    }
  }
  
  if (needRedraw) {
    drawBallisticCalibrationScreen();
  }
}
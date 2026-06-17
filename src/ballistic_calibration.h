#ifndef BALLISTIC_CALIBRATION_H
#define BALLISTIC_CALIBRATION_H

#include <Arduino.h>
#include "config.h"


enum BallisticModelType {
  MODEL_TYPE_OFF = 0,             
  MODEL_TYPE_PHYSICS_DRAG = 1,    
  MODEL_TYPE_POLYNOMIAL_FIT = 2,  
  MODEL_TYPE_COUNT = 3            
};


typedef struct {
  float dragCoefficientK;    
} PhysicsDragModelParams;


typedef struct {
  float coefficientA;        
  float coefficientB;        
  float coefficientC;        
} PolynomialFitModelParams;


typedef struct {
  union {
    PhysicsDragModelParams physicsDrag;     
    PolynomialFitModelParams polynomialFit; 
  } modelParams;
  
  int pixelOffset5m;         // 5m距离的Y轴像素偏移
  int pixelOffset10m;        // 10m距离的Y轴像素偏移
  int pixelOffset15m;        // 15m距离的Y轴像素偏移
  int mountAngleOffset;      // 安装角度偏移
  bool isCalibrated;         // 是否已完成校准
  uint8_t calibratedPoints;  // 已校准的点数
  uint8_t modelType;         // 模型类型
  uint16_t checksum;         // 校验和
} BallisticCalibrationData;  


enum CalibrationPoint {
  CALIB_POINT_5M = 0,   
  CALIB_POINT_10M = 1,  
  CALIB_POINT_15M = 2,  
  CALIB_POINT_COUNT = 3 
};


enum BallisticField {
  BALLISTIC_FIELD_MODEL_TYPE = 0,    
  BALLISTIC_FIELD_CALIBRATION = 1,   
  BALLISTIC_FIELD_SHOOT_ADJUST = 2,  
  BALLISTIC_FIELD_EXIT = 3,          // 改为EXIT而不是SAVE
  BALLISTIC_FIELD_COUNT = 4          
};


// 新增校准模式枚举
enum CalibrationMode {
  CALIB_MODE_MENU = 0,        // 菜单模式
  CALIB_MODE_ADJUSTING = 1    // 调整模式
};


struct BallisticCalibrationState {
  int selectedField;                
  int currentPoint;                 
  int tempPixelOffsets[3];           // 临时存储各距离点的Y轴像素偏移值
  bool pointRecorded[3];             // 各点是否已记录
  BallisticCalibrationData result;  
  bool calibrationComplete;         
  // 新增字段用于手动调整模式
  int calibrationMode;              // 当前校准模式
  int impactX;                      // 弹着点X坐标（像素）
  int impactY;                      // 弹着点Y坐标（像素）
  int centerX;                      // 屏幕中心X坐标
  int centerY;                      // 屏幕中心Y坐标
};


extern BallisticCalibrationState ballisticState;


void initBallisticCalibration();
void drawBallisticCalibrationScreen();
void drawCalibrationAdjustScreen();
void handleBallisticCalibrationInput();
void handleCalibrationAdjustInput();

// 新增：根据距离直接计算Y轴像素偏移
int calculateDropPointPixelOffset(int distance, const BallisticCalibrationData& calibData);

#endif
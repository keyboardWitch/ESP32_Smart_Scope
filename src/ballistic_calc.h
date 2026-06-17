#ifndef BALLISTIC_CALC_H
#define BALLISTIC_CALC_H

#include <Arduino.h>
#include "config.h"
#include "ballistic_calibration.h"  


#define GRAVITY_ACCEL   9.80665f    
#define RAD_TO_MOA      3437.75f    
#define UPDATE_INTERVAL 100         


const float WIND_DRAG_COEFF = 0.5f;  


struct BallisticResult {
  float correctionPitch;     
  float correctionYaw;       
  float correctionMOA;       
  bool isValid;              
};


// 主要的弹道修正计算函数
BallisticResult calculateBallisticCorrection(float distance, float bulletVelocity, 
                                           float windSpeed, float windDirection,
                                           const BallisticCalibrationData& calibData);

// 计算水平风偏修正
float calculateHorizontalCorrection(float distance, float bulletVelocity, 
                                   float windSpeed, float windDirection);

// 基于像素偏移计算垂直修正角度
float calculateVerticalCorrectionByPixelOffset(float distance, const BallisticCalibrationData& calibData);

#endif 
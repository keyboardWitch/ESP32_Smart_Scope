#include "ballistic_calc.h"
#include "eeprom_manager.h"  
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <cmath>
#include "system_init.h"  
#include "ballistic_calibration.h"  


extern Adafruit_MPU6050 mpu;
extern MPU6050CalibrationData cachedMPUCalibrationData;

// 像素到弧度的转换系数，用于将像素偏移转换为角度
// 这个值需要根据实际显示系统的DPI和物理尺寸进行校准
#define DROP_POINT_PX_PER_RADIAN 100.0f

static float filteredIMUPitch = 0.0f;


float getIMUPitchForBallistics() {
  
  if (!mpu6050Working) {
    return 0.0f;
  }
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  
  if (isnan(a.acceleration.y) || isnan(a.acceleration.z)) {
    return 0.0f;
  }
  
  
  if (fabs(a.acceleration.y) < 0.01f && fabs(a.acceleration.z) < 0.01f) {
    return 0.0f;
  }
  

  float accelY = a.acceleration.y;  
  float accelZ = -a.acceleration.z;   
  if (fabs(accelZ) < 0.01f) {
    accelZ = (accelZ >= 0) ? 0.01f : -0.01f;
  }
  
  float rawPitch = atan2(accelY, accelZ);
  
  
  if (cachedMPUCalibrationData.isCalibrated) {
    
    float correctedAccelY = -a.acceleration.y - cachedMPUCalibrationData.accelOffsetY;
    float correctedAccelZ = a.acceleration.z - cachedMPUCalibrationData.accelOffsetZ;
    
    
    if (isnan(correctedAccelY) || isnan(correctedAccelZ)) {
      
    } else if (fabs(a.acceleration.y) < 0.01f && fabs(a.acceleration.z) < 0.01f) {
      
    } else {
      
      if (fabs(correctedAccelZ) < 0.01f) {
        correctedAccelZ = (correctedAccelZ >= 0) ? 0.01f : -0.01f;
      }
      rawPitch = atan2(correctedAccelY, correctedAccelZ);
    }
  }
  
  
  
  if (cachedMPUCalibrationData.isCalibrated) {
    rawPitch -= cachedMPUCalibrationData.pitchAngleOffset;
  }
  
  
  if (isnan(rawPitch)) {
    return 0.0f;
  }
  
  
  filteredIMUPitch = (1.0f - IMU_LPF_ALPHA) * filteredIMUPitch + IMU_LPF_ALPHA * rawPitch;
  
  return filteredIMUPitch;
}


// 新增：根据距离计算弹着点Y坐标（像素偏移）
int calculateDropPointPixelOffset(int distance, const BallisticCalibrationData& calibData) {
  if (!calibData.isCalibrated || calibData.calibratedPoints < 3) {
    // 未校准，返回中心位置（0偏移）
    return 0;
  }
  
  // 基于分段线性插值计算Y偏移
  if (distance <= 5) {
    // 0-5m: 从0到pixelOffset5m线性插值
    float ratio = (float)distance / 5.0f;
    return (int)(calibData.pixelOffset5m * ratio);
  }
  else if (distance <= 10) {
    // 5-10m: 从pixelOffset5m到pixelOffset10m线性插值
    float ratio = (float)(distance - 5) / 5.0f;
    int offset = calibData.pixelOffset5m + (int)((calibData.pixelOffset10m - calibData.pixelOffset5m) * ratio);
    return offset;
  }
  else if (distance <= 15) {
    // 10-15m: 从pixelOffset10m到pixelOffset15m线性插值
    float ratio = (float)(distance - 10) / 5.0f;
    int offset = calibData.pixelOffset10m + (int)((calibData.pixelOffset15m - calibData.pixelOffset10m) * ratio);
    return offset;
  }
  else {
    // 15m以上: 继续使用15m的偏移趋势进行外推
    // 简单线性外推：每增加1米，偏移增加(pixelOffset15m - pixelOffset10m)/5
    float extrapolationRatio = (float)(distance - 15) / 5.0f;
    int additionalOffset = (int)((calibData.pixelOffset15m - calibData.pixelOffset10m) * extrapolationRatio);
    return calibData.pixelOffset15m + additionalOffset;
  }
}


float calculateHorizontalCorrection(float distance, float bulletVelocity, 
                                   float windSpeed, float windDirection) {
  if (windSpeed <= 0.5f || bulletVelocity <= 0.0f || distance <= 0.0f) {
    return 0.0f; 
  }
  
  float flightTime = distance / bulletVelocity;
  float windDirRad = windDirection * PI / 180.0f;
  float crossWind = -windSpeed * sin(windDirRad);
  float horizontalOffset = crossWind * flightTime * WIND_DRAG_COEFF;
  return atan2(horizontalOffset, distance);
}


// 修改原有的弹道计算函数，现在主要返回像素偏移而不是角度
BallisticResult calculateBallisticCorrection(float distance, float bulletVelocity, 
                                           float windSpeed, float windDirection,
                                           const BallisticCalibrationData& calibData) {
  BallisticResult result;
  result.isValid = false;
  result.correctionPitch = 0.0f;
  result.correctionYaw = 0.0f;
  result.correctionMOA = 0.0f;
  
  if (distance <= 0.0f || distance > 50.0f) {
    return result; 
  }
  
  // 计算Y轴像素偏移（用于主显示）
  int pixelOffsetY = calculateDropPointPixelOffset((int)distance, calibData);
  
  // 转换为弧度角度（用于兼容现有接口）
  float angleRadians = (float)pixelOffsetY / DROP_POINT_PX_PER_RADIAN;
  result.correctionPitch = angleRadians;
  
  // 风偏计算保持不变
  if (windSpeed > 0.5f && bulletVelocity > 0.0f) {
    float flightTime = distance / bulletVelocity;
    float windDirRad = windDirection * PI / 180.0f;
    float crossWind = -windSpeed * sin(windDirRad);
    float horizontalOffset = crossWind * flightTime * WIND_DRAG_COEFF;
    result.correctionYaw = atan2(horizontalOffset, distance);
  }
  
  result.correctionMOA = result.correctionPitch * RAD_TO_MOA;
  result.isValid = true;
  
  return result;
}


// 保留原有函数以维持接口兼容性
float calculateVerticalCorrection(float distance, float bulletVelocity) {
  if (bulletVelocity <= 0.0f || distance <= 0.0f) {
    return 0.0f;
  }
  
  // 简化的默认弹道计算（未校准时使用）
  float ratio = GRAVITY_ACCEL * distance / (bulletVelocity * bulletVelocity);
  ratio = constrain(ratio, -1.0f, 1.0f);
  return 0.5f * asin(ratio);
}
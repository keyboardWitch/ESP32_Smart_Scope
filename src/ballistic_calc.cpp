#include "ballistic_calc.h"
#include "eeprom_manager.h"  
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <cmath>
#include "system_init.h"  
#include "ballistic_calibration.h"  


extern Adafruit_MPU6050 mpu;


extern MPU6050CalibrationData cachedMPUCalibrationData;




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


float calculateVerticalCorrectionWithDrag(float distance, float bulletVelocity, float dragCoefficientK) {
  if (bulletVelocity <= 0.0f || distance <= 0.0f || dragCoefficientK <= 0.0f) {
    return 0.0f;
  }
  
  
  float dropHeight = calcPhysicsDrop(distance, bulletVelocity, dragCoefficientK);
  
  
  
  return atan2(dropHeight, distance);
}


float calculateVerticalCorrectionWithPolynomial(float distance, const BallisticCalibrationData& calibData) {
  if (!calibData.isCalibrated || calibData.calibratedPoints < 2) {
    return 0.0f;
  }
  
  float dropHeight = 0.0f;
  
  
  if (distance <= 5.0f && calibData.calibratedPoints >= 1) {
    
    
    float dropHeight5m = 5.0f * tan(calibData.pitch5m);
    
    
    
    dropHeight5m = -dropHeight5m;
    
    
    if (dropHeight5m < 0.0f) {
      dropHeight5m = 0.0f;
    }
    
    
    dropHeight = (distance / 5.0f) * dropHeight5m;
    
    

  } else {
    
    
    
    dropHeight = calibData.modelParams.polynomialFit.coefficientA * distance * distance + 
                calibData.modelParams.polynomialFit.coefficientB * distance + 
                calibData.modelParams.polynomialFit.coefficientC;
    
    
    if (dropHeight < 0.0f) {
      dropHeight = 0.0f;
    }
  }
  
  
  return atan2(dropHeight, distance);
}


float calculateVerticalCorrectionByModel(float distance, float bulletVelocity, 
                                       const BallisticCalibrationData& calibData) {
  if (calibData.isCalibrated && calibData.calibratedPoints >= 3) {
    if (calibData.modelType == MODEL_TYPE_PHYSICS_DRAG) {
      
      return calculateVerticalCorrectionWithDrag(distance, bulletVelocity, 
                                               calibData.modelParams.physicsDrag.dragCoefficientK);
    } else if (calibData.modelType == MODEL_TYPE_POLYNOMIAL_FIT) {
      
      return calculateVerticalCorrectionWithPolynomial(distance, calibData);
    }
    
  }
  
  
  return calculateVerticalCorrection(distance, bulletVelocity);
}


float calculateVerticalCorrection(float distance, float bulletVelocity) {
  if (bulletVelocity <= 0.0f || distance <= 0.0f) {
    return 0.0f;
  }
  
  
  float ratio = GRAVITY_ACCEL * distance / (bulletVelocity * bulletVelocity);
  
  
  ratio = constrain(ratio, -1.0f, 1.0f);
  
  
  return 0.5f * asin(ratio);
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
  
  
  float verticalCorrection = 0.0f;
  
  float horizontalCorrection = 0.0f;
  
  
  verticalCorrection = calculateVerticalCorrectionByModel(distance, bulletVelocity, calibData);
  
  
  if (calibData.isCalibrated) {
    verticalCorrection += calibData.mountAngleOffset;
  } else {
    verticalCorrection += MOUNT_ANGLE_OFFSET;
  }
  
  
  horizontalCorrection = calculateHorizontalCorrection(distance, bulletVelocity, 
                                                      windSpeed, windDirection);
  
  
  if (isnan(verticalCorrection) || isinf(verticalCorrection) || 
      isnan(horizontalCorrection) || isinf(horizontalCorrection)) {
    return result; 
  }
  
  result.correctionPitch = verticalCorrection;
  result.correctionYaw = horizontalCorrection;
  
  result.correctionMOA = verticalCorrection * RAD_TO_MOA;
  result.isValid = true;
  
  return result;
}
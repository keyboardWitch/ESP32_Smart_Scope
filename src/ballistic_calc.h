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


BallisticResult calculateBallisticCorrection(float distance, float bulletVelocity, 
                                           float windSpeed, float windDirection,
                                           const BallisticCalibrationData& calibData);


float calculateVerticalCorrection(float distance, float bulletVelocity);


float calculateVerticalCorrectionByModel(float distance, float bulletVelocity, 
                                       const BallisticCalibrationData& calibData);

#endif 
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
  
  float pitch5m;             
  float pitch10m;            
  float pitch15m;            
  int mountAngleOffset;      
  bool isCalibrated;         
  uint8_t calibratedPoints;  
  uint8_t modelType;         
  uint16_t checksum;         
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
  BALLISTIC_FIELD_SAVE = 3,          
  BALLISTIC_FIELD_COUNT = 4          
};


struct BallisticCalibrationState {
  int selectedField;                
  int currentPoint;                 
  float recordedPitch[3];           
  bool pointRecorded[3];            
  BallisticCalibrationData result;  
  bool calibrationComplete;         
};


extern BallisticCalibrationState ballisticState;


void initBallisticCalibration();
void drawBallisticCalibrationScreen();
void handleBallisticCalibrationInput();
bool calculateBallisticCoefficients();  
float getIMUPitch(); 


float calibrateKByPitch(float pitch5, float pitch10, float pitch15, float bulletVelocity);


bool calibratePolynomialCoefficients(float pitch5, float pitch10, float pitch15, PolynomialFitModelParams& params);


float calcPhysicsDrop(float distance, float bulletVelocity, float dragCoefficientK);

#endif 
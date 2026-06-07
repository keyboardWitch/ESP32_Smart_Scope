#ifndef RANGE_FINDER_H
#define RANGE_FINDER_H

#include <Arduino.h>
#include "config.h"


enum RangeFinderMode {
  RANGE_MODE_SINGLE = 0,      
  RANGE_MODE_CONTINUOUS       
};


enum RangeFinderField {
  RANGE_FIELD_MODE_SINGLE = 0,    
  RANGE_FIELD_MODE_CONTINUOUS = 1,
  RANGE_FIELD_SAVE = 2,           
  RANGE_FIELD_COUNT = 3           
};


struct RangeFinderState {
  float distance;             
  RangeFinderMode selectedMode; 
  int selectedField;          
};


extern RangeFinderState rangeFinderState;


void initRangeFinder();
void drawRangeFinderScreen();
void handleRangeFinderInput();
float readTOFDistance(); 

#endif 
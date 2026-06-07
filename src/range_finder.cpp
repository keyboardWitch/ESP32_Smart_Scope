#include "range_finder.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"
#include "eeprom_manager.h"
#include "preset_manager.h"
#include "config.h"


const char* rangeModeNames[] = {
  "single",
  "continuous"
};


RangeFinderState rangeFinderState = {0.00, RANGE_MODE_SINGLE, RANGE_FIELD_MODE_SINGLE};


static HardwareSerial tofSerial(1);

#if TOF_ACTIVE == 50

static uint8_t syncBuffer[22];
static int syncBufferIndex = 0;
#elif TOF_ACTIVE == 600

static uint8_t parseBuffer[64];
static int parseIndex = 0;

const uint8_t MEASURE_CMD[] = {0xAE, 0xA7, 0x04, 0x00, 0x0E, 0x12, 0xBC, 0xBE};

const uint8_t STOP_CMD[] = {0xAE, 0xA7, 0x04, 0x00, 0x0F, 0x13, 0xBC, 0xBE};
#endif

void initRangeFinder() {
  
  tofSerial.begin(UART_BAUDRATE, SERIAL_8N1, UART_RX, UART_TX);
  
  
  PresetData preset;
  loadPresetData(presetManagerState.selectedPreset, preset);
  rangeFinderState.selectedMode = static_cast<RangeFinderMode>(preset.rangeMode);
  rangeFinderState.selectedField = RANGE_FIELD_MODE_SINGLE;
  
#if TOF_ACTIVE == 600
  
  tofSerial.write(MEASURE_CMD, sizeof(MEASURE_CMD));
  tofSerial.flush();
#endif
}

#if TOF_ACTIVE == 50
float readTOFDistance() {
  
  while (tofSerial.available() > 0 && syncBufferIndex < 22) {
    syncBuffer[syncBufferIndex] = tofSerial.read();
    syncBufferIndex++;
  }
  
  float latestValidDistance = -1.0f;
  bool foundValidFrame = false;
  
  
  for (int i = 0; i <= syncBufferIndex - 11; i++) {
    
    if (syncBuffer[i] == 0xDF) {
      
      if (syncBuffer[i+1] == 0x32 && syncBuffer[i+2] == 0x00 && syncBuffer[i+3] == 0x40) {
        
        if (syncBuffer[i+5] == 0x04) {
          
          uint16_t checksum = 0;
          for (int j = 0; j < 10; j++) {
            checksum += syncBuffer[i + j];
          }
          checksum &= 0xFF;
          
          
          if (checksum == syncBuffer[i+10]) {
            
            uint16_t distance_mm = (syncBuffer[i+7] << 8) | syncBuffer[i+6];
            float distance_m = distance_mm / 1000.0f;
            
            
            if (distance_m >= TOF_MIN_DISTANCE && distance_m <= TOF_MAX_DISTANCE) {
              latestValidDistance = distance_m;
              foundValidFrame = true;
              
              
              int bytesToKeep = syncBufferIndex - (i + 11);
              if (bytesToKeep > 0) {
                memmove(syncBuffer, &syncBuffer[i + 11], bytesToKeep);
                syncBufferIndex = bytesToKeep;
              } else {
                syncBufferIndex = 0;
              }
              
              
              i = -1;
              continue;
            }
          }
        }
      }
    }
  }
  
  
  if (!foundValidFrame && syncBufferIndex >= 22) {
    syncBufferIndex = 0;
  }
  
  
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 1000) {
    if (syncBufferIndex > 0) {
      Serial.print("TOF Sync Buffer: ");
      Serial.println(syncBufferIndex);
    }
    lastDebugTime = millis();
  }
  
  if (foundValidFrame) {
    Serial.print("TOF: Valid distance=");
    Serial.print(latestValidDistance);
    Serial.println(" m");
    return latestValidDistance;
  }
  
  return -1.0f;
}
#elif TOF_ACTIVE == 600

int16_t bytesToSignedInt16(uint8_t high, uint8_t low) {
    return (static_cast<int16_t>(high) << 8) | low;
}


float parseMMSGData(const uint8_t* mmsgData) {
    
    int16_t straightDistance = bytesToSignedInt16(mmsgData[2], mmsgData[3]);
    float distance_m = straightDistance * 0.1f; 
    
    
    if (distance_m >= TOF_MIN_DISTANCE && distance_m <= TOF_MAX_DISTANCE) {
        return distance_m;
    }
    
    return -1.0f; 
}

float readTOFDistance() {
    
    while (tofSerial.available() > 0 && parseIndex < 64) {
        uint8_t b = tofSerial.read();
        
        
        if (parseIndex == 0 && b == 0xAE) {
            parseBuffer[parseIndex++] = b;
        } else if (parseIndex == 1 && b == 0xA7) {
            parseBuffer[parseIndex++] = b;
        } else if (parseIndex >= 2) {
            parseBuffer[parseIndex++] = b;
            
            
            
            if (parseIndex == 27) {
                if (parseBuffer[0] == 0xAE && parseBuffer[1] == 0xA7 && 
                    parseBuffer[4] == 0x85 && 
                    parseBuffer[25] == 0xBC && parseBuffer[26] == 0xBE) {
                    
                    
                    float distance = parseMMSGData(&parseBuffer[5]);
                    if (distance > 0) {
                        Serial.print("TOF: Valid distance=");
                        Serial.print(distance);
                        Serial.println(" m");
                        return distance;
                    }
                }
                parseIndex = 0; 
            }
            
            
            if (parseIndex >= 64) {
                parseIndex = 0;
            }
        } else {
            
            parseIndex = 0;
        }
    }
    
    return -1.0f;
}
#endif

void drawRangeFinderScreen() {
  if (!g_display) return;
  
  
  if (currentMode != MODE_RANGE_FINDER) {
    return;
  }
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRect(0, 0, SCREEN_WIDTH, 12, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 2);
  display->print("Range Finder");
  
  
  if (rangeFinderState.distance > 0) {
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(10, 14);
    display->print("Dist: ");
    display->print(rangeFinderState.distance, 2);
    display->print(" m");
  } else {
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(10, 14);
    display->print("No signal");
  }
  
  
  #define RANGE_FINDER_TOTAL_ITEMS 3
  
  #define RANGE_FINDER_ITEMS_PER_PAGE 4
  
  
  int scrollOffset = 0;
  if (rangeFinderState.selectedField >= RANGE_FINDER_ITEMS_PER_PAGE) {
    
    scrollOffset = rangeFinderState.selectedField - RANGE_FINDER_ITEMS_PER_PAGE + 1;
    
    if (scrollOffset > RANGE_FINDER_TOTAL_ITEMS - RANGE_FINDER_ITEMS_PER_PAGE) {
      scrollOffset = RANGE_FINDER_TOTAL_ITEMS - RANGE_FINDER_ITEMS_PER_PAGE;
    }
  }
  
  
  int startIdx = scrollOffset;
  int endIdx = min(startIdx + RANGE_FINDER_ITEMS_PER_PAGE, RANGE_FINDER_TOTAL_ITEMS);
  
  
  display->setTextSize(1);
  
  
  int startY = 26; 
  int lineHeight = 10;
  
  for (int i = startIdx; i < endIdx; i++) {
    int y = startY + (i - startIdx) * lineHeight;
    
    
    if (i == rangeFinderState.selectedField) {
      display->fillRect(5, y - 2, SCREEN_WIDTH - 10, lineHeight - 1, WHITE);
      display->setTextColor(BLACK);
    } else {
      display->setTextColor(WHITE);
    }
    
    if (i < 2) {
      
      
      if (static_cast<RangeFinderMode>(i) == rangeFinderState.selectedMode) {
        display->setCursor(8, y);
        display->print("*");
      } else {
        display->setCursor(8, y);
        display->print(" ");
      }
      
      
      display->setCursor(20, y);
      display->print("mode: ");
      display->print(rangeModeNames[i]);
    } else {
      display->setCursor(10, y);
      display->print("Save & Exit");
    }
  }
  
  
  if (RANGE_FINDER_TOTAL_ITEMS > RANGE_FINDER_ITEMS_PER_PAGE) {
    display->setTextColor(WHITE);
    if (startIdx > 0) {
      display->setCursor(SCREEN_WIDTH - 10, 2); 
      display->print("^");
    }
    if (endIdx < RANGE_FINDER_TOTAL_ITEMS) {
      display->setCursor(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
      display->print("v");
    }
  }
  
  display->display();
}











    




    








        







            






                    











            









    



void handleRangeFinderInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (rangeFinderState.selectedField > 0) {
      rangeFinderState.selectedField--;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (rangeFinderState.selectedField < RANGE_FINDER_TOTAL_ITEMS - 1) {
      rangeFinderState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (rangeFinderState.selectedField < 2) {
      
      rangeFinderState.selectedMode = static_cast<RangeFinderMode>(rangeFinderState.selectedField);
      
#if TOF_ACTIVE == 600
      
      if (rangeFinderState.selectedMode == RANGE_MODE_CONTINUOUS) {
        
        tofSerial.write(MEASURE_CMD, sizeof(MEASURE_CMD));
        tofSerial.flush();
      } else {
        
        tofSerial.write(MEASURE_CMD, sizeof(MEASURE_CMD));
        tofSerial.flush();
      }
#endif
      
      needRedraw = true;
    } else if (rangeFinderState.selectedField == 2) {
      
      PresetData preset;
      loadPresetData(presetManagerState.selectedPreset, preset);
      preset.rangeMode = rangeFinderState.selectedMode;
      savePresetData(presetManagerState.selectedPreset, preset);
      
      
      currentMode = MODE_MENU;
      drawMenu();
      return; 
    }
  }
  
  if (needRedraw) {
    drawRangeFinderScreen();
  }
}

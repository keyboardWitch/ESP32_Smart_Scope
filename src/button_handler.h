#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "config.h"


#define BTN_INDEX_UP     0
#define BTN_INDEX_DOWN   1
#define BTN_INDEX_LEFT   2
#define BTN_INDEX_RIGHT  3
#define BTN_INDEX_ENTER  4


struct ButtonState {
  int pin;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
  bool pressed;
};


extern ButtonState buttons[5];


void initButtons();
void updateButtons();
bool isButtonPressed(int index);
bool readButton(int index);

#endif 
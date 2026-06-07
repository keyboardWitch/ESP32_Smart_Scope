#include "button_handler.h"

ButtonState buttons[5] = {
  {BTN_UP, HIGH, HIGH, 0, false},
  {BTN_DOWN, HIGH, HIGH, 0, false},
  {BTN_LEFT, HIGH, HIGH, 0, false},
  {BTN_RIGHT, HIGH, HIGH, 0, false},
  {BTN_ENTER, HIGH, HIGH, 0, false}
};

void initButtons() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  
  for (int i = 0; i < 5; i++) {
    buttons[i].lastState = digitalRead(buttons[i].pin);
    buttons[i].currentState = buttons[i].lastState;
    buttons[i].pressed = false;
  }
}

bool readButton(int index) {
  if (index < 0 || index >= 5) return false;
  
  ButtonState& btn = buttons[index];
  bool reading = digitalRead(btn.pin);
  
  if (reading != btn.lastState) {
    btn.lastDebounceTime = millis();
  }
  
  if ((millis() - btn.lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != btn.currentState) {
      btn.currentState = reading;
      
      if (btn.currentState == LOW) {
        btn.pressed = true;
        return true;
      }
    }
  }
  
  btn.lastState = reading;
  return false;
}

void updateButtons() {
  for (int i = 0; i < 5; i++) {
    readButton(i);
  }
}

bool isButtonPressed(int index) {
  if (index < 0 || index >= 5) return false;
  
  bool pressed = buttons[index].pressed;
  if (pressed) {
    buttons[index].pressed = false;
  }
  return pressed;
}
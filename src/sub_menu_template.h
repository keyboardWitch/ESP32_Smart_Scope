#ifndef SUB_MENU_TEMPLATE_H
#define SUB_MENU_TEMPLATE_H

#include <Arduino.h>
#include "config.h"
#include "menu_system.h"


enum TemplateField {
  TEMPLATE_FIELD_PARAM1 = 0,
  TEMPLATE_FIELD_PARAM2 = 1,
  TEMPLATE_FIELD_SAVE = 2,
  TEMPLATE_FIELD_COUNT = 3
};


struct TemplateState {
  float param1;
  int param2;
  int selectedField;
};


extern TemplateState templateState;


void initTemplateSubMenu();
void drawTemplateSubMenuScreen();
void handleTemplateSubMenuInput();

#endif 
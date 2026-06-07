#include "sub_menu_template.h"
#include <Adafruit_SSD1306.h>
#include "button_handler.h"
#include "menu_system.h"


TemplateState templateState = {0, 0, 0};

void initTemplateSubMenu() {
  
  templateState.param1 = 0;
  templateState.param2 = 0;
  templateState.selectedField = 0;
}

void drawTemplateSubMenuScreen() {
  if (!g_display) return;
  
  Adafruit_SSD1306* display = static_cast<Adafruit_SSD1306*>(g_display);
  display->clearDisplay();
  
  
  display->fillRoundRect(0, 0, SCREEN_WIDTH, 16, 4, WHITE);
  display->setTextSize(1);
  display->setTextColor(BLACK);
  display->setCursor(5, 4);
  display->print("Template SubMenu");
  
  
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(10, 35);
  display->print("Param1: ");
  
  if (templateState.selectedField == TEMPLATE_FIELD_PARAM1) {
    display->fillRect(60, 32, 60, 12, WHITE);
    display->setTextColor(BLACK);
  }
  display->setCursor(65, 35);
  char param1Str[10];
  dtostrf(templateState.param1, 4, 1, param1Str);
  display->print(param1Str);
  
  
  display->setTextColor(WHITE);
  display->setCursor(10, 55);
  display->print("Param2: ");
  
  if (templateState.selectedField == TEMPLATE_FIELD_PARAM2) {
    display->fillRect(60, 52, 60, 12, WHITE);
    display->setTextColor(BLACK);
  }
  display->setCursor(65, 55);
  display->print(templateState.param2);
  
  
  display->setTextColor(WHITE);
  display->setCursor(10, 75);
  display->print("Save Settings");
  
  if (templateState.selectedField == TEMPLATE_FIELD_SAVE) {
    display->fillRect(8, 72, SCREEN_WIDTH - 16, 12, WHITE);
    display->setTextColor(BLACK);
  }
  
  
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(5, 95);
  display->print("UP/DOWN: Select");
  
  display->setCursor(5, 110);
  display->print("LEFT/RIGHT: Adjust");
  
  display->display();
}

void handleTemplateSubMenuInput() {
  if (!g_display) return;
  
  bool needRedraw = false;
  
  
  if (isButtonPressed(BTN_INDEX_UP)) {
    if (templateState.selectedField > 0) {
      templateState.selectedField--;
      needRedraw = true;
    }
  }
  
  if (isButtonPressed(BTN_INDEX_DOWN)) {
    if (templateState.selectedField < TEMPLATE_FIELD_COUNT - 1) {
      templateState.selectedField++;
      needRedraw = true;
    }
  }
  
  
  if (templateState.selectedField == TEMPLATE_FIELD_PARAM1) {
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      templateState.param1 -= 0.1;  
      if (templateState.param1 < 0.0) templateState.param1 = 0.0;
      needRedraw = true;
    }
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      templateState.param1 += 0.1;
      
      needRedraw = true;
    }
  }
  else if (templateState.selectedField == TEMPLATE_FIELD_PARAM2) {
    if (isButtonPressed(BTN_INDEX_LEFT)) {
      templateState.param2 -= 1;
      if (templateState.param2 < 0) templateState.param2 = 0;
      needRedraw = true;
    }
    if (isButtonPressed(BTN_INDEX_RIGHT)) {
      templateState.param2 += 1;
      
      needRedraw = true;
    }
  }
  
  
  if (isButtonPressed(BTN_INDEX_ENTER)) {
    if (templateState.selectedField == TEMPLATE_FIELD_SAVE) {
      Serial.println("Template SubMenu Saved");
      
      
      currentMode = MODE_MENU;
      drawMenu();
      return;
    }
  }
  
  if (needRedraw) {
    drawTemplateSubMenuScreen();
  }
}
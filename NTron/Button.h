#ifndef BUTTON_H
#define BUTTON_H

#include "Constants.h"

typedef struct Button_ {
  int pin;  //The pin the button is connected to
  int isPressed; //If the button is pressed (as of when Update() was last called)
  unsigned long stateChangeTime; //When the last state change happened (for debouncing)
  bool wasPressed;
} Button;

Button CreateButton(uint8_t pin) {
  Button btn = { pin, HIGH, 0, false };
  pinMode(pin, INPUT_PULLUP);
  return btn;
}

void Update(Button &btn) {
  //Ignore any state change within the debounce time window
  if(millis() - btn.stateChangeTime > DEBOUNCE_MS) {
    bool newState = digitalRead(btn.pin);
    if(btn.isPressed != newState){ //Button has changed state
      if(btn.isPressed == HIGH && newState == LOW){ //Button down event
        btn.wasPressed = true;
      } //Nothing special to do for button up event
      btn.isPressed = newState;
      btn.stateChangeTime = millis();
    }
  }
}

#endif

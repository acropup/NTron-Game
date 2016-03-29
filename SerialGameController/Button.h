#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#define DEBOUNCE_MS 40

typedef struct Button_ {
  int pin;  //The pin the button is connected to
  bool isPressed; //If the button is pressed (as of when Update() was last called)
  unsigned long stateChangeTime; //When the last state change happened (for debouncing)
  bool wasPressed;
} Button;

Button CreateButton(int pin) {
  Button btn = { pin, HIGH, 0, false };
  pinMode(pin, INPUT_PULLUP);
  return btn;
}

/* Called to check each button as often as possible,
   so as to not miss any short button presses. On
   a Button Down Event, sets btn.wasPressed to true.
   It is up to external code to set it back to false,
   after it has been read and handled. */
void Update(Button &btn) {
  //Ignore any state change within the debounce time window
  if (millis() - btn.stateChangeTime > DEBOUNCE_MS) {
    bool newState = digitalRead(btn.pin);
    if (btn.isPressed == newState) { //Button has changed state (because newState is inverted due to INPUT_PULLUP)
      if (btn.isPressed == false && newState == LOW) { //Button down event
        btn.wasPressed = true;
      } //Nothing special to do for button up event
      btn.isPressed = !newState; //newState is inverted due to INPUT_PULLUP
      btn.stateChangeTime = millis();
    }
  }
}

#endif

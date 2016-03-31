#include "Button.h"

#define SER Serial1
#define LED_PIN 13
#define NUM_BUTTONS 8
//Offset for each player in the button array
#define P1 4
#define P2 0
//Offset for each button in the button array
#define B_LEFT   3
#define B_RIGHT  2
#define B_FENCE  1
#define B_ROCKET 0
Button buttons[NUM_BUTTONS];

void setup() {
  //Assign Pin numbers for buttons
  buttons[7] = CreateButton(2); //P1 Left
  buttons[6] = CreateButton(3); //P1 Right
  buttons[5] = CreateButton(4); //P1 Fence
  buttons[4] = CreateButton(5); //P1 Rocket
  buttons[3] = CreateButton(6); //P2 Left
  buttons[2] = CreateButton(7); //P2 Right
  buttons[1] = CreateButton(8); //P2 Fence
  buttons[0] = CreateButton(9); //P2 Rocket
  //The byte of button info sent by Serial is mapped according to array index, like 76543210

  SER.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}

/* Returns the button state for player P1 or P2 (see #defines)
   in the format 0bLRFr0000 for P1, and 0b0000LRFr for P2.
   L = Left turn, R = Right turn. Only one of these bits are
   ever set.
   F = Fence button, r = Rocket button. Fence stays on for as
   long as player holds it down, r is only set when the button
   is first pressed.
   The wasPressed states of these buttons are cleared in accordance
   with their intended behaviour. */
uint8_t getButtonStateForPlayer(int playerOffset) {
  uint8_t result = 0;
  Button& bL      = buttons[playerOffset + B_LEFT];
  Button& bR      = buttons[playerOffset + B_RIGHT];
  Button& bFence  = buttons[playerOffset + B_FENCE];
  Button& bRocket = buttons[playerOffset + B_ROCKET];
  //Only Right or Left bit can be set
  if (bL.wasPressed && (!bR.wasPressed || bL.stateChangeTime < bR.stateChangeTime)) {
    result |= _BV(B_LEFT);
    bL.wasPressed = false;
  }
  else if (bR.wasPressed) {
    result |= _BV(B_RIGHT);
    bR.wasPressed = false;
  }
  //Fence button remains in effect until it is released
  if (bFence.wasPressed || bFence.isPressed) {
    result |= _BV(B_FENCE);
    bFence.wasPressed = false;
  }
  //Rocket button is active on initial press
  if (bRocket.wasPressed) {
    result |= _BV(B_ROCKET);
    bRocket.wasPressed = false;
  }
  return result;
}

void pollButtonStates() {
  uint8_t id = NUM_BUTTONS;
  while (id--) {
    Update(buttons[id]);
  }
}

void loop() {
  pollButtonStates();
  if (SER.available()) {
    digitalWrite(LED_PIN, HIGH);
    //Send Button state for both players
    uint8_t state = (getButtonStateForPlayer(P1) << P1) | (getButtonStateForPlayer(P2) << P2);
    SER.write(state);
    //Empty the Serial input buffer (we treat any character as a query)
    while (SER.read() != -1) {}
    digitalWrite(LED_PIN, LOW);
  }
}


/*
void clearWasPressed() {
  uint8_t id = NUM_BUTTONS;
  while (id--) {
    buttons[id].wasPressed = false;
  }
}

#if NUM_BUTTONS > 8
#error This code does not support NUM_BUTTONS > 8
#endif

// Returns a byte containing the isPressed state of
// all NUM_BUTTONS (<= 8) buttons, where the nth
// button is represented by the nth bit.
uint8_t getButtonIsPressedState() {
  uint8_t allStates = 0;
  uint8_t id = NUM_BUTTONS;
  while (id--) {
    //Set bit in allStates if button is pressed
    if (buttons[id].isPressed)
      allStates |= _BV(id);
  }
  return allStates;
}

// Returns a byte containing the wasPressed state of
// all NUM_BUTTONS (<= 8) buttons, where the nth
// button is represented by the nth bit.
uint8_t getButtonWasPressedState() {
  uint8_t allStates = 0;
  uint8_t id = NUM_BUTTONS;
  while (id--) {
    //Set bit in allStates if button was pressed
    if (buttons[id].wasPressed)
      allStates |= _BV(id);
  }
  return allStates;
}
*/

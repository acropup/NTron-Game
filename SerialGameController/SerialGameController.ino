#include "Button.h"

/* SerialGameController.h
   Monitors up to 8 pins for button presses, and sends the button states
   over Serial to another microcontroller whenever queried. The logic in
   getButtonStateForPlayer() is customized to make each button work and
   feel right for my particular game. Your needs may differ.

   int pins[NUM_BUTTONS] = { P1 Left, P1 Right, P1 Fence, P1 Rocket,
                             P2 Left, P2 Right, P2 Fence, P2 Rocket };
   The Serial byte of button info sent is mapped according to array index,
   with first button (P1 Left) controlling MSB, and last button (P2 Rocket)
   controlling LSB.
*/

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

const uint8_t headerMask = 0b11001100; //Use this mask because it is an invalid PlayerButtonState

/*
  //Teensy 3.1 - Serial1 uses pins (0, 1) for (Rx, Tx)
  #define SER Serial1
  #define LED_PIN 13
  int pins[NUM_BUTTONS] = { 2, 3, 4, 5, 6, 7, 8, 9 };
*/
//Arduino Nano - Serial uses pins (0, 1) for (Rx, Tx)
#define SER Serial //Note: For Arduino Nano, uploading code might fail if RX/TX pins are connected!
#define LED_PIN 13
#define PIXEL_TWEEN_PIN 12
#define SPEED_POT_PIN A0
#define BRIGHT_POT_PIN A1
int pins[NUM_BUTTONS] = { 2, 3, 4, 5, 6, 7, 8, 9 };

void setup() {
  //Assign Pin numbers for buttons
  for (int bit = 0; bit < NUM_BUTTONS; bit++) {
    buttons[bit] = CreateButton(pins[NUM_BUTTONS - 1 - bit]);
  }

  SER.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  //pinMode(PIXEL_TWEEN_PIN, INPUT_PULLUP);
  //pinMode(SPEED_POT_PIN, INPUT);
  //pinMode(BRIGHT_POT_PIN, INPUT);

  //pinMode(13, OUTPUT);
  //digitalWrite(13, LOW);
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

// Returns a header along with a bit requesting whether pixel tweening should be enabled, controlled by a switch
uint8_t getHeaderValue() {
  uint8_t pixelTweenBit = digitalRead(PIXEL_TWEEN_PIN) ? 0 : 1;
  return headerMask | pixelTweenBit;
}

// Returns a value 0-255 requesting a particular game speed, controlled by a potentiometer
uint8_t getGameSpeedValue() {
  int val = analogRead(SPEED_POT_PIN);
  return map(val, 0, 1023, 0, 255);
}

// Returns a value 0-255 requesting a particular display brightness, controlled by a potentiometer
uint8_t getBrightnessValue() {
  int val = analogRead(BRIGHT_POT_PIN);
  return map(val, 0, 1023, 0, 255);
}

void loop() {
  pollButtonStates();
  if (SER.available()) {
    digitalWrite(LED_PIN, HIGH);
    //SER.write(getHeaderValue());
    //SER.write(getGameSpeedValue());
    //SER.write(getBrightnessValue());
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

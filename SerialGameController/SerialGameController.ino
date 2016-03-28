#include "Button.h"

#define NUM_BUTTONS 8 //Any more than 8 and we'll have to change code to transfer state in more than one byte
//Offset for each player in the button array
#define P1 0
#define P2 4
//Offset for each button in the button array
#define B_LEFT   0
#define B_RIGHT  1
#define B_FENCE  2
#define B_ROCKET 3
Button buttons[NUM_BUTTONS];

void setup() {
  //Assign Pin numbers for buttons
  buttons[0] = CreateButton(2); //P1 Left
  buttons[1] = CreateButton(2); //P1 Right
  buttons[2] = CreateButton(2); //P1 Fence
  buttons[3] = CreateButton(2); //P1 Rocket
  buttons[4] = CreateButton(2); //P2 Left
  buttons[5] = CreateButton(2); //P2 Right
  buttons[6] = CreateButton(2); //P2 Fence
  buttons[7] = CreateButton(2); //P2 Rocket
  
  Serial.begin(9600);

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
  Button& bL      = buttons[playerOffset+B_LEFT];
  Button& bR      = buttons[playerOffset+B_RIGHT];
  Button& bFence  = buttons[playerOffset+B_FENCE];
  Button& bRocket = buttons[playerOffset+B_ROCKET];
  //Only Right or Left bit can be set
  if(bL.wasPressed && (!bR.wasPressed || bL.stateChangeTime < bR.stateChangeTime)) {
    result |= 1 << (7-p-B_LEFT);
    bL.wasPressed = false;
  }
  else {
    result |= 1 << (3 - B_RIGHT);
    bR.wasPressed = false;
  }
  if(bFence.wasPressed || bFence.isPressed) {
    result |= 1 << (3 - B_FENCE);
    bFence.wasPressed = false;
  }
  if(bRocket.wasPressed) {
    result |= 1 << (3 - B_ROCKET);
    bRocket.wasPressed = false;
  }
  result = result << (4 - playerOffset);
  return result;
}

void clearWasPressed(){
  uint8_t id = NUM_BUTTONS;
  while(--id) {
    buttons[id].wasPressed = false;
  }
}

void pollButtonStates(){
  uint8_t id = NUM_BUTTONS;
  while(--id) {
    Update(buttons[id]);
  }
}

/* Returns a byte containing the isPressed state of
   all NUM_BUTTONS (<= 8) buttons, where the nth
   button is represented by the nth bit. */
uint8_t getButtonIsPressedState(){
  uint8_t allStates = 0;
  uint8_t id = NUM_BUTTONS;
  while(--id) {
    //Set bit in allStates if button is pressed
    if (buttons[id].isPressed)
      allStates |= _BV(id);
  }
  return allStates;
}

/* Returns a byte containing the wasPressed state of
   all NUM_BUTTONS (<= 8) buttons, where the nth
   button is represented by the nth bit. */
uint8_t getButtonWasPressedState(){
  uint8_t allStates = 0;
  uint8_t id = NUM_BUTTONS;
  while(--id) {
    //Set bit in allStates if button was pressed
    if (buttons[id].wasPressed)
      allStates |= _BV(id);
  }
  return allStates;
}

void loop() {
  pollButtonStates();
  if(Serial.available()) {
    //Empty the Serial input buffer (we respond the same way to any character)
    while(Serial.read() != -1) {}
    //Send Button state for both players
    uint8_t state = getButtonStateForPlayer(P0) | getButtonStateForPlayer(P1);
    Serial.write(state);
    /*
    //Send isPressed and wasPressed state
    Serial.write(getButtonIsPressedState());
    Serial.write(getButtonWasPressedState());
    //Clear wasPressed state because client knows about it now
    clearWasPressed();
    */
  }
}

#include "Button.h"

#define NUM_BUTTONS 8 //Any more than 8 and we'll have to change code to transfer state in more than one byte
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
    //Send isPressed and wasPressed state
    Serial.write(getButtonIsPressedState());
    Serial.write(getButtonWasPressedState());
    //Clear wasPressed state because client knows about it now
    clearWasPressed();
  }
}

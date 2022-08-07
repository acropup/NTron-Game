#ifndef SERIALGAMECONTROLLERCLIENT_H
#define SERIALGAMECONTROLLERCLIENT_H

const int bytesPerQuery = 4; //Header byte, Speed byte, Brightness byte, Player Button States
const uint8_t pixelTweenBit = 1;
const uint8_t headerMask = 0b11001100; //Use this mask because it is an invalid PlayerButtonState

//Warning: bit-field behaviour is largely dependent on platform implementation.
//This works with the Arduino IDE and Teensy, but might not elsewhere.
union PlayerButtonState {
  struct {
    bool Rocket : 1; //LSB
    bool Fence  : 1;
    bool Right  : 1;
    bool Left   : 1; //MSB
  };
  uint8_t raw;
};

uint8_t btnStates;
uint8_t headerByte;
uint8_t speedByte;
uint8_t brightnessByte;

//Call this in program setup
void initSerialController() {
  //On Teensy 3.1, Serial1 uses pins 0 (RX) and 1 (TX). These are available while using OctoWS2811 shield.
  Serial1.begin(9600);
}

//Sends a character to the SerialGameController to query for new button state.
void askForButtonStatus() {
  Serial1.write('?');
}

//Checks for a received button state (keeps only the most recent)
//and returns true if a new button state is available.
//Returns false if not enough bytes are available or if header is invalid.
bool checkForButtonStatus() {
  while (Serial1.available() > bytesPerQuery) { Serial1.read(); } //Discard any extra bytes
  
  if (Serial1.available() == bytesPerQuery) {
    headerByte = Serial1.read();      // headerByte should look like 0xFF or 0xFE. Low bit toggles pixel tweening.
    //Sanity check
    if ((headerByte & headerMask) != headerMask) {
      headerByte = 0;
      return false;
    }
    speedByte = Serial1.read();
    brightnessByte = Serial1.read();
    btnStates = Serial1.read();
    return true;
  }
  headerByte = 0;
  return false;
}

// Waits for up to 'timeout' milliseconds for the button state to arrive.
// If it arrives, saves new state to btnStates and returns number of ms remaining.
// If timeout, returns -1 and sets btnStates to 0.
int waitForButtonStatus(int timeout) {
  while ((Serial1.available() < bytesPerQuery) && timeout--) { delay(1); }
  
  if (timeout >= 0) { //Button status is available; go get it!
    checkForButtonStatus();
  }g
  else { //Did not receive button status within timeout
    //Clear the previous button state
    btnStates = 0;
  }
  return timeout;
}

//Copies button states from the serial response to each player's state.
void setPlayerButtonState(PlayerButtonState & p1State, PlayerButtonState & p2State) {
  p1State.raw = (btnStates >> 4);
  p2State.raw = (btnStates & 0x0F);
}

bool wasControllerReadSuccessfully() {
  return ((headerByte & headerMask) == headerMask);
}

//Returns a game speed (ms per frame), within range 10ms-2sec
uint8_t getControllerGameSpeed() {
  return map(speedByte, 0, 255, 10, 2000);
}

//Returns a screen brightness, within range 0-255
uint8_t getControllerGameBrightness() {
  return brightnessByte;
}

bool getControllerGameTweening() {
  return headerByte & pixelTweenBit;
}

//Queries the controller 8 times to determine the average response time
unsigned long measureControllerDelay() {
  //Poll and wait until controller is responding
  while(!checkForButtonStatus()) {
    askForButtonStatus();
  }
  checkForButtonStatus();
  
  elapsedMillis totalTime = 0;
  //Measure response time and average over 8 queries
  for(int i = 0; i < 8; i++) {
    askForButtonStatus();
    int retry = 10;
    while(!checkForButtonStatus() && retry--) { delay(1); }
  }
  unsigned long avg = totalTime >> 3; //Divide by 8 to get average per query

  #ifdef DEBUG
    Serial.print("Controller response time is ");
    Serial.print(avg);
    Serial.print("ms.");
  #endif
  return avg;
}

void debugPrintButtonState(uint8_t playerNum, PlayerButtonState btns) {
  Serial.print("Player ");
  Serial.print(playerNum);
  Serial.print(" button state: ");
  Serial.println(btns.raw, BIN);
  Serial.print("Left: ");
  Serial.println(btns.Left);
  Serial.print("Right: ");
  Serial.println(btns.Right);
  Serial.print("Fence: ");
  Serial.println(btns.Fence);
  Serial.print("Rocket: ");
  Serial.println(btns.Rocket);
}

#endif

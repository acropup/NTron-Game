#ifndef SERIALGAMECONTROLLERCLIENT_H
#define SERIALGAMECONTROLLERCLIENT_H

struct {
  bool p1Left   : 1;
  bool p1Right  : 1;
  bool p1Fence  : 1;
  bool p1Rocket : 1;
  bool p2Left   : 1;
  bool p2Right  : 1;
  bool p2Fence  : 1;
  bool p2Rocket : 1;
} btnIsPressed, btnWasPressed;

typedef struct PlayerButtonState_ {
  bool Left      : 1;
  bool Right     : 1;
  bool Fence     : 1;
  bool Rocket    : 1;
  bool WasLeft   : 1;
  bool WasRight  : 1;
  bool WasFence  : 1;
  bool WasRocket : 1;
} PlayerButtonState;

void askForButtonStatus() {
  Serial.write('?');
}

bool checkForButtonStatus() {
  if(Serial.available() > 1) {
    btnIsPressed = Serial.read();
    btnWasPressed = Serial.read();
    return true;
  }
}

void setPlayerButtonState(PlayerButtonState& p1State, PlayerButtonState& p2State) {
  p1State = (btnIsPressed & 0xF0) | (btnWasPressed >> 4);
  p2State = (btnIsPressed << 4)   | (btnWasPressed & 0x0F);
}

#endif

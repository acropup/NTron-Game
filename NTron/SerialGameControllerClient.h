#ifndef SERIALGAMECONTROLLERCLIENT_H
#define SERIALGAMECONTROLLERCLIENT_H

typedef struct PlayerButtonState_ {
  bool Left      : 1;
  bool Right     : 1;
  bool Fence     : 1;
  bool Rocket    : 1;
} PlayerButtonState;

struct {
  PlayerButtonState p1 : 4;
  PlayerButtonState p1 : 4;
/*bool p1Left   : 1;
  bool p1Right  : 1;
  bool p1Fence  : 1;
  bool p1Rocket : 1;
  bool p2Left   : 1;
  bool p2Right  : 1;
  bool p2Fence  : 1;
  bool p2Rocket : 1;*/
} btnPressed;

void askForButtonStatus() {
  Serial.write('?');
}

bool checkForButtonStatus() {
  if(Serial.available() > 1) {
    btnPressed = Serial.read();
    return true;
  }
  return false;
}

void setPlayerButtonState(PlayerButtonState& p1State, PlayerButtonState& p2State) {
/*p1State = (btnPressed >> 4);
  p2State = (btnPressed & 0x0F);*/
  p1State = btnPressed.p1;
  p2State = btnPressed.p2;
}

#endif

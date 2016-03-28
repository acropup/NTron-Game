#ifndef SERIALGAMECONTROLLERCLIENT_H
#define SERIALGAMECONTROLLERCLIENT_H

union PlayerButtonState {
  struct {
    bool Left   : 1;
    bool Right  : 1;
    bool Fence  : 1;
    bool Rocket : 1;
  };
  uint8_t raw;
};

uint8_t btnPressed;

void askForButtonStatus() {
  Serial.write('?');
}

bool checkForButtonStatus() {
  if (Serial.available() > 1) {
    btnPressed = Serial.read();
    return true;
  }
  return false;
}

void setPlayerButtonState(PlayerButtonState& p1State, PlayerButtonState& p2State) {
  p1State.raw = (btnPressed >> 4);
  p2State.raw = (btnPressed & 0x0F);
}

#endif

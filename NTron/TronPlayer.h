#ifndef TRONPLAYER_H
#define TRONPLAYER_H

#include "Button.h"

struct player {
  Button  btnL;    //Button that turns player Left (CCW)
  Button  btnR;    //Button that turns player Right (CW)
//  uint8_t btnPinL; 
//  uint8_t btnPinR; 
  uint8_t btnPinA; //Button that leaves fences behind player
  uint8_t btnPinB; //Button that shoots bombs
  bool isFencing;  //Player is actively leaving fences along their trail
  bool isFiring;   //Player is firing a bomb
  bool isAlive;    //Player is still alive
  unsigned long isTurning; //Player is turning (LSB is 0 if turning left, 1 if turning right, other bits is millis() for debouncing)
  int8_t x;  //Position on screen
  int8_t y;
  int8_t dx; //Direction traveling. Either dx or dy should be 0.
  int8_t dy;
  uint8_t power; //Power level for using special ability
};

typedef struct player Player;

static const uint8_t NUMPLAYERS = 2;
Player players[NUMPLAYERS];
uint8_t XMAX;
uint8_t YMAX;

void setScreenDims(uint8_t xmax, uint8_t ymax) {
  XMAX = xmax;
  YMAX = ymax;
}

void initPlayer(uint8_t pid, uint8_t leftPin, uint8_t rightPin, uint8_t aPin, uint8_t bPin, int8_t posX, int8_t posY) {
  players[pid] = { CreateButton(leftPin), CreateButton(rightPin), aPin, bPin, false, false, true, 0, posX, posY, 0, 0, 0 };
  //pinMode(leftPin, INPUT_PULLUP);
  //pinMode(rightPin, INPUT_PULLUP);
  pinMode(aPin, INPUT_PULLUP);
  pinMode(bPin, INPUT_PULLUP);
}

inline Player& getPlayer(uint8_t pid){
  return players[pid];
}

void checkButtons(Player& p){
  p.isFencing |= (digitalRead(p.btnPinA) == LOW);
  p.isFiring  |= (digitalRead(p.btnPinB) == LOW);
  Update(p.btnL);
  Update(p.btnR);
  /*
  if(!p.isTurning) {
    if(digitalRead(p.btnPinL) == LOW) {
      //Record time for debouncing, and set LSB to 0 to denote a left turn
      p.isTurning = millis() & ~1ul;
    }
    else if(digitalRead(p.btnPinR) == LOW) {
      //Record time for debouncing, and set LSB to 1 to denote a right turn
      p.isTurning = millis() & 1;
    }
  }
  */
}

//Should check buttons as often as possible, not just every frame
void checkButtons(){
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    checkButtons(getPlayer(--pid));
  }
}

void updatePlayerDirection(Player& p){
  Button &bL = p.btnL;
  Button &bR = p.btnR;
  if(bL.wasPressed && (!bR.wasPressed || bL.stateChangeTime < bR.stateChangeTime)) {
    //Turn left
    if(p.dx) {
      p.dy = p.dx;
      p.dx = 0;
    } 
    else {
      p.dx = -p.dy;
      p.dy = 0;
    }
  } else if(bR.wasPressed) {
    //Turn right
    if(p.dx) {
      p.dy = -p.dx;
      p.dx = 0;
    } 
    else {
      p.dx = p.dy;
      p.dy = 0;
    }
  }
  bL.wasPressed = false;
  bR.wasPressed = true;
  /*
  if(p.isTurning && millis() - p.isTurning > DebounceMS) {
    //LSB == 0 means turn left, LSB == 1 means turn right
    if((p.isTurning & 1) == 0) {
      //Turn left
      if(p.dx) {
        p.dy = p.dx;
        p.dx = 0;
      } 
      else {
        p.dx = -p.dy;
        p.dy = 0;
      }
    }
    else {
      //Turn right
      if(p.dx) {
        p.dy = -p.dx;
        p.dx = 0;
      } 
      else {
        p.dx = p.dy;
        p.dy = 0;
      }
    }
    //Reset
    p.isTurning = 0;
  }
  */
  /*
  int x = analogRead(p.btnPinL);
  int y = analogRead(p.btnPinL);
  if (abs(x) > abs(y)) {
    p.dx = (x > 0) ? 1 : -1;
    p.dy = 0;
  }
  else {
    p.dx = 0;
    p.dy = (y > 0) ? 1 : -1;
  }
  */
}

void updatePlayerPosition(Player& p){
  p.x += p.dx;
  p.y += p.dy;
  //Bounds check and wrap around the screen
  if(p.x == -1)   p.x = XMAX-1;
  if(p.y == -1)   p.y = YMAX-1;
  if(p.x == XMAX) p.x = 0;
  if(p.y == YMAX) p.y = 0;
}

//Call this once per frame before checking if any players have collided
void updatePlayers(){
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    Player& p = getPlayer(--pid);
    updatePlayerDirection(p);
    updatePlayerPosition(p);

    //If the buttons are being held down, these will be set true again before the next frame
    p.isFencing = false;
    p.isFiring = false;
  }
}

#endif

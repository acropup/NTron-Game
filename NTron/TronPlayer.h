#ifndef TRONPLAYER_H
#define TRONPLAYER_H

#include "Constants.h"
#include "Button.h"
#include "Rocket.h"

struct player {
  Button  btnL;    //Button that turns player Left (CCW)
  Button  btnR;    //Button that turns player Right (CW)
  Button  btnFire; //Button that fires rockets
  uint8_t btnFencePin; //Button that leaves fences behind player
  bool isFencing;  //Player is actively leaving fences along their trail
  bool isAlive;    //Player is still alive
  int8_t x;  //Position on screen
  int8_t y;
  int8_t dx; //Direction traveling. Either dx or dy should be 0.
  int8_t dy;
  uint8_t power; //Power level for using special ability. 16=1px Fence costs 2, powerup gives 16, auto-regen 1 per 2 frames.
};

typedef struct player Player;

static const uint8_t NUMPLAYERS = 2;
Player players[NUMPLAYERS];
uint8_t XMAX;
uint8_t YMAX;

uint8_t powerRegenRate = 50; //Percent of a power point regenerated per frame
uint8_t powerRegenCnt = 0;   //When this gets to 100, give all players 1 power

void setScreenDims(uint8_t xmax, uint8_t ymax) {
  XMAX = xmax;
  YMAX = ymax;
}

void initPlayer(uint8_t pid, uint8_t leftPin, uint8_t rightPin, uint8_t fencePin, uint8_t firePin, int8_t posX, int8_t posY) {
  players[pid] = { CreateButton(leftPin), CreateButton(rightPin), CreateButton(firePin), fencePin, false, true, posX, posY, 1, 0, 0 };
  pinMode(fencePin, INPUT_PULLUP);
}

inline Player& getPlayer(uint8_t pid){
  return players[pid];
}

void checkButtons(Player& p){
  p.isFencing |= (digitalRead(p.btnFencePin) == LOW);
  Update(p.btnFire);
  Update(p.btnL);
  Update(p.btnR);
}

//Should check buttons as often as possible, not just every frame
void checkButtons(){
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    checkButtons(getPlayer(--pid));
  }
}

/* Checks if player has pressed the isFencing button,
 * and if they have enough power to lay a fence.
 * If so, consumes power and returns true.
 */
bool isPlayerFencing(Player& p){
  if(p.isFencing && p.power > 1) {
    p.power-=2;
    return true;
  }
  return false;
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
  bR.wasPressed = false;
}

void updatePlayerPosition(Player& p){
  p.x += p.dx;
  p.y += p.dy;
  //Bounds check and wrap around the screen
  if(p.x == -1)   p.x = XMAX-1;
  if(p.y == -1)   p.y = YMAX-3;
  if(p.x == XMAX) p.x = 0;
  if(p.y == YMAX-2) p.y = 0;
}

//Fires a rocket if Player pressed the Fire button this frame
void maybeFireRocket(Player& p){
  Button &b = p.btnFire;
  if(b.wasPressed && p.power > 16) {
    p.power-=16;
    fireRocket(p.x, p.y, p.dx, p.dy);
  }
  b.wasPressed = false;
}

void moveRandom(uint8_t pid) {
    players[pid].x = random(WIDTH);
    players[pid].y = random(HEIGHT - 2); //height minus bottom bar
    players[pid].dx = 0;
    players[pid].dy = 0;
    switch(random(4)) {
      case 0: players[pid].dx = 1; break;
      case 1: players[pid].dx = -1; break;
      case 2: players[pid].dy = 1; break;
      case 3: players[pid].dy = -1; break;
    }
}

//Call this once per frame before checking if any players have collided
void updatePlayers(){
  //Player power regenerates slowly over time
  uint8_t regenAmt = 0;
  powerRegenCnt += powerRegenRate;
  if(powerRegenCnt >= 100) {
    powerRegenCnt -= 100;
    regenAmt = 1;
  }
  
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    Player& p = getPlayer(--pid);
    updatePlayerDirection(p);
    updatePlayerPosition(p);
    maybeFireRocket(p);
    p.power += regenAmt;
    //Check for overflow
    if(p.power < regenAmt) p.power = 255;

    //If the fencing button is being held down, it will be set true again before the next frame
    p.isFencing = false;
  }
}

void applyPowerup(Player& p) {
  p.power+=16;
  //Check for overflow
  if(p.power < 16) p.power = 255;
}

#endif

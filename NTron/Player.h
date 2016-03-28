#ifndef PLAYER_H
#define PLAYER_H

#include "Constants.h"
#include "SerialGameControllerClient.h"
#include "Rocket.h"

typedef struct Player_ {
  PlayerButtonState btnState; //State of all player buttons, from SerialGameControllerClient.h
  bool isAlive;    //Player is still alive
  int8_t x;  //Position on screen
  int8_t y;
  int8_t dx; //Direction traveling. Either dx or dy should be 0.
  int8_t dy;
  uint8_t power; //Power level for using special ability. Rocket costs 16, 1px Fence costs 2, powerup gives 16, auto-regen 1 per 2 frames. Config in Constants.h.
} Player;

static const uint8_t NUMPLAYERS = 2;
Player players[NUMPLAYERS];

uint8_t powerRegenRate = 50; //Percent of a power point regenerated per frame
uint8_t powerRegenCnt = 0;   //When this gets to 100, give all players 1 power

void resetPlayer(Player& p, int8_t posX, int8_t posY, int8_t dx, int8_t dy){
  p.isAlive = true;
  p.power = 0;
  p.x = posX;
  p.y = posY;
  p.dx = dx;
  p.dy = dy;
}

Player& initPlayer(uint8_t pid) {
  players[pid] = { 0, true, 0, 0, 1, 0, 0 };
  return players[pid];
}

inline Player& getPlayer(uint8_t pid){
  return players[pid];
}

/*Checks if player has pressed the isFencing button,
  and if they have enough power to lay a fence.
  If so, consumes power and returns true. */
bool isPlayerFencing(Player& p){
  if(p.btnState.Fence && p.power >= FENCE_COST) {
    p.power-=FENCE_COST;
    return true;
  }
  return false;
}

/*Lays a fence if Player is holding down the isFencing button, 
  and they have enough power to afford it. */
void maybeLayFence(CRGB leds[], Player& p) {
  if(isPlayerFencing(p)) {
    addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], FENCECOLOUR));
  }
  else {
    addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], BGCOLOUR));
  }
}

//Fires a rocket if Player pressed the Fire button this frame
void maybeFireRocket(Player& p){
  if(p.btnState.Rocket && p.power > ROCKET_COST) {
    p.power-=ROCKET_COST;
    fireRocket(p.x, p.y, p.dx, p.dy);
  }
}

//Draws a fence behind every player that is laying fences this frame
inline void layAllFences(CRGB leds[]) {
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    maybeLayFence(leds, getPlayer(--pid));
  }
}

void updatePlayerDirection(Player& p){
  //Only one of these should ever be true
  bool bL = p.btnState.Left;
  bool bR = p.btnState.Right;
  if(bL) {
    //Turn left
    if(p.dx) {
      p.dy = p.dx;
      p.dx = 0;
    } 
    else {
      p.dx = -p.dy;
      p.dy = 0;
    }
  } else if(bR) {
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
}

/* Moves position (x,y) by amount (dx,dy),
   and wraps around WIDTH,HEIGHT dimensions automatically. */
inline void moveAndWrap(int8_t& x, int8_t& y, int8_t dx, int8_t dy) {
  x += dx;
  y += dy;
  //Bounds check and wrap around the screen
  if(x == -1)       x = WIDTH-1;
  if(y == -1)       y = HEIGHT-3;
  if(x == WIDTH)    x = 0;
  if(y == HEIGHT-2) y = 0;
}

/* Updates player position by moving player in direction dx or dy. */
void updatePlayerPosition(Player& p){
  moveAndWrap(p.x, p.y, p.dx, p.dy);
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
void updatePlayers(CRGB leds[]){
  //Player power regenerates slowly over time
  uint8_t regenAmt = 0;
  powerRegenCnt += POWER_REGEN_RATE;
  if(powerRegenCnt >= 100) {
    powerRegenCnt -= 100;
    regenAmt = 1;
  }
  
  layAllFences(leds);
  
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    Player& p = getPlayer(--pid);
    updatePlayerDirection(p);
    updatePlayerPosition(p);
    maybeFireRocket(p);
    p.power += regenAmt;
    //Check for overflow
    if(p.power < regenAmt) p.power = 255;
  }
}

void applyPowerup(Player& p) {
  p.power+=POWERUP_VALUE;
  //Check for overflow
  if(p.power < POWERUP_VALUE) p.power = 255;
}

/*To be called after the players are moved. Checks for player-to-player collisions,
  returns true if collided, in which case both players should die. */
bool checkPlayerCollision() {
  Player& p1 = getPlayer(0);
  Player& p2 = getPlayer(1);
  if(p1.x == p2.x && p1.y == p2.y){
    return true;
  }
  if(p1.dx == -p2.dx && p1.dy == -p2.dy) { //Players are moving in opposite directions
    int8_t x1 = p1.x;
    int8_t y1 = p1.y;
    moveAndWrap(x1, y1, -p1.dx, -p1.dy);
    if(x1 == p2.x && y1 == p2.y) {
      return true;
    }
  }
  return false;
}

#endif

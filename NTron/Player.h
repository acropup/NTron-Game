#ifndef PLAYER_H
#define PLAYER_H

#include <FastLED.h>
#include "Constants.h"
#include "SerialGameControllerClient.h"
#include "Rocket.h"

typedef struct Player_ {
  PlayerButtonState buttons; //State of all player buttons, from SerialGameControllerClient.h
  bool isAlive;    //Player is still alive
  int8_t x;  //Position on screen. (0,0) is top left.
  int8_t y;
  int8_t dx; //Direction traveling. Either dx or dy should be 0.
  int8_t dy; //+dx is right, +dy is down.
  uint8_t power; //Power level for using special ability. Rocket costs 16, 1px Fence costs 2, powerup gives 16, auto-regen 1 per 2 frames. Config in Constants.h.
  CRGB colour;      // Colour of player pixel
  CRGB fenceColour; // Colour of player's fences
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

Player& initPlayer(uint8_t pid, CRGB playerColour, CRGB fenceColour) {
  Player &p = players[pid];
  p = { 0, true, 0, 0, 1, 0, 0, 0, 0 };
  
  //TODO: Figure out why this is necessary to set the colours. Doesn't seem to work in struct initializer.
  p.colour = playerColour;
  p.fenceColour = fenceColour;
  
  return players[pid];
}

inline Player& getPlayer(uint8_t pid){
  return players[pid];
}

void killPlayer(Player& p) {
  p.isAlive = false;
  p.dx = 0;
  p.dy = 0;
}

// Checks if player has pressed the isFencing button,
// and if they have enough power to lay a fence.
// If so, consumes power and returns true.
bool isPlayerFencing(Player& p){
  if(p.buttons.Fence && p.power >= FENCE_COST) {
    p.power-=FENCE_COST;
    return true;
  }
  return false;
}

// Lays a fence if Player is holding down the isFencing button, 
// and they have enough power to afford it.
void maybeLayFence(CRGB leds[], Player& p) {
  if(isPlayerFencing(p)) {
    addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], p.fenceColour));
  }
  else {
    addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], BGCOLOUR));
  }
}

//Draws a fence behind every player that is laying fences this frame
inline void layAllFences(CRGB leds[]) {
  uint8_t pid = NUMPLAYERS;
  while(pid > 0){
    maybeLayFence(leds, getPlayer(--pid));
  }
}

//Fires a rocket if Player pressed the Fire button this frame
void maybeFireRocket(Player& p){
  if(p.buttons.Rocket && p.power > ROCKET_COST) {
    p.power-=ROCKET_COST;
    fireRocket(p.x, p.y, p.dx, p.dy);
  }
}

void updatePlayerDirection(Player& p){
  //Only one of these should ever be true
  bool bL = p.buttons.Left;
  bool bR = p.buttons.Right;
  if(bL) {
    //Turn left
    if(p.dx) {
      p.dy = -p.dx;
      p.dx = 0;
    } 
    else {
      p.dx = p.dy;
      p.dy = 0;
    }
  } else if(bR) {
    //Turn right
    if(p.dx) {
      p.dy = p.dx;
      p.dx = 0;
    } 
    else {
      p.dx = -p.dy;
      p.dy = 0;
    }
  }
}

// Moves position (x,y) by amount (dx,dy),
// and wraps around WIDTH,PLAYABLEHEIGHT dimensions automatically.
inline void moveAndWrap(int8_t& x, int8_t& y, int8_t dx, int8_t dy) {
  x += dx;
  y += dy;
  //Bounds check and wrap around the screen
  if(y == PLAYABLEHEIGHT) y = 0;
  else if(x == WIDTH)     x = 0;
  else if(y == -1)        y = PLAYABLEHEIGHT-1;
  else if(x == -1)        x = WIDTH-1;
}

// Updates player position by moving player in direction dx or dy.
void updatePlayerPosition(Player& p){
  moveAndWrap(p.x, p.y, p.dx, p.dy);
}

// Call this once per frame before checking if any players have collided
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

// To be called after the players are moved. Checks for player-to-player collisions,
// returns true if collided, in which case both players should die.
bool checkPlayerToPlayerCollision() {
  Player& p1 = getPlayer(0);
  Player& p2 = getPlayer(1);
  
  if(!p1.isAlive || !p2.isAlive) return false;
  
  if(p1.x == p2.x && p1.y == p2.y){
    return true;
  }
  //If Players are moving in opposite directions
  if(p1.dx == -p2.dx && p1.dy == -p2.dy) {
    int8_t x1 = p1.x;
    int8_t y1 = p1.y;
    //Move p1 back a step and test again
    moveAndWrap(x1, y1, -p1.dx, -p1.dy);
    if(x1 == p2.x && y1 == p2.y) {
      return true;
    }
  }
  return false;
}

// To be called after the players are moved. Checks for player-to-environment collisions,
// returns true if collided, in which case Player p should die.
// If Player collided with a Powerup, Player receives the Powerup bonus, and new Powerups
// are spawned onto the field.
bool checkPlayerCollision(CRGB leds[], Player& p) {
  if(leds[XY(p.x, p.y)] == BGCOLOUR) { //Player is moving into an empty pixel
    addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], p.colour));
  }
  else if (tryHitPowerup(p.x, p.y)) { //Player is moving into a pixel with a powerup
    addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], p.colour));
    applyPowerup(p);
    spawnPowerups(leds, NUM_POWERUP_REGROWTH);
  }
  else {
    return true; //Collided
  }
  return false;
}
    
// Tries to hit player at coordinates (x, y). Kills player if shootToKill is true. Return values are:
// 0 if no player is hit.
// 1 if a player is hit at (x, y). The player is automatically killed.
// 2 if no player is there, but a player was there last frame (player is tweening out of that frame).
uint8_t tryHitPlayer(uint8_t x, uint8_t y, bool shootToKill) {
  uint8_t hit = 0;
  for (int i = 0; i < NUMPLAYERS; i++) {
    Player& p = getPlayer(i);
    if (!p.isAlive) continue;
    int8_t px = p.x;
    int8_t py = p.y;
    if (px == x && py == y) {
      if (shootToKill) {
        killPlayer(p);
      }
      hit = 1;
    }
    else if(hit == 0) {
      moveAndWrap(px, py, -p.dx, -p.dy);
      if (px == x && py == y) {
        hit = 2; //Near miss - player was at this coord last frame
      }
    }
  }
  
  return hit;
}

#endif

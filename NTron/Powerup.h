#ifndef POWERUP_H
#define POWERUP_H

#include <FastLED.h>
#include "PixelTweening.h"
#include "Constants.h"

typedef struct Powerup_ {
  uint16_t pos;  //Position on screen
  uint8_t hue;
} Powerup;

Powerup powerups[MAX_POWERUPS];
int numPowerups = 0;

//Call when resetting game
void clearPowerups() {
  numPowerups = 0;
}

//Spawn a powerup (maybe). Randomization logic is simple and may result in no powerup spawned.
void spawnPowerup(CRGB* leds) {
  if(numPowerups >= MAX_POWERUPS) return;
  
  int8_t x, y;
  x = random(WIDTH);
  y = random(PLAYABLEHEIGHT);
  if (leds[XY(x, y)] == BGCOLOUR) {
    powerups[numPowerups++] = {XY(x, y), (uint8_t)random(255)};
  }
}

//Spawn howMany powerups
void spawnPowerups(CRGB* leds, uint8_t howMany) {
  while (howMany--) {
    spawnPowerup(leds);
  }
}

//TODO: This is a test function to spawn powerups at specific positions
void spawnPowerup(CRGB* leds, uint8_t x, uint8_t y) {
  if(numPowerups < MAX_POWERUPS && leds[XY(x, y)] == BGCOLOUR) {
    powerups[numPowerups++] = {XY(x, y), (uint8_t)random(255)};
  }
}

void drawPowerups(CRGB* leds) {
  for (int i = 0; i < numPowerups; i++) {
    powerups[i].hue += POWERUP_HUE_SPEED;
    addPixelTween(tweenPixelTo(leds[powerups[i].pos], 
      CHSV(powerups[i].hue, ~0, ~0)));
  }
}

// Checks if there is a powerup at (x,y), returns true and removes it if there is one.
bool tryHitPowerup(uint8_t x, uint8_t y) {
  uint16_t target = XY(x, y);
  for (int i = 0; i < numPowerups; i++) {
    if (target == powerups[i].pos) { //Powerup is at (x,y)
      powerups[i] = powerups[--numPowerups]; //Unordered remove of powerup i
      return true;
    }
  }
  return false;
}

#endif

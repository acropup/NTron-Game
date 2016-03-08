#ifndef POWERUP_H
#define POWERUP_H

#include <FastLED.h>
#include "PixelTweening.h"
#include "Constants.h"

#define BGCOLOUR CRGB::Black

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

void spawnPowerup(CRGB* leds) {
  if(numPowerups >= MAX_POWERUPS) return;
  
  int8_t x, y;
  x = random(WIDTH);
  y = random(HEIGHT - 2);
  if (leds[XY(x, y)] != (CRGB)BGCOLOUR) {
    return;
  }
  
  powerups[numPowerups++] = {XY(x, y), (uint8_t)random(255)};
}

//TODO: This is a test function to spawn powerups at specific positions
void spawnPowerup(CRGB* leds, uint8_t x, uint8_t y) {
  if(numPowerups >= MAX_POWERUPS) return;
  if (leds[XY(x, y)] != (CRGB)BGCOLOUR) {
    return;
  }
  
  powerups[numPowerups++] = {XY(x, y), (uint8_t)random(255)};
}

void drawPowerups(CRGB* leds) {
  for (int i = 0; i < numPowerups; i++) {
    powerups[i].hue += POWERUP_HUE_SPEED;
    addPixelTween(tweenPixelTo(leds[powerups[i].pos], 
      CHSV(powerups[i].hue, ~0, ~0)));
  }
}

bool hitPowerup(uint8_t x, uint8_t y) {
  uint16_t target = XY(x, y);
  bool hit = false;
  for (int i = 0; i < numPowerups; i++) {
    if (hit) {
      powerups[i-1] = powerups[i];
    } else if (target == powerups[i].pos) {
      hit = true;
    }
  }
  if (hit) {
    numPowerups--;
  }
  
  return hit;
}

#endif

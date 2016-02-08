#ifndef POWERBAR_H
#define POWERBAR_H

#include <FastLED.h>
#include "TronPlayer.h"
#include "Constants.h"

uint8_t offset = 0;

//Draw the bar across the bottom of the screen, to mark off the power levels
void drawPowerBar(CRGB* leds) {
}

//Draw the power level pixels
void updatePowerBar(CRGB* leds) {
  Player& pLeft = getPlayer(0);
  Player& pRight = getPlayer(1);
  int y = HEIGHT - 1;
  for (int x = 0; x < pLeft.power; x++) {
    leds[XY(x, y)] = CRGB(255, 255, 255);
  }
  for (int x = WIDTH - pRight.power; x < WIDTH; x++) {
    leds[XY(x, y)] = CRGB(255, 255, 255);
  }
  
  y = HEIGHT - 2;
  offset -= 20;
  for (uint8_t x = 0; x < WIDTH; x++) {
    addPixelTween(tweenPixelTo(leds[XY(x, y)], 
      CHSV(0, 0, (sin(((x*19+offset)/128.0) * 3.14159265) * 128) + 128)));
  }
}

#endif

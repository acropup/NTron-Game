#ifndef POWERBAR_H
#define POWERBAR_H

#include <FastLED.h>
#include "Constants.h"
#include "PixelTweening.h"

uint8_t offset = 0;
uint8_t oldLeft = 0;
uint8_t oldRight = 0;

//Draw the bar across the bottom of the screen, to mark off the power levels
void drawPowerBar(CRGB* leds) {
}

//Draw the power level pixels
void updatePowerBar(CRGB* leds, uint8_t leftPower, uint8_t rightPower) {
  //If left power increase is significant
  if(leftPower > oldLeft && leftPower - oldLeft > 8) {
    
  }
  if(rightPower > oldRight && rightPower - oldRight > 8) {
    
  }
  oldLeft  = leftPower;
  oldRight = rightPower;

  //Draw power level on left side of screen
  int y = HEIGHT - 1;
  int xl = 0;
  while(leftPower >= 16) { //A full pixel represents 16 power
    leds[XY(xl, y)] = CRGB(255, 255, 255);
    leftPower-=16;
    xl++;
  }
  int bright = map(leftPower, 0, 15, 0, 255); //Partial brightness for last pixel
  addPixelTween(tweenPixelTo(leds[XY(xl, y)], CRGB(bright, bright, bright)));

  //Draw power level on right side of screen
  int xr = WIDTH - 1;
  while(rightPower >= 16) { //A full pixel represents 16 power
    leds[XY(xr, y)] = CRGB(255, 255, 255);
    rightPower-=16;
    xr--;
  }
  bright = map(rightPower, 0, 15, 0, 255); //Partial brightness for last pixel
  addPixelTween(tweenPixelTo(leds[XY(xr, y)], CRGB(bright, bright, bright)));
  
  while(xl <= xr) { //Make sure the remaining pixels are black
    leds[XY(xl++, y)] = BGCOLOUR;
  }

  //Constant light wave
  y = HEIGHT - 2;
  offset -= 20;
  for (uint8_t x = 0; x < WIDTH; x++) {
    addPixelTween(tweenPixelTo(leds[XY(x, y)], 
      CHSV(0, 0, (sin(((x*19+offset)/128.0) * 3.14159265) * 128) + 128)));
  }
}

#endif
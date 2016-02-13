#ifndef POWERBAR_H
#define POWERBAR_H

#include <FastLED.h>
#include "Constants.h"
#include "PixelTweening.h"

static const uint8_t yBottom = HEIGHT - 1;

uint8_t offset = 0;
uint8_t oldLeft = 0;
uint8_t oldRight = 0;

inline CRGB CRGBgrayscale(uint8_t lightness) {
  return CRGB(lightness, lightness, lightness);
}

//Draw power level on left side of screen. Returns column where power bar ends.
uint8_t drawLeftPowerBar(CRGB* leds, uint8_t leftPower){
  int xl = 0;
  while(leftPower >= 16) { //A full pixel represents 16 power
    leds[XY(xl, yBottom)] = CRGB(255, 255, 255);
    leftPower-=16;
    xl++;
  }
  int bright = map(leftPower, 0, 15, 0, 255); //Partial brightness for last pixel
  addPixelTween(tweenPixelTo(leds[XY(xl, yBottom)], CRGBgrayscale(bright)));
  return xl;
}

//Draw power level on right side of screen. Returns column where power bar ends.
uint8_t drawRightPowerBar(CRGB* leds, uint8_t rightPower){
  int xr = WIDTH - 1;
  while(rightPower >= 16) { //A full pixel represents 16 power
    leds[XY(xr, yBottom)] = CRGB(255, 255, 255);
    rightPower-=16;
    xr--;
  }
  int bright = map(rightPower, 0, 15, 0, 255); //Partial brightness for last pixel
  addPixelTween(tweenPixelTo(leds[XY(xr, yBottom)], CRGBgrayscale(bright)));
  return xr;
}

//Make a row of pixels black, from column xl to column xr
void clearRowPixels(CRGB* leds, uint8_t yRow, uint8_t xl, uint8_t xr){
  while(xl <= xr) {
    leds[XY(xl++, yRow)] = BGCOLOUR;
  }
}

//Draw the power level pixels
void updatePowerBar(CRGB* leds, uint8_t leftPower, uint8_t rightPower) {
  //TODO: optimize these functions based on oldLeft and oldRight values
  uint8_t xl = drawLeftPowerBar(leds, leftPower);
  uint8_t xr = drawRightPowerBar(leds, rightPower);
  //Make sure the remaining pixels are black
  clearRowPixels(leds, yBottom, xl, xr);

  //If power increase is significant on left or right side, player got a powerup
  bool lPowerup = (leftPower > oldLeft && leftPower - oldLeft > 8);
  bool rPowerup = (rightPower > oldRight && rightPower - oldRight > 8);
  uint8_t y = yBottom - 1;
  if(lPowerup) {
    for (int8_t x = WIDTH/2-1; x >= 0; x--) {
      leds[XY(x, y)] = CRGB::White;
    }
  }
  if(rPowerup) {
    for (int8_t x = WIDTH/2; x < WIDTH; x++) {
      leds[XY(x, y)] = CRGB::White;
    }
  }

  /*
  //Constant light wave
  offset -= 20;
  for (uint8_t x = 0; x < WIDTH; x++) {
    addPixelTween(tweenPixelTo(leds[XY(x, y)], 
      CRGBgrayscale((sin(((x*19+offset)/128.0) * 3.14159265) * 128) + 128)));
  }
*/
  //If edge LEDs are off on this row, then all LEDs are off
  if(leds[XY(0, y)].r || leds[XY(WIDTH-1, y)].r) {
    //Fade to black
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint8_t brightness = leds[XY(x, y)].r;
      if(brightness) {
        addPixelTween(tweenPixelTo(leds[XY(x, y)], CRGBgrayscale(brightness/1.5)));
      }
    }
  }
  oldLeft  = leftPower;
  oldRight = rightPower;
}

#endif

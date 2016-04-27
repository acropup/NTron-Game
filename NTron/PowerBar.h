#ifndef POWERBAR_H
#define POWERBAR_H

#include <FastLED.h>
#include "Constants.h"
#include "PixelTweening.h"

// PowerBar usese the bottom row of the screen, shows Player 1's power on left half,
// and Player 2's power on right half. Flashes a notification over the Player's
// PowerBar area if the Player picks up a Powerup.

static const uint8_t yBottom = HEIGHT - 1;

uint8_t oldLeft = 0;
uint8_t oldRight = 0;

//Call when resetting game
void clearPowerBars() {
  oldLeft = 0;
  oldRight = 0;
}

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

//Fade a row of pixels toward black, from column xl to column xr
void fadeRowPixels(CRGB* leds, uint8_t yRow, uint8_t xl, uint8_t xr){
  while(xl <= xr) {
    CRGB c = leds[XY(xl, yRow)];
    if (c != BGCOLOUR) {
      c.r *= 0.6;
      c.g *= 0.6;
      c.b *= 0.6;
      addPixelTween(tweenPixelTo(leds[XY(xl, yRow)], c));
    }
    xl++;
  }
}

//Draw the power level pixels
void updatePowerBar(CRGB* leds, uint8_t leftPower, uint8_t rightPower) {
  //TODO: optimize these functions based on oldLeft and oldRight values
  uint8_t xl = drawLeftPowerBar(leds, leftPower);
  uint8_t xr = drawRightPowerBar(leds, rightPower);

  //If power increase is significant on left or right side, player got a powerup
  bool lPowerup = (leftPower > oldLeft && leftPower - oldLeft > 8);
  bool rPowerup = (rightPower > oldRight && rightPower - oldRight > 8);
  if(lPowerup) {
    for (int8_t x = WIDTH/2-1; x >= xl; x--) {
      leds[XY(x, yBottom)] = PLAYER1COLOUR;
    }
  }
  if(rPowerup) {
    for (int8_t x = WIDTH/2; x < xr; x++) {
      leds[XY(x, yBottom)] = PLAYER2COLOUR;
    }
  }
  
  //Fade the flashing pixels to black
  fadeRowPixels(leds, yBottom, xl, xr);

  oldLeft  = leftPower;
  oldRight = rightPower;
}

#endif

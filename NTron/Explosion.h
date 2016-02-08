#ifndef EXPLOSION_H
#define EXPLOSION_H

#include<FastLED.h>
#include "XYmatrix.h"
#include "PixelTweening.h"

void explodeAt(CRGB leds[], uint8_t x, uint8_t y) {
  /*
   *       *
   *     * * *
   *   * * * * *
   *     * * *
   *       *
   *
   for(uint8_t j = -2; j < 2; j++) {
     if(y+j >= 24) continue;
     for(int i = -2+abs(j); i < 2-abs(j); i++) {
       if(x+i >= 32) continue;
       addPixelTween({&leds[XY(x+i,y+j)], CRGB::Yellow, CRGB::Red });
     }
   }
   */

  /*
   *       *
   *     * * *
   *       *
   */
   addPixelTween({&leds[XY(x,y)], CRGB::White, CRGB::Yellow });
   if(x > 0) addPixelTween({&leds[XY(x-1,y)], CRGB::Yellow, CRGB::Black });
   if(x < 31)addPixelTween({&leds[XY(x+1,y)], CRGB::Yellow, CRGB::Black });
   if(y > 0) addPixelTween({&leds[XY(x,y-1)], CRGB::Yellow, CRGB::Black });
   if(y < 23)addPixelTween({&leds[XY(x,y+1)], CRGB::Yellow, CRGB::Black });
   
}

#endif

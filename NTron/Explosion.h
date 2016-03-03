#ifndef EXPLOSION_H
#define EXPLOSION_H

#include<FastLED.h>
#include "XYmatrix.h"
#include "PixelTweening.h"

typedef struct Explosion_ {
  uint8_t x;
  uint8_t y;
  uint8_t size; //Size or kind of explosion
  uint8_t age;  //Lifetime of explosion, useful for multi-frame animations
} Explosion;

Explosion explosions[MAX_EXPLOSIONS];
int numExplosions = 0;

//Call when resetting game
void clearExplosions() {
  numExplosions = 0;
}

void explodeAt(uint8_t x, uint8_t y, uint8_t size) {
  if(numExplosions < MAX_EXPLOSIONS && x < WIDTH && y < HEIGHT) {
    explosions[numExplosions++] = (Explosion) { x, y, size, 0 };
  }
}

//Removes explosion from the explosions array, shifts other explosions over
void removeExplosion(uint8_t eid) {
  numExplosions--;
  while(eid < numExplosions) {
    explosions[eid] = explosions[eid+1];
    eid++;
  }
}

//Draw the frame of the explosion at index eid,
//and remove explosion if this is its last frame
void drawExplosion(CRGB leds[], uint8_t eid) {
  Explosion& e = explosions[eid];
  uint8_t x = e.x, y = e.y;
  switch(e.size) {
    case 0:
    case 1:
    {
      switch(e.age) {
        case 0:
        {
          
          //       *
          //     * * *
          //       *
          addPixelTween({&leds[XY(x,y)], CRGB::White, CRGB::Yellow });                //Center
          if(x > 0) addPixelTween({&leds[XY(x-1,y)], CRGB::Yellow, (CRGB)BGCOLOUR }); //Left
          if(x < 31)addPixelTween({&leds[XY(x+1,y)], CRGB::Yellow, (CRGB)BGCOLOUR }); //Right
          if(y > 0) addPixelTween({&leds[XY(x,y-1)], CRGB::Yellow, (CRGB)BGCOLOUR }); //Up
          if(y < 23)addPixelTween({&leds[XY(x,y+1)], CRGB::Yellow, (CRGB)BGCOLOUR }); //Down
          e.age = 1;
          break;
        }
        case 1:
        {
          addPixelTween({&leds[XY(x,y)], CRGB::Yellow, (CRGB)BGCOLOUR });             //Center
          removeExplosion(eid);
          break;
        }
        default:
          removeExplosion(eid);
      }
      break;
    }
    default:
      removeExplosion(eid);
  }
}
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

void drawExplosions(CRGB leds[]) {
  int i = numExplosions;
  while(i--) {
    drawExplosion(leds, i);
  }
}

#endif

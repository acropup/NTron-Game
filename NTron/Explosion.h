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

//Removes explosion from the explosions array, puts last explosion in its place.
void removeExplosion(uint8_t eid) {
  if (eid < --numExplosions) {
    explosions[eid] = explosions[numExplosions];
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
          if(x > 0) addPixelTween({&leds[XY(x-1,y)], CRGB::Yellow, BGCOLOUR }); //Left
          if(x < 31)addPixelTween({&leds[XY(x+1,y)], CRGB::Yellow, BGCOLOUR }); //Right
          if(y > 0) addPixelTween({&leds[XY(x,y-1)], CRGB::Yellow, BGCOLOUR }); //Up
          if(y < 23)addPixelTween({&leds[XY(x,y+1)], CRGB::Yellow, BGCOLOUR }); //Down
          e.age = 1;
          break;
        }
        case 1:
        {
          addPixelTween({&leds[XY(x,y)], CRGB::Yellow, BGCOLOUR });             //Center
          removeExplosion(eid);
          break;
        }
        default:
          removeExplosion(eid);
      }
      break;
    }
    case 2:
    {
      switch(e.age) {
        case 0:
        {
          if(x > 0) explodeAt(x-2,y,1); //Left
          if(x < 31)explodeAt(x+2,y,1); //Right
          if(y > 0) explodeAt(x,y-2,1); //Up
          if(y < 23)explodeAt(x,y+2,1); //Down
          break;
        }
        case 1:
        {
          if(x > 0) explodeAt(x-2,y-1,1); //Left
          if(x < 31)explodeAt(x+2,y+1,1); //Right
          if(y > 0) explodeAt(x+1,y-2,1); //Up
          if(y < 23)explodeAt(x-1,y+2,1); //Down
          removeExplosion(eid);
        }
      }
      e.age++;
      break;
    }
    case 6: //-dy T-shaped small explosion
    case 7: //-dx T-shaped small explosion
    case 9: //+dx T-shaped small explosion
    case 10://+dy T-shaped small explosion
    {
      //T-shaped small explosion, computed size = 8 + r.dx + 2*r.dy
      //Example, for case 9 (rocket was travelling to the right):
      //       *
      //       * *
      //       *
      addPixelTween({&leds[XY(x,y)], CRGB::White, BGCOLOUR});                               //Center
      if(e.size != 9  && x > 0) addPixelTween({&leds[XY(x-1,y)], CRGB::Yellow, BGCOLOUR }); //Left
      if(e.size != 7  && x < 31)addPixelTween({&leds[XY(x+1,y)], CRGB::Yellow, BGCOLOUR }); //Right
      if(e.size != 10 && y > 0) addPixelTween({&leds[XY(x,y-1)], CRGB::Yellow, BGCOLOUR }); //Up
      if(e.size != 6  && y < 23)addPixelTween({&leds[XY(x,y+1)], CRGB::Yellow, BGCOLOUR }); //Down
      removeExplosion(eid);
      break;
    }
    default:
      removeExplosion(eid);
  }
}

void drawExplosions(CRGB leds[]) {
  int i = numExplosions;
  while(i--) {
    drawExplosion(leds, i);
  }
}

#endif

#ifndef ROCKET_H
#define ROCKET_H

#include <FastLED.h>
#include "Constants.h"
#include "PixelTweening.h"
#include "Explosion.h"

typedef struct Rocket_ {
  int8_t x;  //Position on screen of the nose of the rocket
  int8_t y;
  int8_t dx; //Direction traveling. Either dx or dy should be 0.
  int8_t dy; //The other should be ±1. Rocket travels 2 pixels per frame.
  uint8_t age; //Counter for number of frames since the rocket was fired.
} Rocket;

Rocket rockets[MAX_ROCKETS];
int numRockets = 0;

//Fires a rocket from position (fromx, fromy) in the direction (dx, dy).
//dx or dy must be 0, the other must be ±1.
void fireRocket(int8_t fromx, int8_t fromy, int8_t dx, int8_t dy) {
  if(numRockets < MAX_ROCKETS) {
    rockets[numRockets++] = (Rocket) { fromx, fromy, dx, dy, 0 };
  }
}

inline Rocket& getRocket(uint8_t rid){
  return rockets[rid];
}

//Removes rocket from the rockets array, shifts other rockets over
void removeRocket(uint8_t rid) {
  numRockets--;
  while(rid < numRockets) {
    rockets[rid] = rockets[rid+1];
    rid++;
  }
}

//Moves Rocket one frame forward. Rocket moves two pixels per frame.
//Returns true if collision detected.
bool stepRocket(CRGB leds[], Rocket& r) {
  r.age++;
  r.x+=r.dx;
  r.y+=r.dy;
  //Check for collision with first pixel
  if(leds[XY(r.x, r.y)] == (CRGB)BGCOLOUR) { //Rocket is moving into an empty pixel
    r.x+=r.dx;
    r.y+=r.dy;
    //Check for collision with second pixel
    if(leds[XY(r.x, r.y)] == (CRGB)BGCOLOUR) { //Rocket is moving into an empty pixel
      //Rocket did not collide
      return false;
    }
  }
  //Rocket collided with something at position (r.x, r.y)
  return true;
}

void drawRocket(CRGB leds[], Rocket& r) {
  int8_t x1, x2, x3, x4, x5;
  int8_t y1, y2, y3, y4, y5;
  if(r.dx) { //Moving along x-axis
    x1 = r.x;
    x2 = x1 - r.dx;
    x3 = x2 - r.dx;
    x4 = x3 - r.dx;
    x5 = x4 - r.dx;
    y1 = y2 = y3 = y4 = y5 = r.y;
  }
  else {     //Moving along y-axis
    y1 = r.y;
    y2 = y1 - r.dy;
    y3 = y2 - r.dy;
    y4 = y3 - r.dy;
    y5 = y4 - r.dy;
    x1 = x2 = x3 = x4 = x5 = r.x;
  }
  //Drawing rocket front to back, making sure not to overwrite the player pixel that fired it
  addPixelTween({ &leds[XY(x1, y1)], BGCOLOUR, ROCKETCOLOUR }); //Nose
  if(r.age == 1) { //First time drawing rocket, so it's only 2 px ahead of the player
    addPixelTween({ &leds[XY(x2, y2)], ROCKETCOLOUR, BGCOLOUR });
  }
  else {
    leds[XY(x2, y2)] = ROCKETCOLOUR;
    if(r.age == 2) { //Second frame of rocket, so it's maximum 3 px ahead of the player
      addPixelTween({ &leds[XY(x3, y3)], ROCKETCOLOUR, BGCOLOUR });
    }
    else { //Third frame of rocket, rocket is at least 4 frames ahead of player
      addPixelTween({ &leds[XY(x3, y3)], ROCKETCOLOUR, ROCKETFADECOLOUR });
      addPixelTween({ &leds[XY(x4, y4)], ROCKETFADECOLOUR, BGCOLOUR }); //Tail
      leds[XY(x5, y5)] = BGCOLOUR; //Rocket moves two px per frame, so last two must end in BGCOLOUR
    }
  }
}

//Remove the rocket trails from the previous frame's rendering of the rocket
void clearRocketTrail(CRGB leds[], Rocket& r) {
  //If there's an explosion at (r.x, r.y), it automatically wipes out rocket positions (x1, y1) and (x2, y2).
  //We should keep that in mind and avoid tweening when the tween might overlap with an explosion.
  int8_t x2, x3, x4, x5;
  int8_t y2, y3, y4, y5;
  if(r.dx) { //Moving along x-axis
    x2 = r.x - r.dx;
    x3 = x2 - r.dx;
    x4 = x3 - r.dx;
    x5 = x4 - r.dx;
    y2 = y3 = y4 = y5 = r.y;
  }
  else {     //Moving along y-axis
    y2 = r.y - r.dy;
    y3 = y2 - r.dy;
    y4 = y3 - r.dy;
    y5 = y4 - r.dy;
    x2 = x3 = x4 = x5 = r.x;
  }
  //if r.age == 1, there is no cleanup to do because the rocket hasn't ever been rendered yet
  addPixelTween(tweenPixelTo(leds[XY(x3, y3)], BGCOLOUR)); //TODO: Should maybe just set to black if r.age == 2
  if(r.age > 2) {
    addPixelTween(tweenPixelTo(leds[XY(x4, y4)], BGCOLOUR)); //TODO: Should maybe just set to black if r.age == 3
    if(r.age > 3) {
      addPixelTween(tweenPixelTo(leds[XY(x5, y5)], BGCOLOUR)); //TODO: Should maybe just set to black, should test this
    }
  }
}

//Draws an explosion for Rocket r and cleans up the trail of r from the previous frame
void explodeRocket(CRGB leds[], Rocket& r) {
  clearRocketTrail(leds, r);
  explodeAt(r.x, r.y, 1);
}

void updateRockets(CRGB leds[]) {
  uint8_t rid = numRockets;
  while(rid > 0){
    Rocket& r = getRocket(--rid);
    //Move the rocket and check for collisions
    if(stepRocket(leds, r)) {
      explodeRocket(leds, r);
      removeRocket(rid);
      continue;
    }
    //If rocket runs out of bounds
    if(r.x < 0 || r.x >= WIDTH || r.y < 0 || r.y >= HEIGHT-2) {
      removeRocket(rid);
      continue; //TODO: Doesn't catch bounds error off the right side of bottom rown. Also, there will still be some rocket trail left over!
    }
    drawRocket(leds, r);
  }
  
}

#endif

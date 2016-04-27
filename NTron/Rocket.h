#ifndef ROCKET_H
#define ROCKET_H

#include <FastLED.h>
#include "Constants.h"
#include "Player.h"
#include "PixelTweening.h"
#include "Explosion.h"
#include "Powerup.h"

uint8_t tryHitPlayer(uint8_t x, uint8_t y, bool shootToKill);

typedef struct Rocket_ {
  int8_t x;  //Position on screen of the nose of the rocket
  int8_t y;
  int8_t dx; //Direction traveling. Either dx or dy should be 0.
  int8_t dy; //The other should be ±1. Rocket travels 2 pixels per frame.
  uint8_t age; //Counter for number of frames since the rocket was fired.
} Rocket;

enum CollisionState {
  NoCollision = 0,
  Collision = 1,
  OutOfBounds = 2
};

Rocket rockets[MAX_ROCKETS];
int numRockets = 0;

//Call when resetting game
void clearRockets() {
  numRockets = 0;
}

//Fires a rocket from position (fromx, fromy) in the direction (dx, dy).
//dx or dy must be 0, the other must be ±1.
void fireRocket(int8_t fromx, int8_t fromy, int8_t dx, int8_t dy) {
  if (numRockets < MAX_ROCKETS) {
    rockets[numRockets++] = (Rocket) { fromx, fromy, dx, dy, 0 };
  }
}

inline Rocket& getRocket(uint8_t rid) {
  return rockets[rid];
}

//Removes rocket from the rockets array, puts last rocket in its place.
void removeRocket(uint8_t rid) {
  if (rid < --numRockets) {
    rockets[rid] = rockets[numRockets];
  }
}

//Returns true if rocket ran out of bounds
inline bool isRocketOOB(Rocket& r) {
  return r.x < 0 || r.x >= WIDTH || r.y < 0 || r.y >= PLAYABLEHEIGHT;
}

void drawRocket(CRGB leds[], Rocket& r) {
  int8_t x1, x2, x3, x4, x5;
  int8_t y1, y2, y3, y4, y5;
  if (r.dx) { //Moving along x-axis
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
  if (r.age == 1) { //First time drawing rocket, so it's only 2 px ahead of the player
    addPixelTween({ &leds[XY(x2, y2)], ROCKETCOLOUR, BGCOLOUR });
  }
  else {
    leds[XY(x2, y2)] = ROCKETCOLOUR;
    if (r.age == 2) { //Second frame of rocket, so it's maximum 3 px ahead of the player
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
  int8_t x1, x2, x3, x4, x5;
  int8_t y1, y2, y3, y4, y5;
  if (r.dx) { //Moving along x-axis
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
  //if r.age == 1, there is no cleanup to do because the rocket hasn't ever been rendered yet
  if (r.age > 1) {
    addPixelTween(tweenPixelTo(leds[XY(x3, y3)], BGCOLOUR));
    if (r.age > 2) {
      addPixelTween(tweenPixelTo(leds[XY(x4, y4)], BGCOLOUR));
      if (r.age > 3) {
        addPixelTween(tweenPixelTo(leds[XY(x5, y5)], BGCOLOUR));
      }
    }
  }
}

// Turns Rocket r into a triple-rocket!            --->
// If Rocket collides with Powerup, we do this.     --->
// Formation is like the diagram on the right      --->
void tripleRocket(Rocket& r) {
  if (r.dx) {
    fireRocket(r.x - r.dx, r.y - 1, r.dx, r.dy);
    fireRocket(r.x - r.dx, r.y + 1, r.dx, r.dy);
  }
  else {
    fireRocket(r.x - 1, r.y - r.dy, r.dx, r.dy);
    fireRocket(r.x + 1, r.y - r.dy, r.dx, r.dy);
  }
}

// Moves Rocket up to numSteps pixels forward and checks for collisions along the way.
// Returns CollisionState.NoCollision if no collision,
//         CollisionState.Collision   if collision detected,
//         CollisionState.OutOfBounds if out-of bounds.
// Reference variable numSteps returns with the number of steps remaining,
// so the calling function knows at which point the Rocket collided.
CollisionState stepRocket(CRGB leds[], Rocket& r, uint8_t& numSteps) {
  //Test for direct hit with Players before moving the rocket (Players move before Rockets, but )
  if (r.age > 1 && tryHitPlayer(r.x, r.y, false) == 1) return Collision;
  while (numSteps > 0) {
    numSteps--;
    r.x += r.dx;
    r.y += r.dy;
    if (isRocketOOB(r)) return OutOfBounds;
    //Check for collision with next pixel
    if (leds[XYsafe(r.x, r.y)] != BGCOLOUR || //Rocket is moving into an occupied pixel (collision!)
        tryHitPlayer(r.x, r.y, false) == 1) { //Test for direct hit with Players
      return Collision;
    }
  }
  return NoCollision;
}

//Upon moving a Rocket, check if it collided with a Powerup or the trail of a Player.
//Changes the collisionState in response to what it hit and whether that should make it explode.
void checkCollision(CRGB leds[], Rocket& r, CollisionState& collisionState, bool& hitPowerup) {
  if (collisionState == Collision) { //Collision
    //If Rocket hits Powerup, it turns into a triple-rocket!
    if (tryHitPowerup(r.x, r.y)) {
      hitPowerup = true;
      collisionState = NoCollision; //Do not explode upon hitting powerup
    }
    //If rocket almost hits the player (hits the pixel the player is tweening out of), do not explode.
    if (tryHitPlayer(r.x, r.y, false) == 2) {
      collisionState = NoCollision; //Do not explode, did not hit player
    }
  }
}

//Move all rockets and check for collisions
void updateRockets(CRGB leds[]) {
  uint8_t rid = numRockets;
  while (rid > 0) {
    Rocket& r = getRocket(--rid);
    
    //Move the rocket and check for collisions
    r.age++;
    CollisionState collisionState = NoCollision;
    bool hitPowerup = false;
    uint8_t numSteps = 2; //Rockets move 2 pixels per frame
    do {
      collisionState = stepRocket(leds, r, numSteps);
      checkCollision(leds, r, collisionState, hitPowerup);
    } while (numSteps && collisionState == NoCollision);

    //Check to see if we should powerup the Rocket
    if(hitPowerup && collisionState != OutOfBounds) {
      tripleRocket(r);
    }

    switch (collisionState) {
      case NoCollision:
      {
        drawRocket(leds, r);
        break;
      }
      case Collision:
      {
        clearRocketTrail(leds, r);
        if(tryHitPlayer(r.x, r.y, true) == 1) {
          //Hit a player!
          explodeAt(r.x, r.y, 2); //Big explosion
        }
        else {
          //For newly fired rockets, explode in a T-shape so player doesn't run into explosion.
          if (r.age == 1) {
            //Explode in a T-shape. Funny "explosion size" calculation to encode which direction the T faces.
            explodeAt(r.x, r.y, 8 + r.dx + 2*r.dy);
          }
          else {
            //Explode in a + shape
            explodeAt(r.x, r.y, 1);
          }
        }
        removeRocket(rid);
        break;
      }
      case OutOfBounds:
      {
        //Clear the Powerup pixel (if there was one) or the frontmost of the Rocket trail
        addPixelTween(tweenPixelTo(leds[XY(r.x-r.dx, r.y-r.dy)], BGCOLOUR));
        clearRocketTrail(leds, r);
        removeRocket(rid);
        break;
      }
    }
  }
}

#endif

#define USE_OCTOWS2811
#include <OctoWS2811.h>
#include <FastLED.h>
#include <elapsedMillis.h>
#include "Constants.h"
#include "XYmatrix.h"
#include "PixelTweening.h"
#include "TronPlayer.h"
#include "Explosion.h"
#include "Powerup.h"
#include "PowerBar.h"
//TODO: In FastLED lib8tion.h, can use beat8() and similar functions for generating waves

//The last element is never shown, it's a convenience for out-of-bounds writes to go to (see XYsafe())
CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP + 1];

static uint8_t P1LPin = 23;
static uint8_t P1RPin = 22;
static uint8_t P2LPin = 19;
static uint8_t P2RPin = 18;
static uint8_t P1APin = 0;
static uint8_t P1BPin = 1;
static uint8_t P2APin = -1;
static uint8_t P2BPin = -1;

elapsedMillis timeElapsed;
unsigned long msPerFrame = 150;

void resetGame() {
  timeElapsed = 0;
  clearExplosions();
  clearPixelTweens();
  clearPowerBars();
  clearPowerups();
  clearRockets();
  resetPlayer(getPlayer(0),  3,  3,  1, 0);
  resetPlayer(getPlayer(1), 28, 20, -1, 0);
}

void setup() {
  randomSeed(analogRead(17)); //Reading a floating pin for a random seed
  
  TweenIgnoreOOBPixel = &leds[NUM_STRIPS * NUM_LEDS_PER_STRIP]; //Last array element is the out-of-bounds catch-all pixel for XYSafe()
  resetPlayer(initPlayer(0, P1LPin, P1RPin, P1APin, P1BPin), 3, 3, 1, 0);
  resetPlayer(initPlayer(1, P2LPin, P2RPin, P2APin, P2BPin), 28, 20, -1, 0);
  // Pin layouts on the teensy 3:
  // OctoWS2811: 2,14,7,8,6,20,21,5
  FastLED.addLeds<OCTOWS2811,RGB>(leds, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(32);
  FastLED.setDither(0); //This prevents FastLED from doing temporal dithering, which creates noticeable flicker

/*
  for(int j = 0; j < 22; j++) {
    fireRocket(j+10, 20-j, 0, -1);
  }*/
  
  spawnPowerup(leds);
  spawnPowerup(leds);
  spawnPowerup(leds);
  spawnPowerup(leds);
  spawnPowerup(leds);
  spawnPowerup(leds);
  spawnPowerup(leds);
  for(int j = 0; j < 20; j++) {
    leds[XY(30,j)] = FENCECOLOUR;
  }
}

void processFrame() {
  //Move players, lay fence, update power, fire rockets
  updatePlayers(leds);
  //Move rockets and check for collisions
  updateRockets(leds);
  
  //Check for collisions
  for(int pid = 0; pid < NUMPLAYERS; pid++) {
    Player& p = getPlayer(pid);
    if(leds[XY(p.x, p.y)] == (CRGB)BGCOLOUR) { //Player is moving into an empty pixel
      addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], PLAYERCOLOUR));
    }
    else if (hitPowerup(p.x, p.y)) { //Player is moving into a pixel with a powerup
      addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], PLAYERCOLOUR));
      applyPowerup(p);
      spawnPowerup(leds);
      spawnPowerup(leds); //TODO: Remove these, just useful for testing
      spawnPowerup(leds);
      spawnPowerup(leds);
    }
    else {
      explodeAt(p.x, p.y, 1);
      delay(1000);
      resetGame();
      return;
    }
  }
  drawPowerups(leds);
  drawExplosions(leds);
  //TODO, need to check player-player collisions, if(p1.x == p2.x && p1.y == p2.y)
  updatePowerBar(leds, getPlayer(0).power, getPlayer(1).power);
}

void loop() {
  //Check for user input
  checkButtons();
  
  if (timeElapsed >= msPerFrame) { //Time for a new frame
    timeElapsed -= msPerFrame;
    //Finish up the last frame
    finalizeTweens();

    processFrame();
  }

  //Tween pixels while in between frames
  updateFrame(timeElapsed, msPerFrame);
  FastLED.show();
}

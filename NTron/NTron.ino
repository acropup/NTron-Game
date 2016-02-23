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


void setup() {
  TweenIgnoreOOBPixel = &leds[NUM_STRIPS * NUM_LEDS_PER_STRIP]; //Last array element is the out-of-bounds catch-all pixel for XYSafe()
  setScreenDims(WIDTH, HEIGHT);
  initPlayer(0, P1LPin, P1RPin, P1APin, P1BPin, 3, 3);
  initPlayer(1, P2LPin, P2RPin, P2APin, P2BPin, 28, 20);
  // Pin layouts on the teensy 3:
  // OctoWS2811: 2,14,7,8,6,20,21,5
  FastLED.addLeds<OCTOWS2811,RGB>(leds, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(32);
  FastLED.setDither(0); //This prevents FastLED from doing temporal dithering, which creates noticeable flicker

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

void loop() {
  //Check for user input
  checkButtons();
  
  if (timeElapsed >= msPerFrame) { //Time for a new frame
    timeElapsed -= msPerFrame;
    //Finish up the last frame
    finalizeTweens();
    
    for(int pid = 0; pid < 2; pid++) {
      Player& p = getPlayer(pid);
      if(isPlayerFencing(p)) {
        addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], FENCECOLOUR));
      }
      else {
        addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], BGCOLOUR));
      }
    }
    //Move players, update power, fire rockets
    updatePlayers();

    updateRockets(leds);
    
    //Check for collisions
    for(int pid = 0; pid < 2; pid++) {
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
      }
    }
    drawPowerups(leds);
    drawExplosions(leds);
    //TODO, need to check player-player collisions, if(p1.x == p2.x && p1.y == p2.y)
    updatePowerBar(leds, getPlayer(0).power, getPlayer(1).power);
  }

  //Tween pixels while in between frames
  updateFrame(timeElapsed, msPerFrame);
  FastLED.show();
}

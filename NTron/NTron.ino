//TODO: Figure out why plugging in the controller sometimes takes a long time to be recognized

#define FASTLED_INTERNAL //Suppresses the FastLED version "warning" during compilation
#define USE_OCTOWS2811
#include <OctoWS2811.h>
#include <FastLED.h>
#include <elapsedMillis.h>
#include "Constants.h"
#include "XYmatrix.h"
#include "PixelTweening.h"
#include "Player.h"
#include "Explosion.h"
#include "Powerup.h"
#include "PowerBar.h"

//The last element is never shown, it's a convenience for out-of-bounds writes to go to (see XYsafe())
CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP + 1];

elapsedMillis timeElapsed;
int8_t framesUntilReset;
unsigned long msPerFrame = 150;       //This can be changed real-time to change game speed.
unsigned long msControllerDelay = 13; //Screen refresh is 7.8ms and Serial response is 3-5ms. Upper bound is 8+5 = 13ms.
uint8_t screenBrightness = 32;
extern bool pixelTweenEnabled = true;

void resetGame() {
  timeElapsed = 0;
  framesUntilReset = -1;
  memset(leds, 0, WIDTH*HEIGHT*sizeof(CRGB));
  clearExplosions();
  clearPixelTweens();
  clearPowerBars();
  clearPowerups();
  clearRockets();
  resetPlayer(getPlayer(0),  3,  3,  1, 0);
  resetPlayer(getPlayer(1), 28, 18, -1, 0);

  spawnPowerups(leds, NUM_POWERUP_INIT);
}

void setup() {
  randomSeed(analogRead(17)); //Reading a floating pin for a random seed
  
  TweenIgnoreOOBPixel = &leds[NUM_STRIPS * NUM_LEDS_PER_STRIP]; //Last array element is the out-of-bounds catch-all pixel for XYSafe()
  initSerialController();
  initPlayer(0, PLAYER1COLOUR, PLAYER1FENCECOLOUR);
  initPlayer(1, PLAYER2COLOUR, PLAYER2FENCECOLOUR);
  resetGame();
  // Pin layouts on the teensy 3:
  // OctoWS2811: 2,14,7,8,6,20,21,5
  FastLED.addLeds<OCTOWS2811,RGB>(leds, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(screenBrightness);
  FastLED.setDither(0); //This prevents FastLED from doing temporal dithering, which creates noticeable flicker
}

void processFrame() {
  //Move players, lay fence, update power, fire rockets
  updatePlayers(leds);
  
  //Move rockets and check for collisions
  updateRockets(leds);

  Player& p1 = getPlayer(0);
  Player& p2 = getPlayer(1);
  //Check inter-player collision
  if(checkPlayerToPlayerCollision()){
    explodeAt(p1.x, p1.y, 2);
    killPlayer(p1);
    killPlayer(p2);
    Serial.println("Players collided!");
  }
  
  //Check for player collisions
  for(int pid = 0; pid < NUMPLAYERS; pid++) {
    Player& p = getPlayer(pid);
    if (checkPlayerCollision(leds, p)) {
      killPlayer(p);
      explodeAt(p.x, p.y, 1);
      Serial.print("Player ");
      Serial.print(pid+1);
      Serial.println(" collided!");
      //Begin countdown to reset the game
      if(framesUntilReset == -1) {
        framesUntilReset = 10;
      }
    }
  }
  drawPowerups(leds);
  drawExplosions(leds);
  updatePowerBar(leds, p1.power, p2.power);
}


void loop() {
  static bool askController = true; //static = local persistent variable
  
  if (timeElapsed >= msPerFrame - msControllerDelay) {
    //Ask controller for button status in advance of the end of frame, so controller has time to respond
    if (askController) {
      askForButtonStatus();
      askController = false; //Only ask once per frame
    }
    if (timeElapsed >= msPerFrame) { //Time for a new frame
      timeElapsed -= msPerFrame;
      //Finish up the last frame
      finalizeTweens();
      
      //Wait for up to 10ms for the button status to arrive
      int timeLeft = waitForButtonStatus(10);
      
      if (timeLeft == -1) { //Failed to receive button status before the timeout
        //Make the error visible on screen, as a randomly coloured horizontal bar over the PowerBar area
        uint8_t y = HEIGHT - 1;
        CRGB randColour = CHSV((uint8_t)random(255), ~0, ~0);
        for (int8_t x = WIDTH-1; x >= 0; x--) {
          leds[XY(x, y)] = randColour;
        }
    #ifdef DEBUG
        Serial.print("*****Did not receive button states within 10ms!*****");
      }
      else {
        Serial.print("Received button states within ");
        Serial.print(10 - timeLeft);
        Serial.print("ms. Button states: ");
        Serial.println(btnStates, BIN);
    #endif
      }
      if (wasControllerReadSuccessfully()) {
        msPerFrame = getControllerGameSpeed();
        screenBrightness = getControllerGameBrightness();
        pixelTweenEnabled = getControllerGameTweening();
      }
      
      //Take all new button states and apply to each player
      Player& p1 = getPlayer(0);
      Player& p2 = getPlayer(1);
      setPlayerButtonState(p1.buttons, p2.buttons);
      
      //Clear button states if player is dead
      if(!p1.isAlive) p1.buttons.raw = 0;
      if(!p2.isAlive) p2.buttons.raw = 0;
      
      //Reset to ask again next frame
      askController = true;

      #ifdef DEBUG
        debugPrintButtonState(1, p1.buttons);
      #endif

      //Game reset is delayed for a number of frames after player death
      if(framesUntilReset > -1) {
        if (!framesUntilReset--) {
          resetGame();
          return;
        }
      }

      processFrame();
    }
  }

  //Tween pixels while in between frames
  updateFrame(timeElapsed, msPerFrame);
  FastLED.show(); //FastLED takes ~7.8ms to update our screen of 3x256 pixels.
}

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
unsigned long msPerFrame = 150;       //This can be changed real-time to change game speed.
unsigned long msControllerDelay = 13; //Screen refresh is 7.8ms and Serial response is 3-5ms. Upper bound is 8+5 = 13ms.


//int xtest = 12;
void resetGame() {
  timeElapsed = 0;
  memset(leds, 0, WIDTH*HEIGHT*sizeof(CRGB));
  clearExplosions();
  clearPixelTweens();
  clearPowerBars();
  clearPowerups();
  clearRockets();
  resetPlayer(getPlayer(0),  3,  3,  1, 0);
  resetPlayer(getPlayer(1), 28, 20, -1, 0);

  /*for(int i = 0; i < 7; i++) {
    spawnPowerup(leds, 31, 3*i+1);
    leds[XY(21, 3*i+1)] = (CRGB)FENCECOLOUR;
    leds[XY(22, 3*i+1)] = (CRGB)FENCECOLOUR;
    leds[XY(23, 3*i+1)] = (CRGB)FENCECOLOUR;
    leds[XY(24, 3*i+1)] = (CRGB)FENCECOLOUR;
  }*/
  
  spawnPowerups(leds, 7);
  
  //fireRocket(4, 3, 1, 0);
  //fireRocket(xtest++, 11, 0, -1);
  //leds[XY(xtest--, 3)] = (CRGB)FENCECOLOUR;
  //leds[XY(14, 3)] = (CRGB)FENCECOLOUR;
}

void setup() {
  randomSeed(analogRead(17)); //Reading a floating pin for a random seed
  
  TweenIgnoreOOBPixel = &leds[NUM_STRIPS * NUM_LEDS_PER_STRIP]; //Last array element is the out-of-bounds catch-all pixel for XYSafe()
  initSerialController();
  initPlayer(0, PLAYER1COLOUR);
  initPlayer(1, PLAYER2COLOUR);
  resetGame();
  // Pin layouts on the teensy 3:
  // OctoWS2811: 2,14,7,8,6,20,21,5
  FastLED.addLeds<OCTOWS2811,RGB>(leds, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(32);
  FastLED.setDither(0); //This prevents FastLED from doing temporal dithering, which creates noticeable flicker

/*
  for(int j = 0; j < 22; j++) {
    fireRocket(j+10, 20-j, 0, -1);
  }
  for(int j = 0; j < 20; j++) {
    leds[XY(30,j)] = FENCECOLOUR;
  }*/
}

void processFrame() {
  //Move players, lay fence, update power, fire rockets
  updatePlayers(leds);
  //Move rockets and check for collisions
  updateRockets(leds);

  //Check inter-player collision
  if(checkPlayerCollision()){
    Player& p = getPlayer(0);
    explodeAt(p.x, p.y, 2);
    Serial.println("Players collided!");
  }
  //Check for collisions
  for(int pid = 0; pid < NUMPLAYERS; pid++) {
    Player& p = getPlayer(pid);
    if(leds[XY(p.x, p.y)] == (CRGB)BGCOLOUR) { //Player is moving into an empty pixel
      addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], p.colour));
    }
    else if (hitPowerup(p.x, p.y)) { //Player is moving into a pixel with a powerup
      addPixelTween(tweenPixelTo(leds[XY(p.x, p.y)], p.colour));
      applyPowerup(p);
      spawnPowerups(leds, 4);
    }
    else {
      p.isAlive = false;
      explodeAt(p.x, p.y, 1);
      Serial.print("Player ");
      Serial.print(pid+1);
      Serial.println(" collided!");
      if(pid == 0) {
        delay(1000);
        resetGame();
        return;
      }
    }
  }
  drawPowerups(leds);
  drawExplosions(leds);
  //TODO, need to check player-player collisions, if(p1.x == p2.x && p1.y == p2.y)
  updatePowerBar(leds, getPlayer(0).power, getPlayer(1).power);
}

bool askController = true;

void loop() {
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
        //Make the error visible on screen, as a randomly coloured horizontal bar
        uint8_t y = HEIGHT - 2;
        CRGB c = CHSV((uint8_t)random(255), ~0, ~0);
        for (int8_t x = WIDTH-1; x >= 0; x--) {
          leds[XY(x, y)] = c;
        }
      }
      #ifdef DEBUG
        if (timeLeft == -1) {
         Serial.print("*****Did not receive button states within 10ms!*****");
        }
        else {
          Serial.print("Received button states within ");
          Serial.print(10 - timeLeft);
          Serial.print("ms. Button states: ");
          Serial.println(btnStates, BIN);
        }
      #endif
      
      //Take all new button states and apply to each player
      setPlayerButtonState(getPlayer(0).buttons, getPlayer(1).buttons);
      //Reset to ask again next frame
      askController = true;

      #ifdef DEBUG
        PlayerButtonState btns = getPlayer(0).buttons;
        Serial.print("Player 1 button state: ");
        Serial.println(btns.raw, BIN);
        Serial.print("Left: ");
        Serial.println(btns.Left);
        Serial.print("Right: ");
        Serial.println(btns.Right);
        Serial.print("Fence: ");
        Serial.println(btns.Fence);
        Serial.print("Rocket: ");
        Serial.println(btns.Rocket);
      #endif

      processFrame();
    }
  }

  //Tween pixels while in between frames
  updateFrame(timeElapsed, msPerFrame);
  FastLED.show(); //FastLED takes ~7.8ms to update our screen of 3x256 pixels.
}

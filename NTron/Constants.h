#ifndef CONSTANTS_H
#define CONSTANTS_H

#define DEBUG //Defining DEBUG enables extra Serial debug output

#define NUM_LEDS_PER_STRIP 256
#define NUM_STRIPS 3

#define WIDTH  32
#define HEIGHT 24
#define PLAYABLEHEIGHT   (HEIGHT-1)          //HEIGHT minus one row, occupied by the PowerBar

#define FENCECOLOUR      ((CRGB)CRGB::Blue)
#define BGCOLOUR         ((CRGB)CRGB::Black) //Background colour
#define PLAYER1COLOUR    ((CRGB)CRGB::Green)
#define PLAYER1FENCECOLOUR    (CRGB(0, 100, 100)) //((CRGB)CRGB::Cyan) //(CRGB(0, 100, 56))
#define PLAYER2COLOUR    ((CRGB)CRGB::DeepPink)
#define PLAYER2FENCECOLOUR    (CRGB(100, 0, 100)) //((CRGB)CRGB::Magenta) //(CRGB(199, 15, 171))
#define ROCKETCOLOUR     ((CRGB)CRGB::Orange)
#define ROCKETFADECOLOUR ((CRGB)CRGB::Red)

#define POWERUP_HUE_SPEED 30 //Speed of powerup rainbow animation

#define MAX_TWEENS    256 //Maximum number of pixels that can be tweened simultaneously
#define MAX_POWERUPS   32
#define MAX_ROCKETS    32
#define MAX_EXPLOSIONS 32

//Number of powerups that start in play, and rate that they reproduce
#define NUM_POWERUP_INIT     7
#define NUM_POWERUP_REGROWTH 3

//Power and powerup constants. Power storage is in uint8_t, so max value per player is 255
#define FENCE_COST        2
#define ROCKET_COST      16
#define POWERUP_VALUE    16
#define POWER_REGEN_RATE 50 //Percent of a power point regenerated per frame

#endif

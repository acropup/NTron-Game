#ifndef CONSTANTS_H
#define CONSTANTS_H

#define NUM_LEDS_PER_STRIP 256
#define NUM_STRIPS 3

#define WIDTH  32
#define HEIGHT 24

#define FENCECOLOUR      CRGB::Blue
#define BGCOLOUR         CRGB::Black //Background colour
#define PLAYERCOLOUR     CRGB::Green
#define ROCKETCOLOUR     CRGB::Orange
#define ROCKETFADECOLOUR CRGB::Red

#define POWERUP_HUE_SPEED 30 //Speed of powerup rainbow animation

#define MAX_TWEENS    256 //Maximum number of pixels that can be tweened simultaneously
#define MAX_POWERUPS   32
#define MAX_ROCKETS    32
#define MAX_EXPLOSIONS 32

//Power and powerup constants. Power storage is in uint8_t, so max value per player is 255
#define FENCE_COST        2
#define ROCKET_COST      16
#define POWERUP_VALUE    16
#define POWER_REGEN_RATE 50 //Percent of a power point regenerated per frame

#define DEBOUNCE_MS 40

#endif

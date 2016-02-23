#ifndef PIXELTWEEN_H
#define PIXELTWEEN_H

#include<FastLED.h> //CRGB data type defined in FastLED library, pixeltypes.h

typedef struct PixelTween_ {
  CRGB* pixel;      //Pointer to pixel location in buffer
  CRGB fromColour;  //The colour the tween starts at
  CRGB toColour;    //The colour the tween ends at
} PixelTween;

#define MAX_TWEENS 128 //Maximum number of pixels that can be tweened simultaneously
PixelTween tweens[MAX_TWEENS];
int numTweens = 0;

CRGB* TweenIgnoreOOBPixel; //Pointer to out-of-bounds catch-all pixel. No reason to tween it because it's never displayed.

// Adds a PixelTween definition to be tweened over the next frame
inline void addPixelTween(const PixelTween& pt) {
  //Add this tween as long as we still have room and the pixel is not out-of-bounds
  if(numTweens < MAX_TWEENS && pt.pixel != TweenIgnoreOOBPixel) {
    tweens[numTweens++] = pt;
  }
}

// Create PixelTween for pixel from current pixel colour to newColour
inline PixelTween tweenPixelTo(CRGB& pixel, const CRGB& newColour) {
  return (PixelTween) { &pixel, pixel, newColour };
}

// Updates all pixels marked for tweening
void updateFrame(int msSinceLastFrame, int msPerFrame){
  //scale is the fraction of toColour to use, as scale/256
  int scale = map(msSinceLastFrame, 0, msPerFrame, 0, 255);
  int t = numTweens;
  while (t > 0) {
    //Get the next pixel to be tweened
    PixelTween pt = tweens[--t];
    //Set the pixel to a tweened colour between fromColour and toColour
    *(pt.pixel) = pt.fromColour.lerp8(pt.toColour, scale);
  }
}

// Updates all tweened pixels to their final colour, and removes them from the tweens list
void finalizeTweens() {
  while (numTweens > 0) {
    //Get the next pixel to be tweened
    PixelTween pt = tweens[--numTweens];
    //Set the pixel to the ending colour
    *(pt.pixel) = pt.toColour;
  }
}

  //in some function
// addPixelTween(tweenPixelTo(leds[2], CRGB::Red));

#endif

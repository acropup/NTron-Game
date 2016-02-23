#ifndef XYMATRIX_H
#define XYMATRIX_H

#if !defined(WIDTH) || !defined(HEIGHT)
#error Must define matrix WIDTH and HEIGHT
#endif

#if WIDTH == 32 && HEIGHT == 24
/*Converts x,y coordinates into an array index
  For the VHS LED Wall, made of 8x8 matrices with LED strips in serpentine pattern
  4 matrices per row, 3 per column
  LED panels are wired per-row */
uint16_t XY( uint8_t x, uint8_t y) {
  uint16_t i;
  if(x & 0b1) { // If x is odd
    i = x * 8 + 7 - (y & 0b111) + (y>>3)*256;
  }
  else {
    i = x * 8 + (y & 0b111) + (y>>3)*256;
  }
  
  return i;
}
#else
#error function XY(x,y) currently only works for 32x24 matrices
#endif

/*To use XYsafe, define your array with one extra element, such that
  ar[WIDTH*HEIGHT] is the last, valid element, but is not used for
  anything besides eating up an otherwise invalid array lookup.
 */
uint16_t XYsafe( uint8_t x, uint8_t y) {
  if(x < WIDTH && y < HEIGHT)
    return XY(x, y);
  else
    return WIDTH*HEIGHT;
}

#endif

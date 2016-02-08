#ifndef XYMATRIX_H
#define XYMATRIX_H
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
/*
uint16_t XYsafe( uint8_t x, uint8_t y) {
  if(x < 32 || y < 24)
    return XY(x, y);
  else
    return -1;
}*/

#endif

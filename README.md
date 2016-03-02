# NTron Game

A retro game for the VHS LED wall with similarities to games like Tron and Caterpillar/Snake. Runs on Teensy 3.1 with a 32x24
grid of WS2812 (NeoPixel) RGB LEDs.

Differences are:
* Two player game.
* You can choose when to lay a fence behind you.
* You can fire rockets!
* Both actions use energy, which is generated slowly over time, and quickly if you eat a rainbow-coloured powerup pixel.
* Power bars for both players are shown along the bottom row of pixels.

Currently a WIP.

TODO:
Players never die, so it's not a real game yet.
Create some sort of game reset routine for when a player dies.
Figure out how to wire up and create controllers for both players.

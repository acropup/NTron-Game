## *NOTE: This is an old and incomplete version of the game! Choose the other branch for the latest code.*
*This branch contained button controller code which ran directly on the Teensy 3.1, meaning you'd connect the buttons directly to the Teensy that was playing the game and controlling the LEDs. This is simpler, as it requires only one microcontroller, but the Teensy with the OctoWS2811 Adaptor has limited pins available, and you are physically limited to placing your buttons close to the Teensy and the LEDs. If that doesn't matter to you, feel free to refer to this code. The newer, active branch uses a second microcontroller to connect the buttons, and it talks to the Teensy by Serial communication, which can be done over a longer cable.*

# NTron Game

A retro game for the VHS LED wall with similarities to games like Tron and Caterpillar/Snake. Runs on Teensy 3.1 with a 32x24
grid of WS2812 (NeoPixel) RGB LEDs.

Differences are:
* Two player game.
* You can choose when to lay a fence behind you.
* You can fire rockets!
* Both actions use energy, which is generated slowly over time, and quickly if you eat a rainbow-coloured powerup pixel.
* Power bars for both players are shown along the bottom row of pixels.

Currently a WIP. Short demo video [here](https://www.youtube.com/watch?v=VNycczbwMuM).

TODO:
Players never die, so it's not a real game yet.
Create some sort of game reset routine for when a player dies.
Figure out how to wire up and create controllers for both players.

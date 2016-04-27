## SerialGameController

[SerialGameController.ino](SerialGameController.ino) is designed to run on a second Arduino microcontroller and communicate the button state to the Teensy 3.1 (which is running the rest of the game). The controller buttons are wired up to the SerialGameController Arduino, and the Teensy asks it for the button state once per frame. This was done because of the limited number of pins available on the Teensy OctoWS2811 Shield, and because I wanted the button controller to be physically distant from the Teensy and LED wall. For more information, see the [harware documentation](../hardware/README.md).

[SerialByteQuery.py](SerialByteQuery.py) is a Python script for testing the Serial output of SerialGameController.ino on a computer.

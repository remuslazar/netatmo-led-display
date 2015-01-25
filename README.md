Netatmo LED Display
===================

Project Goal
------------

Drive a 64x32 RGA LED Display and display current environmental data
from a Netatmo Weather-Station.

Technical Details
-----------------

The heart of the system is an
[Arduino Yún](http://arduino.cc/en/Main/ArduinoBoardYun?from=Products.ArduinoYUN).

The ATmega32u4 processor will drive the LCD-Display and the Atheros
Linux SOC will talk to the Netatmo API using the
[Netatmo Python Client](https://github.com/remuslazar/netatmo-python).

Using the 5x7 Font (included in the Adafruit GFX Library) the display
is capable of displaying 10x4 Chars (using some spacing between the
chars).


Project Status
--------------

Currently WIP and not fully functional (just displaying some static
demo).

Arduino
-------

This project uses the available LEDMatrix and Adafruit_GXF libraries,
which are both available for free on Github (see below):

```
cd ~/Documents/Arduino/libraries/ # your Arduino libraries dir
git clone https://github.com/Seeed-Studio/Ultrathin_LED_Matrix.git
git clone https://github.com/PaulStoffregen/TimerThree.git
git clone https://github.com/adafruit/Adafruit-GFX-Library.git
```

PCB / Eagle
-----------

A custom Arduino-shield is provided for the connection between the
Arduino and the LED Display. To minimize costs the shield is one-sided
(TOP coper).


References
----------

* [LDP-6432 LCD Matrix Display](http://www.embeddedadventures.com/led_matrix_display_LDP-6432.html)
* [Adafruit GFX Lib Tutorial](https://learn.adafruit.com/downloads/pdf/adafruit-gfx-graphics-library.pdf)
* [Adafruit GFX Arduino Library](https://github.com/adafruit/Adafruit-GFX-Library)
* [Ultrathin LED Matrix Library](https://github.com/Seeed-Studio/Ultrathin_LED_Matrix)
* [GLCD Library](https://github.com/andygock/glcd)

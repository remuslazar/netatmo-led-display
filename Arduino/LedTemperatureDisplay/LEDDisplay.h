/**
 * LED Matrix library for LCD displays with a HUB08 Interface
 * The LED Matrix panel has 64x32 pixels. Several panel can be combined together as a large screen.
 */

#ifndef __LED_DISPLAY_H__
#define __LED_DISPLAY_H__

#include <Adafruit_GFX.h>
#include <stdint.h>
#include <LEDMatrix.h>

class LEDDisplay : public Adafruit_GFX {
 public:

	LEDDisplay(uint8_t width, uint8_t height);

	// init the LCD Display, setup and buffer and width/height
	void init(LEDMatrix *matrix);

	// we implement this function using the LEDMatrix library
	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void setCharCursor(int16_t x, int16_t y);

 private:
	LEDMatrix *matrix;

};

#endif

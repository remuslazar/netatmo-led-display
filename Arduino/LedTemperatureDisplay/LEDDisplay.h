/**
 * LED Matrix library for LCD displays with a HUB08 Interface
 * The LED Matrix panel has 64x32 pixels. Several panel can be combined together as a large screen.
 */

#define DEBUG

#ifndef __LED_DISPLAY_H__
#define __LED_DISPLAY_H__

// currently hardcoded to this size for the LDP-6432 display
#define LED_MATRIX_WIDTH 64
#define LED_MATRIX_HEIGHT 32

// using the 5x7 font supplied by the Adafruit GFX library
// put 1px spacing between the chars
#define LED_DISPLAY_FONT_CHAR_WIDTH 6
#define LED_DISPLAY_FONT_CHAR_HEIGHT 8

#include <Adafruit_GFX.h>
#include <stdint.h>

class LEDDisplay : public Adafruit_GFX {
 public:

	LEDDisplay(void);

	// init
	void begin();

	// we implement this function using the LEDMatrix library
	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void setCharCursor(int16_t x, int16_t y);

#ifdef DEBUG
	volatile long refresh = 0;
#endif
	void updateDisplay();

	void clearScreen();

	// set the display brightness (0-100%)
	void setBrightness(uint8_t brightness);

 private:
	uint8_t matrixbuff[LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT / 8 * 2]; // red/green = 2 bits

	// Counters/pointers for interrupt handler:
	volatile uint8_t row = 0;
	volatile uint8_t *buffptr;
	void setOutputModeForPortAndMask(uint8_t port, uint8_t mask);

};

#endif

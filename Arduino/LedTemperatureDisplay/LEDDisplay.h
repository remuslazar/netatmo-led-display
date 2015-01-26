/**
 * LED Matrix library for LCD displays with a HUB08 Interface
 * The LED Matrix panel has 64x32 pixels. Several panel can be combined together as a large screen.
 */

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

	// LEDDisplay(width, height, A, B, C, D, L, S, OE, R1, R2, G1, G2);
	LEDDisplay(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
	           uint8_t l, uint8_t s, uint8_t oe,
	           uint8_t r1, uint8_t r2,
	           uint8_t g1, uint8_t g2);

	// init the LCD Display, setup and buffer and width/height
	void begin();

	// we implement this function using the LEDMatrix library
	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void setCharCursor(int16_t x, int16_t y);

	volatile long refresh = 0;
	void updateDisplay();

 private:
	uint8_t matrixbuff[LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT / 8];

	// PORT register pointers, pin bitmasks, pin numbers:
	volatile uint8_t
	*latport, *oeport, *addraport, *addrbport, *addrcport, *addrdport,
		*r1port, *r2port, *g1port, *g2port;

	uint8_t
	sclkpin, latpin, oepin, addrapin, addrbpin, addrcpin, addrdpin,
		_sclk, _latch, _oe, _a, _b, _c, _d,
		_r1, _r2, _g1, _g2,
		r1pin, r2pin, g1pin, g2pin;

	// Counters/pointers for interrupt handler:
	volatile uint8_t row = 0;
	volatile uint8_t *buffptr;
};

#endif

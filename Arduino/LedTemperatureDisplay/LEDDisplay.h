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

#define LED_BLACK_COLOR 0
#define LED_RED_COLOR 1
#define LED_GREEN_COLOR 2
#define LED_ORANGE_COLOR 3

// Refresh rate of the display (full refresh for all 32 lines)
// currently the maximum refresh rate is limited by the
// c-implementation and hardware details at about 850Hz
#define LED_REFRESH_RATE 100

#define DEBUG 1

// struct for 8 consecutive pixels
// the MSB being the leftmost pixel
typedef struct {
	uint8_t red;
	uint8_t green;
} display_t;

// a union helper to be able to do memset
union display_word {
	display_t color;
	uint16_t word;
};

class LEDDisplay : public Adafruit_GFX {
 public:

	LEDDisplay(void);

	// init
	void begin(bool useTimer);

	// we implement this function using the LEDMatrix library
	void drawPixel(int16_t x, int16_t y, uint16_t color),
		fillScreen(uint16_t color);

	void setCharCursor(int16_t x, int16_t y);

#ifdef DEBUG
	volatile long refresh = 0;
#endif
	void updateDisplay();

	void clearScreen();

	// Set the display brightness (0-100%)
	void setBrightness(uint8_t brightness);

	void dumpScreen();

 private:
	display_t matrixbuff[LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT / 8];

	// Counters/pointers for interrupt handler:
	volatile uint8_t row = 0;
	volatile display_t *buffptr = matrixbuff;
	void setOutputModeForPortAndMask(uint8_t port, uint8_t mask);
	bool isPwmActive = false;
};

#endif

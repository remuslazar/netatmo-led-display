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

#define DEBUG

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
	void begin();

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

#ifdef DEBUG
	int8_t tcnt4_isr;
#endif

	// commit the composite image from the back-buffer to the front
	// buffer. The switch will be performed asynchronously by the next
	// display refresh cycle (after max. 50ms) and this flag will be
	// cleared.
	void commit();

	boolean ready();

 private:
	// use double buffering to prevent tearing
	display_t matrixbuff[2][LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT / 8];

	// index of the front buffer (visible buffer)
	volatile uint8_t fb;
	volatile boolean switchPlaneRequested = false;

	// Counters/pointers for interrupt handler:
	volatile uint8_t row;
	volatile display_t *buffptr;
};

#endif

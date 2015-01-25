#include "LEDDisplay.h"
#include <TimerThree.h>
#include <LEDMatrix.h>
#include <Adafruit_GFX.h>

#define WIDTH	64
#define HEIGHT	32

// setup the LEDMatrix library for our hardware wire-up
LEDMatrix matrix(7,8,6,4,5,3,1,0);	   // LEDMatrix(LA, LB, LC, LD, EN, R1, STB, SCK);

// we will only use the text functions from the LEDDisplay library,
// for anything else we will use the LEDMatrix library directly
LEDDisplay display(WIDTH, HEIGHT);

uint8_t displaybuf[WIDTH *HEIGHT / 8];			// Display Buffer

// ISR routing will refresh the display every 1.2ms x 16 (19.2ms = 52Hz)
void timer_isr() {
	matrix.scan();
}

void setup() {
	matrix.begin(displaybuf, WIDTH, HEIGHT);
	matrix.clear();

	// setup our LCDDisplay instance
	display.init(&matrix);

	// setup the timer for the isr
	Timer3.initialize(1200);
	Timer3.attachInterrupt(timer_isr);

	// for now just a demo screen
	//                                           12345678901
	//                                          +-----------+
	display.setCharCursor(0,0); display.print(F("34.4° 33.3°"));
	display.setCharCursor(0,1); display.print(F("  49%   50%"));
	display.setCharCursor(0,2); display.print(F(" 898p 1030m"));
	display.setCharCursor(0,3); display.print(F("25.01 14:42"));
	//                                          +-----------+
}

void loop() {
}

#include "LEDDisplay.h"
#include <Adafruit_GFX.h>

#define DEBUG 1

// LEDDisplay(     A, B,C,D, L,S, EN,R1,R2,G1,G2);
LEDDisplay display(9,10,8,6, 1,0,  7, 5, 3, 4, 2);

#ifdef DEBUG
static long lines = 0;
#endif

void setup() {

#ifdef DEBUG
	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}

	// prints title with ending line break
	Serial.println(F("ready()"));
#endif

	// setup our LCDDisplay instance
	display.begin();

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
#ifdef DEBUG
	static uint32_t lastCountTime = 0;
	display.updateDisplay();
	lines++;
#define DISPLAY_REFRESH 1
	if ((millis() - lastCountTime) > DISPLAY_REFRESH * 1000) {
		lastCountTime = millis();
		Serial.print("refresh/s: ");
		Serial.println(lines/16/DISPLAY_REFRESH);
		lines = 0;
	}
#endif
}

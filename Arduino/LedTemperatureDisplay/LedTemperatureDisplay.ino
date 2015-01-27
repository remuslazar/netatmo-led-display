#include "LEDDisplay.h"
#include <Adafruit_GFX.h>

LEDDisplay display;

void setup() {

#define DEBUG

#ifdef DEBUG
	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}

	// prints title with ending line break
	Serial.println(F("ready()"));
#endif

	// setup our LCDDisplay instance
	display.begin(true);
	display.setTextColor(LED_RED_COLOR);

	// for now just a demo screen
	//                                           12345678901
	//                                          +-----------+
	display.setCharCursor(0,0); display.print(F("34.4° 33.3°"));
	display.setCharCursor(0,1); display.print(F("  49%   50%"));
	display.setCharCursor(0,2); display.print(F(" 898p 1030m"));
	display.setCharCursor(0,3); display.print(F("25.01 14:42"));
	//                                          +-----------+

	display.clearScreen();
	display.fillScreen(LED_ORANGE_COLOR);
	//display.clearScreen();
}

void loop() {
#ifdef DEBUG
	static uint32_t lastCountTime = 0;
#define DISPLAY_REFRESH 1
	//display.updateDisplay();
	if ((millis() - lastCountTime) > DISPLAY_REFRESH * 1000) {
		lastCountTime = millis();
		Serial.print("refresh/s: ");
		Serial.println(display.refresh/16/DISPLAY_REFRESH);
		display.refresh = 0;
	}
#endif
}

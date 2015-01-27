#include "LEDDisplay.h"
#include <Adafruit_GFX.h>

#define DEBUG

LEDDisplay display;

void setup() {

	// setup our LCDDisplay instance
	display.begin(true);

	// define the timing for the activity LED (1Hz freq., 10% duty ratio)
#define F_IDLE_LOOP 1
#define PWM_LED .1


	// Setup TIMER3 to drive the activity LED
#define TIMER_R F_CPU * (1+PWM_LED) / 1024 / F_IDLE_LOOP
#define CNT_PWM (1.0-PWM_LED) * TIMER_R

	TCCR3A  = _BV(WGM31) ; // fast PWM, mode 14
	TCCR3B  = _BV(WGM33) | _BV(WGM32) | _BV(CS30) | _BV(CS32); // clk/1024 prescaler
	ICR3 = TIMER_R;

	TIMSK3 |= _BV(TOIE3); // enable irq for timer1
	sei();                // enable global irq

#ifdef DEBUG
	Serial.begin(9600); while (!Serial);
	demoScreen();
	display.dumpScreen();
#endif

}

ISR(TIMER3_OVF_vect, ISR_BLOCK) {
	volatile static bool led = false;

	TCNT3 = led ? CNT_PWM : 0;
	digitalWrite(LED_BUILTIN, led);

	if (!(led = !led)) {
		// do stuff at F_IDLE_LOOP Hz rate

	}

	TIFR3 |= TOV3;
}

void showLedRefreshRate() {
	static uint32_t lastTimestamp = 0;
	uint32_t now = millis();
	double timeElapsed = (now - lastTimestamp) / 1000.0;

	if (Serial) { // if Serial available, print out the refresh rate
		Serial.print("refresh/s: ");
		Serial.println(display.refresh/timeElapsed);
	}
	display.refresh = 0;
	lastTimestamp = now;
}

void loop() {
}

// show some demo screen
void demoScreen() {
	uint32_t then = millis();
	display.setTextColor(LED_RED_COLOR);
	display.clearScreen();
	display.setTextWrap(false);

	//                                           1234567890
	//                                          +-----------+
	display.setCharCursor(0,0); display.print(F("34.4  33.3"));
	display.setCharCursor(0,1); display.print(F("  49%  50%"));
	display.setCharCursor(0,2); display.print(F(" 898 1030m"));
	display.setCharCursor(0,3); display.print(F("25.1 14:42"));
	//                                          +-----------+
}

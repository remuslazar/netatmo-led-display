#include "LEDDisplay.h"
#include <Adafruit_GFX.h>

LEDDisplay display;

void setup() {

	// setup our LCDDisplay instance
	display.begin();

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

	demoScreen();

#ifdef DEBUG
	Serial.begin(9600); while (!Serial);
	display.dumpScreen();
#endif

}

ISR(TIMER3_OVF_vect, ISR_BLOCK) {
	volatile static bool led = false;

	TCNT3 = led ? CNT_PWM : 0;
	digitalWrite(LED_BUILTIN, led);

	TIFR3 |= TOV3; // allow other irqs to be handled

	if (!(led = !led)) {
		// do stuff at F_IDLE_LOOP Hz rate
#ifdef DEBUG
		//if (Serial)
			//showLedRefreshRate();
#endif
	}
}

#ifdef DEBUG
void showLedRefreshRate() {
	static uint32_t lastTimestamp = 0;
	uint32_t now = millis();
	float timeElapsed = (now - lastTimestamp) / 1000.0;

	/* Serial.print("refresh/s: "); */
	/* Serial.println(display.refresh/timeElapsed); */
	//Serial.print("tcnt4_isr: ");
	//Serial.println(display.tcnt4_isr);
	display.refresh = 0;
	lastTimestamp = now;
}
#endif

void loop() {
}

// show some demo screen
void demoScreen() {
	uint32_t then = millis();

	while (!display.ready());

	display.clearScreen();
	display.setTextWrap(false);
	display.setFont(FONT_NORMAL);

	// left display half
	display.setTextColor(LED_ORANGE_COLOR);
	display.setCursor(0,0); display.print(F("23.0\xf7"));
	display.setCursor(0,9); display.print(F(" 54%"));
	display.setCursor(0,18); display.print(F("2014p"));
	display.setFont(FONT_SMALL_DIGITS);
	display.setTextColor(LED_GREEN_COLOR);
	display.setCursor(0,27); display.print(F("28.12.15"));

	// right display half
	display.setFont(FONT_NORMAL);
	display.setTextColor(LED_RED_COLOR);
	display.setCursor(35,0); display.print(F("-1.9\xf7"));
	display.setCursor(35,9); display.print(F(" 89%"));
	display.setCursor(35,18); display.print(F("1020m"));

	display.setFont(FONT_SMALL_DIGITS);
	display.setTextColor(LED_GREEN_COLOR);
	display.setCursor(37,27); display.print(F("12:38:34"));

	display.commit();
}

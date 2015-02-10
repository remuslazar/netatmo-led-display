#include "LEDDisplay.h"
#include <Adafruit_GFX.h>

#ifdef YUN
#include <Process.h>
#endif

// Which pin is the LDR connected to
#define LDR_PIN A0

LEDDisplay display;

struct netatmo_data {
	boolean valid;
	float temperature_indoor;
	float humidity_indoor;
	float co2_indoor;
	float temperature_outdoor;
	float humidity_outdoor;
	float pressure;
};

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

	display.setBrightness(50);
	displayInitScreen();

	// Initialize Bridge
#ifdef YUN
	Bridge.begin();
#endif

#ifdef DEBUG
	Serial.begin(9600);
	//display.dumpScreen();
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
		/* if (Serial) */
		/* 	showLedRefreshRate(); */
#endif
	}
}

struct netatmo_data getNetatmoData() {
	struct netatmo_data data;

#ifdef YUN
	Process p;

#ifdef DEBUG
	Serial.print(F("fetching netatmo data.. "));
#endif

	p.begin(F("/root/netatmo-client.py"));
	p.addParameter(F("-c")); // csv output

	if (p.run() != 0) { // return code != 0
		data.valid = false;
#ifdef DEBUG
		Serial.println(F("error!"));
#endif
		return data;
	}
#ifdef DEBUG
	Serial.println(F("done!"));
#endif

	String s;
	data.temperature_indoor = p.parseFloat();
	data.humidity_indoor = p.parseFloat();
	data.pressure = p.parseFloat();
	data.co2_indoor = p.parseFloat();
	data.temperature_outdoor = p.parseFloat();
	data.humidity_outdoor = p.parseFloat();
#else
	data.temperature_indoor = 23.0;
	data.temperature_outdoor = -2.3;
	data.co2_indoor = 1200;
	data.humidity_indoor = 40;
	data.humidity_outdoor = 60;
	data.pressure = 1010;
#endif

	data.valid = true;
	return data;
}

void displayInitScreen() {
	while (!display.ready());
	display.clearScreen();
	display.setTextWrap(true);
	display.setFont(FONT_NORMAL);
	display.setCursor(7,12);
	display.print(F("booting.."));
	display.commit();
}

boolean displayNetatmoData() {
	struct netatmo_data data = getNetatmoData();
	if (!data.valid) return false;

	while (!display.ready());

	display.clearScreen();
	display.setTextWrap(false);
	display.setFont(FONT_NORMAL);

	/*
	 * left display half
	 */

	// big font for the temperature display
	display.setFont(FONT_LARGE_DIGITS);

	// display the temperature indoor without any decimal places (rounded)
	display.setTextColor(data.temperature_indoor < 15 ? LED_RED_COLOR :
	                     data.temperature_indoor < 18 ? LED_ORANGE_COLOR :
	                     data.temperature_indoor < 23 ? LED_GREEN_COLOR :
	                     data.temperature_indoor < 26 ? LED_ORANGE_COLOR : LED_RED_COLOR);
	display.setCursor(0,0); display.print(data.temperature_indoor,0); display.print(F("\xf7"));

	// normal font for the remaining infos (orange)
	display.setFont(FONT_NORMAL);
	display.setTextColor(LED_ORANGE_COLOR);

	// CO2 indoor
	// use different colors for specific ranged
	display.setTextColor(data.co2_indoor < 1000 ? LED_GREEN_COLOR :
	                     data.co2_indoor < 1500 ? LED_ORANGE_COLOR : LED_RED_COLOR);

	display.setCursor(0,15); display.print(data.co2_indoor,0); display.print('p');

	// reset the color to orange for the remaining infos
	display.setTextColor(LED_GREEN_COLOR);

	// humidity indoor
	display.setTextColor(data.humidity_indoor < 30 ? LED_RED_COLOR :
	                     data.humidity_indoor < 40 ? LED_ORANGE_COLOR :
	                     data.humidity_indoor < 60 ? LED_GREEN_COLOR :
	                     data.humidity_indoor < 70 ? LED_ORANGE_COLOR : LED_RED_COLOR);
	display.setCursor(6,25); display.print(data.humidity_indoor,0); display.print('%');

	/*
	 * right display half
	 */

	// big font for the temperature display
	display.setFont(FONT_LARGE_DIGITS);

	// display the outdoor temperature using one decimal place (23.2)
	display.setTextColor(data.temperature_outdoor < 0 ? LED_RED_COLOR :
	                     data.temperature_outdoor < 15 ? LED_ORANGE_COLOR : LED_GREEN_COLOR);
	display.setCursor(32,0); display.print(data.temperature_outdoor,1); display.print(F("\xf7"));

	// normal font for the remaining infos (orange)
	display.setFont(FONT_NORMAL);
	display.setTextColor(LED_GREEN_COLOR);

	// pressure (outdoor = indoor)
	display.setCursor(35,15); display.print(data.pressure,0); display.print('m');
	// humidity outdoor
	display.setTextColor(data.humidity_outdoor < 70 ? LED_GREEN_COLOR :
	                     data.humidity_outdoor < 85 ? LED_ORANGE_COLOR : LED_RED_COLOR);
	display.setCursor(35+6,25); display.print(data.humidity_outdoor,0); display.print('%');

	display.commit();

#ifdef DEBUG
	//while (!display.ready());
	//display.dumpScreen();
#endif

	return true;
}


#ifdef DEBUG
void showLedRefreshRate() {
	static uint32_t lastTimestamp = 0;
	uint32_t now = millis();
	float timeElapsed = (now - lastTimestamp) / 1000.0;

	/* Serial.print("refresh/s: "); */
	/* Serial.println(display.refresh/timeElapsed); */
	Serial.print("tcnt4_isr: ");
	Serial.println(display.tcnt4_isr);
	display.refresh = 0;
	lastTimestamp = now;
}
#endif


void processLightSensor() {
	const int sample_rate = 2 * 1000; // sample freq := 1/sample_rate
	const float smooth_factor = .15;

	static uint32_t lastTimestamp = 0;
	static double ldr = -1;
	if (!lastTimestamp || millis() - lastTimestamp > sample_rate) { // every sample_rate ms
		lastTimestamp = millis();
		int Vd = analogRead(LDR_PIN);
		if (ldr == -1) {
			ldr = Vd;
		} else {
			double diff = Vd - ldr;
			ldr += diff * smooth_factor;
		}
		Serial.print(F("Vd     = "));Serial.println(Vd);
		Serial.print(F("Vd(s)  = "));Serial.println(ldr);
		Serial.print(F("Bright = "));Serial.println(map(ldr,0,1023,0,100));
		Serial.println();
	}
}

void loop() {
	processLightSensor();
	static uint32_t lastTimestamp = 0;
	if (!lastTimestamp || millis() - lastTimestamp > 30 * 1000) { // every 30 seconds
		lastTimestamp = millis();
		if (!displayNetatmoData()) {
			// something went wrong..
			lastTimestamp += 10 * 1000; // retry after 10 seconds..
		}
	}
}

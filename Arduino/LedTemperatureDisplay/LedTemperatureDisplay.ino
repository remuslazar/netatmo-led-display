#include "LEDDisplay.h"
#include <Adafruit_GFX.h>

/**
 * DEBUG Settings
 */

#define DEBUG

/*
 * Dummy-Mode: the bridge will not be initialized and some dummy
 * environmental data will be used (for development with an Arduino
 * Leonardo board)
 */

#define DUMMY_MODE

/**
 * Some Configuration Options
 */

// Which pin is the LDR connected to
#define LDR_PIN A0

// how often (in seconds) should new data be fetched from the NetAtmo
// server
#define NETATMO_REFRESH_RATE 30

// retry-interval (in seconds) for fetching data from the server
#define NETATMO_RETRY_INTERVAL 10

// sample rate (in seconds) for the ldr (light sensor)
#define BRIGHTNESS_SAMPLE_RATE 5

// # samples for smoothening the raw input value from sensor
#define BRIGHTNESS_SAMPLES_NUM 8


/**
 * Main Arduino Sketch File
 */

#ifndef DUMMY_MODE
#include <Process.h>
#endif

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

	// Initialize Bridge
#ifndef DUMMY_MODE
	Bridge.begin();
#endif

#ifdef DEBUG
	Serial.begin(9600);
	//display.dumpScreen();
#endif

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

	display.setBrightness(50); // set some default brightness (the
							   // brightness will be adjusted automatically
							   // according to the light sensor value
	displayInitScreen();
}

ISR(TIMER3_OVF_vect, ISR_BLOCK) {
	volatile static bool led = false;

	TCNT3 = led ? CNT_PWM : 0;
	digitalWrite(LED_BUILTIN, led);

	TIFR3 |= TOV3; // allow other irqs to be handled

	if (!(led = !led)) {
		// do stuff at F_IDLE_LOOP Hz rate
	}
}

static inline struct netatmo_data getNetatmoData() {
	struct netatmo_data data;

#ifndef DUMMY_MODE
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

void static inline displayInitScreen() {
	while (!display.ready());
	display.clearScreen();
	display.setTextWrap(true);
	display.setFont(FONT_NORMAL);
	display.setCursor(7,12);
	display.print(F("booting.."));
	display.commit();
}

boolean static inline displayNetatmoData() {
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

/**
 * Process the data from the LDR sensor and set the display brightness
 * accordingly
 */

void static inline processLightSensor() {
	const int sampleRate = BRIGHTNESS_SAMPLE_RATE * 1000; // sample freq := 1/sampleRate
	const float smoothSamplesNum = BRIGHTNESS_SAMPLES_NUM;

	static uint32_t triggerTimestamp = 0;
	static double smoothedVal = 500; // start with some middle default value

	if (millis() > triggerTimestamp) {
		int sensorVal = analogRead(LDR_PIN);
		smoothedVal += (double)(sensorVal-smoothedVal) / smoothSamplesNum;

		// calculate the brightness value from 0..100
		uint8_t brightnessValue = map(smoothedVal,0,1023,0,100);

		// set the display brightness accordingly
		display.setBrightness(brightnessValue);

#ifdef DEBUG
		Serial.print(F("sensorVal     = "));Serial.println(sensorVal);
		Serial.print(F("smoothedVal   = "));Serial.println(smoothedVal);
		Serial.print(F("Brightness %  = "));Serial.println(brightnessValue);
		Serial.println();
#endif

		triggerTimestamp = millis() + sampleRate;
	}
}

/**
 * Fetch new data from netatmo and update the display
 */

void static inline processNetatmoRefresh() {
	static uint32_t triggerTimestamp = 0;
	if (millis() > triggerTimestamp) {
		if (displayNetatmoData()) {
			// OK: valid data received
			triggerTimestamp = millis() + NETATMO_REFRESH_RATE * 1000;
		} else {
			// something went wrong..
			triggerTimestamp = millis() + NETATMO_RETRY_INTERVAL * 1000;
		}
	}
}

void loop() {
	processLightSensor();
	processNetatmoRefresh();
}

#include "LEDDisplay.h"
#include "Arduino.h"
#include "cie1931.h"
#include <avr/pgmspace.h>
#include "Hardware.h" // load hardware pin/port mapping

static LEDDisplay *self = NULL;

// returns the psychometric corrected pwm value(0-255) for the given brightness in % (0-100)
int ciePWM(byte percentage) {
	return pgm_read_byte_near(cie + constrain(percentage,0,100));
}

LEDDisplay::LEDDisplay(void) : Adafruit_GFX(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT) {
	self = this; // pointer to the current instance for the ISR
}

void LEDDisplay::setOutputModeForPortAndMask(uint8_t port, uint8_t mask) {
	// code borrowed from wiring_digital.c
	volatile uint8_t *reg, *out;
	reg = portModeRegister(port);
	uint8_t oldSREG = SREG;
	cli();
	*reg |= mask;
	SREG = oldSREG;
}

void LEDDisplay::begin(bool useTimer) {
	// set output mode and initial state for all the pins defined in Hardware.h
	setOutputModeForPortAndMask(LED_HUB08_A_PORT, LED_HUB08_A_MASK);
	setOutputModeForPortAndMask(LED_HUB08_B_PORT, LED_HUB08_B_MASK);
	setOutputModeForPortAndMask(LED_HUB08_C_PORT, LED_HUB08_C_MASK);
	setOutputModeForPortAndMask(LED_HUB08_D_PORT, LED_HUB08_D_MASK);

	setOutputModeForPortAndMask(LED_HUB08_S_PORT, LED_HUB08_S_MASK);
	LED_HUB08_S_PORT &= ~LED_HUB08_S_MASK; // SCLK signal, idle is low

	setOutputModeForPortAndMask(LED_HUB08_L_PORT, LED_HUB08_L_MASK);
	LED_HUB08_L_PORT &= ~LED_HUB08_L_MASK; // Latch signal, idle is low

	setOutputModeForPortAndMask(LED_HUB08_EN_PORT, LED_HUB08_EN_MASK);
	LED_HUB08_EN_PORT |= LED_HUB08_EN_MASK; // high = disable display

	setOutputModeForPortAndMask(LED_HUB08_DATA_PORT, LED_HUB08_R1_MASK);
	setOutputModeForPortAndMask(LED_HUB08_DATA_PORT, LED_HUB08_R2_MASK);
	setOutputModeForPortAndMask(LED_HUB08_DATA_PORT, LED_HUB08_G1_MASK);
	setOutputModeForPortAndMask(LED_HUB08_DATA_PORT, LED_HUB08_G2_MASK);

	buffptr = matrixbuff; // init buffptr (used in the matrix refresh ISR)

	if (useTimer) {
		// hardware timer1 setup: call the ISR at a fixed 100Hz rate
		TCCR1A  = _BV(WGM11); // Mode 14 (fast PWM), OC1A off
		TCCR1B  = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Mode 14, no prescale
		ICR1    = 8000; // 120Hz

		TIMSK1 |= _BV(TOIE1); // enable irq for timer1
		sei();                // enable global irq
	}
}

void LEDDisplay::setBrightness(uint8_t brightness) {
	analogWrite(LED_HUB08_EN_PIN, ciePWM(100-brightness));
}

void LEDDisplay::clearScreen() {
	memset(matrixbuff, 0, sizeof(matrixbuff));
}

// 3 colors: 0: off/dark, 1: red, 2: green, 3: orange
void LEDDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {

	// for the red and green colors we have two consecutive bytes
	display_t *segment = matrixbuff + (x/8 + y * LED_MATRIX_WIDTH / 8) * 2;
	uint8_t  bit = x % 8;

	switch(color) {
	case LED_BLACK_COLOR:
		segment->red &= ~(0x80 >> bit);
		segment->green &= ~(0x80 >> bit);
		break;
	case LED_RED_COLOR:
		segment->red |= 0x80 >> bit;
		segment->green &= ~(0x80 >> bit);
		break;
	case LED_GREEN_COLOR:
		segment->red &= ~(0x80 >> bit);
		segment->green |= 0x80 >> bit;
		break;
	default: // LED_ORANGE_COLOR
		segment->red |= 0x80 >> bit;
		segment->green |= 0x80 >> bit;
	}
}

void LEDDisplay::fillScreen(uint16_t color) {

	union display_word segment;
	switch (color) {
	case LED_BLACK_COLOR:
		segment.color.red = 0;
		segment.color.green = 0;
		break;
	case LED_RED_COLOR:
		segment.color.red = 0xff;
		segment.color.green = 0xff;
		break;
	case LED_GREEN_COLOR:
		segment.color.red = 0;
		segment.color.green = 0xff;
		break;
	default:
		segment.color.red = 0xff;
		segment.color.green = 0xff;
	}
	memset(matrixbuff, segment.word, sizeof(matrixbuff));
}

ISR(TIMER1_OVF_vect, ISR_BLOCK) {
	self->updateDisplay();
	TIFR1 |= TOV1;
}

// A stock Arduino (Leonardo for example, 16MHz crystal) can handle
// (using the implementation below) a maximum of 949 (blank screen) /
// 761 (orange screen, all LEDs on) full screen refresh cycles per
// second.

// Using timer1 we hardwire the refresh rate at about 100Hz, leaving
// enough free CPU resources for the main application.

void LEDDisplay::updateDisplay(void) { // @100Hz rate
	uint8_t tick, tock;
	display_t *ptr1, *ptr2;

	ptr1 = (display_t *)buffptr;
	ptr2 = ptr1 + (16 * LED_MATRIX_WIDTH / 8); // second half of the display

	// we know that the S(CLK) port is not used for the data signals
	// (r1,r2,g1,g2) (see Hardware.h), so we can do the trick here,
	// saving the current port value here and just setting the
	// tick/tock values later on without checking the actual port
	// state (we know that nobody else will clobber the port in
	// between)
	tock = LED_HUB08_S_PORT;
	tick = tock | LED_HUB08_S_MASK;

	// do pre-calculate values for setting the red and green
	// pixels to speedup things later
	// our display uses inverted data lines (0 = on)
	// set all the data lines to high (LED off)
	uint8_t all_off = LED_HUB08_DATA_PORT | LED_HUB08_R1_MASK
	                                      | LED_HUB08_R2_MASK
	                                      | LED_HUB08_G1_MASK
	                                      | LED_HUB08_G2_MASK ;

	// inner loop: speed!
	for (uint8_t i = 0; i < (LED_MATRIX_WIDTH/8); i++) {

		// get the display_data for first half
		display_t pixels_1 = *ptr1++;

		// and second half
		display_t pixels_2 = *ptr2++;

		// we unroll things for speed, use some macros
#define clock(pulse) LED_HUB08_S_PORT = pulse
#define do_col_init() LED_HUB08_DATA_PORT = all_off

		// if the pixel is active, set the corresponding bit to low
#define do_col_r1(mask) if (pixels_1.red & mask) LED_HUB08_DATA_PORT &= ~LED_HUB08_R1_MASK
#define do_col_r2(mask) if (pixels_2.red & mask) LED_HUB08_DATA_PORT &= ~LED_HUB08_R2_MASK
#define do_col_g1(mask) if (pixels_1.green & mask) LED_HUB08_DATA_PORT &= ~LED_HUB08_G1_MASK
#define do_col_g2(mask) if (pixels_2.green & mask) LED_HUB08_DATA_PORT &= ~LED_HUB08_G2_MASK
#define do_col(mask) clock(tick); do_col_init(); do_col_r1(mask); do_col_r2(mask); do_col_g1(mask); do_col_g2(mask); clock(tock)

		// For performance reasons we do use macros and not C loops.
		// We do also hardcode the bit-mask as literal, for the same
		// reason.
		do_col(_BV(7));
		do_col(_BV(6));
		do_col(_BV(5));
		do_col(_BV(4));

		do_col(_BV(3));
		do_col(_BV(2));
		do_col(_BV(1));
		do_col(_BV(0));
	}

	// Because the brightness of the display is adjustable using PWM,
	// we can not assume that the display is currently on or off. We
	// check the current state of the EN output and handle both cases
	// accordingly.

	bool isDisplayActive = LED_HUB08_EN_PORT & LED_HUB08_EN_MASK;

	if (isDisplayActive)
		LED_HUB08_EN_PORT  |= LED_HUB08_EN_MASK;  // Disable LED output

	LED_HUB08_L_PORT |= LED_HUB08_L_MASK; // Latch data (H)

	// setup scan lines
	if(row & 0x1) LED_HUB08_A_PORT |=  LED_HUB08_A_MASK;
	else          LED_HUB08_A_PORT &= ~LED_HUB08_A_MASK;
	if(row & 0x2) LED_HUB08_B_PORT |=  LED_HUB08_B_MASK;
	else          LED_HUB08_B_PORT &= ~LED_HUB08_B_MASK;
	if(row & 0x4) LED_HUB08_C_PORT |=  LED_HUB08_C_MASK;
	else          LED_HUB08_C_PORT &= ~LED_HUB08_C_MASK;
	if(row & 0x8) LED_HUB08_D_PORT |=  LED_HUB08_D_MASK;
	else          LED_HUB08_D_PORT &= ~LED_HUB08_D_MASK;

	LED_HUB08_L_PORT  &= ~LED_HUB08_L_MASK;;  // Latch down (L)

	if (isDisplayActive)
		LED_HUB08_EN_PORT  &= ~LED_HUB08_EN_MASK;   // Re-enable output

	// increment row counter
	if (++row > 15) { // overflow?
		row = 0; // reset row counter and the pointer
		ptr1 = matrixbuff;
	}

	buffptr = ptr1; //+= 16; // save the current pointer to buffptr for next irq

	TCNT1     = 0;        // Restart interrupt timer
#ifdef DEBUG
	refresh++;
#endif
}

void LEDDisplay::setCharCursor(int16_t x, int16_t y) {
	setCursor(x * LED_DISPLAY_FONT_CHAR_WIDTH, y * LED_DISPLAY_FONT_CHAR_HEIGHT);
}

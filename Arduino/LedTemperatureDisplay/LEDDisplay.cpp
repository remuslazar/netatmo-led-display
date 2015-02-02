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

void LEDDisplay::begin() {
	cli(); // disable interrupts for the setup phase
	// set output mode and initial state for all the pins defined in Hardware.h
	LED_HUB08_A_DDR |= LED_HUB08_A_MASK;
	LED_HUB08_B_DDR |= LED_HUB08_B_MASK;
	LED_HUB08_C_DDR |= LED_HUB08_C_MASK;
	LED_HUB08_D_DDR |= LED_HUB08_D_MASK;

	LED_HUB08_S_DDR |= LED_HUB08_S_MASK;
	LED_HUB08_S_PORT &= ~LED_HUB08_S_MASK; // SCLK signal, idle is low

	LED_HUB08_L_DDR |= LED_HUB08_L_MASK;
	LED_HUB08_L_PORT &= ~LED_HUB08_L_MASK; // Latch signal, idle is low

	LED_HUB08_EN_DDR |= LED_HUB08_EN_MASK;
	LED_HUB08_EN_PORT |= LED_HUB08_EN_MASK; // high = disable display

	//LED_HUB08_EN_PORT &= ~LED_HUB08_EN_MASK; // test

	LED_HUB08_DATA_DDR |= ( LED_HUB08_R1_MASK |
	                        LED_HUB08_R2_MASK |
	                        LED_HUB08_G1_MASK |
	                        LED_HUB08_G2_MASK );

	buffptr = matrixbuff; // init buffptr (used in the matrix refresh ISR)
	row = 0; // init row counter


	/*
	  We use the hardware TIMER4 for both the PWM on the OE pin (to
	  allow brightness control) and for the display refresh ISR. To
	  avoid collisions with our ISR, we do synchronize the events, so
	  that we're sure that our ISR isn't running on the duty cycle of
	  the PWM.

	  The Refresh-Rate of the Display should be about 100Hz to avoid
	  flickering. Because of the /16 scan, we need a line scan refresh
	  rate about 16*100Hz = 1.6kHz.

	  A /32 prescaler value leads to a display refresh rate of:
	  16MHz / 32 (prescaler) / 256 (8-bit counter reg) /
	  16 (scan lines) = 122Hz, which is quite fine.

	  We also want the ISR to run in the "off" phase of the PWM (when
	  the display if off anyway, not having to deal with the
	  enabling/disabling of the display during the ISR and to get more
	  speed.. So we setup the timer in the "non-inverting compare
	  output mode" (waveform output is CLEARED on the compare
	  match). CLEARED because our OE signal is INVERTED.

	*/

	// PWM is on OC4D Pin (arduino pin6 = PD7 on leonardo/yun)

	// /32 prescaler
	TCCR4B = _BV(CS41) | _BV(CS42);

	// OC4D pin is PWM, /OC4D pin disconnected
	TCCR4C |= COM4D1;
	TCCR4C &= ~COM4D0;

	TCCR4C |= PWM4D; // enable PWM mode for comparator OCR4D

	// Fast PWM
	TCCR4D &= ~(WGM40 | WGM41);

	TIMSK4 |= _BV(TOIE4); // overflow irq enable

	sei(); // re-enable irq
	setBrightness(100); // max. brightness
}

// The brightnes is controlled by using hardware PWM on the EN signal
void LEDDisplay::setBrightness(uint8_t brightness) {
	int8_t duty = ciePWM(100-brightness);
	// leave a large enough window for the ISR
	OCR4D = constrain(duty,35,255);
}

void LEDDisplay::clearScreen() {
	memset(matrixbuff, 0, sizeof(matrixbuff));
}

// 3 colors: 0: off/dark, 1: red, 2: green, 3: orange
void LEDDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {

	// boundary check
	if ((uint16_t)x >= LED_MATRIX_WIDTH || (uint16_t)y >= LED_MATRIX_HEIGHT) return;

	display_t *segment = matrixbuff + ((uint8_t)x/8 + (uint8_t)y * LED_MATRIX_WIDTH / 8);
	uint8_t bit = x % 8;

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

// dump the screen buffer to the serial console
void LEDDisplay::dumpScreen() {
	display_t *segment = matrixbuff;
	// for now hardcoded to the width of 64
	const char header[] PROGMEM = "   0123456789012345678901234567890123456789012345678901234567890123";

	Serial.println(header);
	for (uint8_t row = 0; row < LED_MATRIX_HEIGHT ; row++) {
	Serial.print(row); Serial.print(row < 10 ? F("  ") : F(" "));
		for (uint8_t col = 0; col < LED_MATRIX_WIDTH; col++) {
			display_t *segment = matrixbuff + (col/8 + row * LED_MATRIX_WIDTH / 8);
			uint8_t  bit = col % 8;
			Serial.print(( (segment->red | segment->green) & (0x80 >> bit)) ? '#' : ' ');
		}
		Serial.print(' ');Serial.println(row);
	}
	Serial.println(header);
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

ISR(TIMER4_OVF_vect, ISR_BLOCK) {
	self->updateDisplay();
	TIFR4 |= TOV4;
}

// A stock Arduino (Leonardo for example, 16MHz crystal) can handle
// (using the implementation below) a maximum of 949 (blank screen) /
// 761 (orange screen, all LEDs on) full screen refresh cycles per
// second.

// Using timer1 we hardwire the refresh rate at about 100Hz, leaving
// enough free CPU resources for the main application.

void LEDDisplay::updateDisplay(void) { // @100Hz rate
	uint8_t tick, tock;
	// we use a local variable here to speed things up (the compiler
	// being able to use cpu registers)
	display_t *ptr1, *ptr2;
	ptr1 = (display_t *)buffptr;
	ptr2 = ptr1 + (16 * LED_MATRIX_WIDTH / 8); // second half of the display

	// we know that the S(CLK) port is not used for the data signals
	// (r1,r2,g1,g2) (see Hardware.h), so we can do the trick here,
	// saving the current port value here and just setting the
	// tick/tock values later on without checking the actual port
	// state (we know that nobody else will clobber the port in
	// between)
	tock = LED_HUB08_S_PORT | LED_HUB08_S_MASK;
	tick = LED_HUB08_S_PORT & ~LED_HUB08_S_MASK;

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

#ifdef DEBUG
	tcnt4_isr = TCNT4; // save the timer4 value for debugging
#endif

	// increment row counter
	if (++row > 15) { // overflow?
		row = 0; // reset row counter and the pointer
		ptr1 = matrixbuff;
#ifdef DEBUG
		refresh++;
#endif
	}

	buffptr = ptr1; //+= 16; // save the current pointer to buffptr for next irq
}

void LEDDisplay::setCharCursor(int16_t x, int16_t y) {
	setCursor(x * LED_DISPLAY_FONT_CHAR_WIDTH, y * LED_DISPLAY_FONT_CHAR_HEIGHT);
}

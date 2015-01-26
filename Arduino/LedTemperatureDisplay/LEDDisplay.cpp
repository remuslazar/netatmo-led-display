#include "LEDDisplay.h"
#include "Arduino.h"

// for performance reasons, the clock pin is only semi-configurable
#define SCLKPORT PORTB
#define DATAPORT

LEDDisplay::LEDDisplay(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                       uint8_t l, uint8_t s, uint8_t oe,
                       uint8_t r1, uint8_t r2,
                       uint8_t g1, uint8_t g2) : Adafruit_GFX(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT) {
	// Look up port registers and pin masks ahead of time,
	// avoids many slow digitalWrite() calls later.
	sclkpin   = digitalPinToBitMask(s);
	latport   = portOutputRegister(digitalPinToPort(l));
	latpin    = digitalPinToBitMask(l);
	oeport    = portOutputRegister(digitalPinToPort(oe));
	oepin     = digitalPinToBitMask(oe);
	addraport = portOutputRegister(digitalPinToPort(a));
	addrapin  = digitalPinToBitMask(a);
	addrbport = portOutputRegister(digitalPinToPort(b));
	addrbpin  = digitalPinToBitMask(b);
	addrcport = portOutputRegister(digitalPinToPort(c));
	addrcpin  = digitalPinToBitMask(c);

	r1port = portOutputRegister(digitalPinToPort(r1));
	r1pin  = digitalPinToBitMask(r1);
	r2port = portOutputRegister(digitalPinToPort(r2));
	r2pin  = digitalPinToBitMask(r2);

	g1port = portOutputRegister(digitalPinToPort(g1));
	g1pin  = digitalPinToBitMask(g1);
	g2port = portOutputRegister(digitalPinToPort(g2));
	g2pin  = digitalPinToBitMask(g2);

	_sclk = s;
	_latch = l;
	_r1 = r1;
	_r2 = r2;
	_g1 = g1;
	_g2 = g2;
	_a = a;
	_b = b;
	_c = c;
	_d = d;

}

void LEDDisplay::begin() {
	// Enable all comm & address pins as outputs, set default states:
	pinMode(_sclk , OUTPUT); SCLKPORT   &= ~sclkpin;  // Low
	pinMode(_latch, OUTPUT); *latport   &= ~latpin;   // Low
	pinMode(_oe   , OUTPUT); *oeport    |= oepin;     // High (disable output)
	pinMode(_a    , OUTPUT); *addraport &= ~addrapin; // Low
	pinMode(_b    , OUTPUT); *addrbport &= ~addrbpin; // Low
	pinMode(_c    , OUTPUT); *addrcport &= ~addrcpin; // Low
	pinMode(_d    , OUTPUT); *addrdport &= ~addrdpin; // Low
	pinMode(_r1   , OUTPUT);
	pinMode(_r2   , OUTPUT);
	pinMode(_g1   , OUTPUT);
	pinMode(_g2   , OUTPUT);
}

// currently we do not support any colors
void LEDDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
	// todo
}

void LEDDisplay::updateDisplay(void) {
	uint8_t  i, tick, tock, *ptr1, *ptr2;
	uint16_t t, duration;

	*oeport  |= oepin;  // Disable LED output during row/plane switchover
	*latport |= latpin; // Latch data loaded during *prior* interrupt

	// buffptr, being 'volatile' type, doesn't take well to optimization.
	// A local register copy can speed some things up:
	ptr1 = (uint8_t *)buffptr;
	ptr2 = ptr1 + (16 * LED_MATRIX_WIDTH / 8);

	if(row & 0x1) *addraport |=  addrapin;
	else          *addraport &= ~addrapin;
	if(row & 0x2) *addrbport |=  addrbpin;
	else          *addrbport &= ~addrbpin;
	if(row & 0x4) *addrcport |=  addrcpin;
	else          *addrcport &= ~addrcpin;
	if(row & 0x8) *addrdport |=  addrdpin;
	else          *addrdport &= ~addrdpin;

	*oeport  &= ~oepin;   // Re-enable output
	*latport &= ~latpin;  // Latch down

	// Record current state of SCLKPORT register, as well as a second
	// copy with the clock bit set.  This makes the innnermost data-
	// pushing loops faster, as they can just set the PORT state and
	// not have to load/modify/store bits every single time.  It's a
	// somewhat rude trick that ONLY works because the interrupt
	// handler is set ISR_BLOCK, halting any other interrupts that
	// might otherwise also be twiddling the port at the same time
	// (else this would clobber them).
	tock = SCLKPORT;
	tick = tock | sclkpin;

	/* #pragma unroll */
	for (uint8_t byte = 0; byte < (LED_MATRIX_WIDTH / 8); byte++) {

		uint8_t pixels1 = *ptr1++;
		uint8_t pixels2 = *ptr2++;

		// apply the same "rude" trick to speedup the port operations
		uint8_t r1on = *r1port | r1pin;
		uint8_t r1off = *r1port & ~r1pin;

		uint8_t r2on = *r2port | r2pin;
		uint8_t r2off = *r2port & ~r2pin;

#define clock(pulse) SCLKPORT = pulse
#define do_col_1(mask) if (pixels1 & (mask)) *r1port = r1on; else *r1port = r1off
#define do_col_2(mask) if (pixels2 & (mask)) *r2port = r2on; else *r2port = r2off
#define do_col(mask) clock(tick); do_col_1(mask); do_col_2(mask); clock(tock)

		// For performance reasons we do use macros and not C loops
		do_col(0x80);
		do_col(0x40);
		do_col(0x20);
		do_col(0x10);
		do_col(0x08);
		do_col(0x04);
		do_col(0x02);
		do_col(0x01);
	}

	buffptr = ptr1; //+= 8;
}

void LEDDisplay::setCharCursor(int16_t x, int16_t y) {
	setCursor(x * LED_DISPLAY_FONT_CHAR_WIDTH, y * LED_DISPLAY_FONT_CHAR_HEIGHT);
}

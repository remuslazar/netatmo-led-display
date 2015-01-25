#include "LEDDisplay.h"
#include "Arduino.h"

#define LED_DISPLAY_FONT_CHAR_WIDTH 6
#define LED_DISPLAY_FONT_CHAR_HEIGHT 8

LEDDisplay::LEDDisplay(uint8_t width, uint8_t height) : Adafruit_GFX(width, height) {
}

void LEDDisplay::init(LEDMatrix *matrix) {
	this->matrix = matrix;
}

// currently we do not support any colors
void LEDDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
	matrix->drawPoint(x,y,(uint8_t)color);
}

void LEDDisplay::setCharCursor(int16_t x, int16_t y) {
	setCursor(x * LED_DISPLAY_FONT_CHAR_WIDTH, y * LED_DISPLAY_FONT_CHAR_HEIGHT);
}

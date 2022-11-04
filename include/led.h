#ifndef LED_HEADER_H
#define LED_HEADER_H

#include <Arduino.h>
#include "constants.h"

extern uint8_t render_buffer[ROWS * COLS];

void clearScreenAndBuffer(uint8_t buffer[ROWS * COLS]);
void setPixel(uint8_t buffer[ROWS * COLS], uint8_t x, uint8_t y, uint8_t value);
void setPixelAtIndex(uint8_t buffer[ROWS * COLS], uint8_t index, uint8_t value);
void renderScreen(uint8_t data[ROWS * COLS]);

#endif
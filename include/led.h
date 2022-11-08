#ifndef LED_HEADER_H
#define LED_HEADER_H

#include <Arduino.h>
#include "constants.h"

extern uint8_t render_buffer[ROWS * COLS];
extern uint8_t positions[ROWS * COLS];
extern int currentRotation;

void clearScreenAndBuffer(uint8_t buffer[ROWS * COLS]);
void drawLine(uint8_t buffer[ROWS * COLS], uint8_t line, bool isHorizontal);
void setPixel(uint8_t buffer[ROWS * COLS], uint8_t x, uint8_t y, uint8_t value);
void setPixelAtIndex(uint8_t buffer[ROWS * COLS], uint8_t index, uint8_t value);
int findPosition(uint8_t count);
void renderScreen(uint8_t data[ROWS * COLS]);
void rotate(uint8_t matrix[ROWS * COLS], bool turnRight);

#endif
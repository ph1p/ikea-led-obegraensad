#pragma once

#include "constants.h"
#include <Arduino.h>

// Bitmaps are a stream of bits, most significant bit first, that fills the
// glyph row by row (e.g. smallNumbers are 4 px wide, so each byte holds two
// rows). They are kept in flash and must be read with pgm_read_byte(), which
// bitmapBit() does.

extern const uint8_t letterU[32];         // 16 px wide
extern const uint8_t letterR[32];         // 16 px wide
extern const uint8_t degreeSymbol[3];     // 4 px wide
extern const uint8_t minusSymbol[3];      // 4 px wide
extern const uint8_t smallNumbers[10][3]; // 4 px wide
extern const uint8_t bigNumbers[10][7];   // 8 px wide

struct Bitmap
{
  const uint8_t *data;
  uint8_t length; // in bytes
};

constexpr uint8_t WEATHER_ICON_COUNT = 7;
extern const Bitmap weatherIcons[WEATHER_ICON_COUNT]; // 16 px wide

struct font
{
  const char *name;
  uint8_t sizeX;
  uint8_t sizeY;
  uint8_t offset; // character code of the first glyph
  uint8_t charCount;
  const uint8_t *data; // charCount glyphs of sizeY bytes, one byte (8 px) per row
};

constexpr uint8_t FONT_COUNT = 2;
extern const font fonts[FONT_COUNT];

inline const uint8_t *fontGlyph(const font &f, uint8_t index)
{
  return f.data + index * f.sizeY;
}

inline uint8_t bitmapBit(const uint8_t *bytes, size_t bit)
{
  return (pgm_read_byte(bytes + (bit >> 3)) >> (7 - (bit & 7))) & 1;
}

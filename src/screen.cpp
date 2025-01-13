#include "screen.h"
#include <SPI.h>

#define TIMER_INTERVAL_US 200
#define GRAY_LEVELS 64 // must be a power of two

using namespace std;

uint8_t Screen_::getCurrentBrightness() const
{
  return brightness_;
}

void Screen_::setBrightness(uint8_t brightness, bool shouldStore)
{
  brightness_ = brightness;

#ifndef ESP8266
  // analogWrite disable the timer1 interrupt on esp8266
  analogWrite(PIN_ENABLE, 255 - brightness);
#endif

#ifdef ENABLE_STORAGE
  if (shouldStore)
  {
    storage.begin("led-wall", false);
    storage.putUInt("brightness", brightness);
    storage.end();
  }
#endif
}

void Screen_::setRenderBuffer(const uint8_t *renderBuffer, bool grays)
{
  if (grays)
  {
    memcpy(renderBuffer_, renderBuffer, ROWS * COLS);
  }
  else
  {
    for (int i = 0; i < ROWS * COLS; i++)
    {
      renderBuffer_[i] = renderBuffer[i] * 255;
    }
  }
}

uint8_t *Screen_::getRenderBuffer()
{
  return renderBuffer_;
}

uint8_t Screen_::getBufferIndex(int index)
{
  return renderBuffer_[index];
}

// CACHE START
void Screen_::clear()
{
  memset(renderBuffer_, 0, ROWS * COLS);
}

bool Screen_::isCacheEmpty() const
{
  for (int i = 0; i < ROWS * COLS; i++)
  {
    if (cache_[i] != 0)
      return false;
  }
  return true;
}

void Screen_::cacheCurrent()
{
  memcpy(cache_, renderBuffer_, ROWS * COLS);
}

void Screen_::restoreCache()
{
  setRenderBuffer(cache_, true);
}
// CACHE END

// STORAGE START
#ifdef ENABLE_STORAGE
void Screen_::loadFromStorage()
{
  storage.begin("led-wall", true);
  setBrightness(255);

  if (currentStatus == NONE)
  {
    clear();
    storage.getBytes("data", renderBuffer_, ROWS * COLS);
  }
  else
  {
    storage.getBytes("data", cache_, ROWS * COLS);
  }

  setBrightness(storage.getUInt("brightness", 255));
  setCurrentRotation(storage.getUInt("rotation", 0));
  storage.end();
}

void Screen_::persist()
{
  storage.begin("led-wall");
  storage.putBytes("data", renderBuffer_, ROWS * COLS);
  storage.putUInt("brightness", brightness_);
  storage.putUInt("rotation", currentRotation);
  storage.end();
}
#endif
// STORAGE END

void Screen_::setup()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  setBrightness(storage.getUInt("brightness", 255));
  Screen.setCurrentRotation(storage.getUInt("rotation", 0));

  storage.end();
#else
  Screen.setCurrentRotation(0);
#endif

  // TODO find proper unused pins for MISO and SS
#ifdef ESP8266
  SPI.pins(PIN_CLOCK, 12, PIN_DATA, 15); // SCLK, MISO, MOSI, SS);
  SPI.begin();
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

  timer1_attachInterrupt(&onScreenTimer);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(100);
#endif

#ifdef ESP32
  SPI.begin(PIN_CLOCK, 34, PIN_DATA, 25); // SCLK, MISO, MOSI, SS
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

  hw_timer_t *Screen_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(Screen_timer, &onScreenTimer, true);
  timerAlarmWrite(Screen_timer, TIMER_INTERVAL_US, true);
  timerAlarmEnable(Screen_timer);
#endif
}

void Screen_::setPixelAtIndex(uint8_t index, uint8_t value, uint8_t brightness)
{
  if (index >= COLS * ROWS)
    return;
  this->renderBuffer_[index] = value <= 0 || brightness <= 0 ? 0 : (brightness > 255 ? 255 : brightness);
}

void Screen_::setPixel(uint8_t x, uint8_t y, uint8_t value, uint8_t brightness)
{
  if (x >= COLS || y >= ROWS)
    return;
  this->renderBuffer_[y * COLS + x] = value <= 0 || brightness <= 0 ? 0 : (brightness > 255 ? 255 : brightness);
}

void Screen_::setCurrentRotation(int rotation, bool shouldPersist)
{
  currentRotation = rotation & 0x3;
  updateTransformations();

#ifdef ENABLE_STORAGE
  if (shouldPersist)
  {
    storage.begin("led-wall", false);
    storage.putUInt("rotation", Screen.currentRotation);
    storage.end();
  }
#endif
}

void Screen_::updateTransformations()
{
  int rotation = currentRotation & 0x3;
  for (int idx = 0; idx < ROWS * COLS; idx++)
  {
    uint8_t pos = positions[idx];
    int x = pos & (COLS - 1);
    int y = pos >> __builtin_ctz(COLS);
    int outX, outY;

    switch (rotation)
    {
    case 1: // 90 degrees
      outX = y;
      outY = COLS - 1 - x;
      break;
    case 2: // 180 degrees
      outX = COLS - 1 - x;
      outY = ROWS - 1 - y;
      break;
    case 3: // 270 degrees
      outX = ROWS - 1 - y;
      outY = x;
      break;
    default: // 0 degrees
      outX = x;
      outY = y;
      break;
    }

    outX = (outX + COLS) % COLS;
    outY = (outY + ROWS) % ROWS;

    transformedPositions_[idx] = outY * COLS + outX;
  }
}

void Screen_::onScreenTimer()
{
  Screen._render();
}

ICACHE_RAM_ATTR void Screen_::_render()
{
  static unsigned long spi_bits[(ROWS * COLS + 8 * sizeof(unsigned long) - 1) / 8 / sizeof(unsigned long)] = {0};
  unsigned char *bits = (unsigned char *)spi_bits;
  memset(bits, 0, ROWS * COLS / 8);

  static unsigned char counter = 0;

  for (int idx = 0; idx < ROWS * COLS; idx++)
  {
    uint8_t brightness = renderBuffer_[transformedPositions_[idx]];
    bits[idx >> 3] |= (brightness > counter ? 0x80 : 0) >> (idx & 7);
  }

  counter += (256 / GRAY_LEVELS);

  digitalWrite(PIN_LATCH, LOW);
  SPI.writeBytes(bits, sizeof(spi_bits));
  digitalWrite(PIN_LATCH, HIGH);

#ifdef ESP8266
  timer1_write(100);
#endif
}

void Screen_::drawLine(int x1, int y1, int x2, int y2, int ledStatus, uint8_t brightness)
{
  int dx = abs(x2 - x1);
  int sx = x1 < x2 ? 1 : -1;
  int dy = abs(y2 - y1);
  int sy = y1 < y2 ? 1 : -1;
  int error = (dx > dy ? dx : -dy) / 2;

  while (x1 != x2 || y1 != y2)
  {
    setPixel(x1, y1, ledStatus, brightness);

    int error2 = error;
    if (error2 > -dx)
    {
      error -= dy;
      x1 += sx;
      setPixel(x1, y1, ledStatus, brightness);
    }

    else if (error2 < dy)
    {
      error += dx;
      y1 += sy;
      setPixel(x1, y1, ledStatus, brightness);
    }
  };
};

void Screen_::drawRectangle(int x, int y, int width, int height, bool fill, int ledStatus, uint8_t brightness)
{
  if (!fill)
  {
    drawLine(x, y, x + width, y, ledStatus, brightness);
    drawLine(x, y + 1, x, y + height - 1, ledStatus, brightness);
    drawLine(x + width, y + 1, x + width, y + height - 1, ledStatus, brightness);
    drawLine(x, y + height - 1, x + width, y + height - 1, ledStatus, brightness);
  }
  else
  {
    for (int i = x; i < x + width; i++)
    {
      drawLine(i, y, i, y + height - 1, ledStatus, brightness);
    }
  }
};

void Screen_::drawCharacter(int x, int y, std::vector<int> bits, int bitCount, uint8_t brightness)
{
  for (int i = 0; i < bits.size(); i += bitCount)
  {
    for (int j = 0; j < bitCount; j++)
    {
      setPixel(x + j, (y + (i / bitCount)), bits[i + j], brightness);
    }
  }
}

std::vector<int> Screen_::readBytes(std::vector<int> bytes)
{
  vector<int> bits;
  int k = 0;

  for (int i = 0; i < bytes.size(); i++)
  {
    for (int j = 8 - 1; j >= 0; j--)
    {
      int b = (bytes[i] >> j) & 1;
      bits.push_back(b);
      k++;
    }
  }

  return bits;
};

void Screen_::drawNumbers(int x, int y, std::vector<int> numbers, uint8_t brightness)
{
  for (int i = 0; i < numbers.size(); i++)
  {
    drawCharacter(x + (i * 5), y, readBytes(smallNumbers[numbers.at(i)]), 4, brightness);
  }
}

void Screen_::drawBigNumbers(int x, int y, std::vector<int> numbers, uint8_t brightness)
{
  for (int i = 0; i < numbers.size(); i++)
  {
    drawCharacter(x + (i * 8), y, readBytes(bigNumbers[numbers.at(i)]), 8, brightness);
  }
}

void Screen_::drawWeather(int x, int y, int weather, uint8_t brightness)
{
  drawCharacter(x, y, readBytes(weatherIcons[weather]), 16, brightness);
}

void Screen_::scrollText(std::string text, int delayTime, uint8_t brightness, uint8_t fontid)
{
  // lets determine the current font
  font currentFont = (fontid < fonts.size()) ? fonts[fontid] : fonts[0];

  int textWidth = text.length() * (currentFont.sizeX + 1); // charsize + space

  for (int i = -ROWS; i < textWidth; i++)
  { // start with negative screen size, so out of screen to the right

    int skippedChars = 0;

    clear();

    for (std::size_t strPos = 0; strPos < text.length(); strPos++)
    { // since i need the pos to calculate, this is the best way to iterate here
      if (text[strPos] == 195)
      {
        // we skip the unicode char indicating special characters
        skippedChars++;
      }
      else
      {
        int xPos = (strPos - skippedChars) * (currentFont.sizeX + 1) - i;

        if (xPos > -6 && xPos < ROWS)
        { // so are we somewhere on screen with the char?
          // ensure that we have a defined char, lets take the first
          uint8_t currentChar = (((text[strPos] - currentFont.offset) < currentFont.data.size()) && (text[strPos] >= currentFont.offset)) ? text[strPos] : currentFont.offset;

          Screen.drawCharacter(xPos, 4, Screen.readBytes(currentFont.data[currentChar - currentFont.offset]), 8);
        }
      }
    }

    delay(delayTime);
  }
}

void Screen_::scrollGraph(std::vector<int> graph, int miny, int maxy, int delayTime, uint8_t brightness)
{
  if (graph.size() <= 0)
  {
    return;
  }

  for (int i = -ROWS; i < (int)graph.size(); i++)
  {
    clear();

    int y1 = -999;

    for (int x = 0; x < ROWS; x++)
    {
      int index = i + x;
      if (index >= 0 && index < graph.size())
      {

        int y2 = ROWS - ((graph[index] - miny + 1) * ROWS) / (maxy - miny + 1);
        // if we are not first pixel on screen
        // and the distance is < 6, so we do not bridge too big gaps
        if (x > 0 && index > 0 && abs(y2 - y1) < 6)
        {
          drawLine(x - 1, y1, x, y2, 1, brightness);
        }
        else
        {
          setPixel(x, y2, 1, brightness);
        }
        y1 = y2; // this value is next values previous value
      }
    }
    delay(delayTime);
  }
}

Screen_ &Screen_::getInstance()
{
  static Screen_ instance;
  return instance;
}

Screen_ &Screen = Screen.getInstance();

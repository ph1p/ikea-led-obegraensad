#include "screen.h"
#include "constants.h"
#include <SPI.h>
#include <algorithm>

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
  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, LOW);
#endif

#ifdef ENABLE_STORAGE
  if (shouldStore)
  {
    storage.begin("led-wall");
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
      renderBuffer_[i] = renderBuffer[i] * MAX_BRIGHTNESS;
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

void Screen_::clear()
{
  memset(renderBuffer_, 0, ROWS * COLS);
}

void Screen_::clearRect(int x, int y, int width, int height)
{
  if (x < 0)
  {
    width += x;
    x = 0;
  }
  if (y < 0)
  {
    height += y;
    y = 0;
  }

  if (x >= COLS || y >= ROWS || width <= 0 || height <= 0)
  {
    return;
  }

  width = std::min(width, COLS - x);
  for (int row = y; row < y + height; row++)
  {
    memset(renderBuffer_ + (row * COLS + x), 0, width);
  }
}

// STORAGE START
void Screen_::loadFromStorage()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);

  clear();
  storage.getBytes("data", renderBuffer_, ROWS * COLS);

  setBrightness(storage.getUInt("brightness", MAX_BRIGHTNESS));
  setCurrentRotation(storage.getUInt("rotation", 0));
  storage.end();
#endif
}

void Screen_::persist()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall");
  storage.putBytes("data", renderBuffer_, ROWS * COLS);
  storage.putUInt("brightness", brightness_);
  storage.putUInt("rotation", currentRotation);
  storage.end();
#endif
}
// STORAGE END

void Screen_::setup()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  setBrightness(storage.getUInt("brightness", MAX_BRIGHTNESS));
  Screen.setCurrentRotation(storage.getUInt("rotation", 0));

  storage.end();
#else
  Screen.setCurrentRotation(0);
#endif

  // TODO find proper unused pins for MISO and SS
#ifdef ESP8266
  // Initialize control pins
  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_LATCH, LOW);

  SPI.pins(PIN_CLOCK, 12, PIN_DATA, 15); // SCLK, MISO, MOSI, SS);
  SPI.begin();
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

  timer1_attachInterrupt(&onScreenTimer);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(100);
#endif

#ifdef ESP32
  // Initialize control pins
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_LATCH, LOW);
  digitalWrite(PIN_ENABLE, LOW);

  SPI.begin(PIN_CLOCK, -1, PIN_DATA, -1); // SCLK, MISO, MOSI, SS (-1 for unused pins)
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

  hw_timer_t *Screen_timer = timerBegin(1000000);
  timerAttachInterrupt(Screen_timer, &onScreenTimer);
  timerAlarm(Screen_timer, TIMER_INTERVAL_US, true, 0);
#endif
}

void Screen_::setPixelAtIndex(uint8_t index, uint8_t value, uint8_t brightness)
{
  if (index >= COLS * ROWS)
    return;
  renderBuffer_[index] =
      value <= 0 || brightness <= 0 ? 0 : (brightness > MAX_BRIGHTNESS ? MAX_BRIGHTNESS : brightness);
}

void Screen_::setPixel(uint8_t x, uint8_t y, uint8_t value, uint8_t brightness)
{
  if (x >= COLS || y >= ROWS)
    return;
  renderBuffer_[y * COLS + x] =
      value <= 0 || brightness <= 0 ? 0 : (brightness > MAX_BRIGHTNESS ? MAX_BRIGHTNESS : brightness);
}

void Screen_::setCurrentRotation(int rotation, bool shouldPersist)
{
  currentRotation = rotation & 0x3;

#ifdef ENABLE_STORAGE
  if (shouldPersist)
  {
    storage.begin("led-wall", false);
    storage.putUInt("rotation", Screen.currentRotation);
    storage.end();
  }
#endif
}

uint8_t *Screen_::getRotatedRenderBuffer()
{
  // No rotation needed - return original buffer directly
  if (currentRotation == 0)
  {
    return renderBuffer_;
  }

  // Copy buffer for rotation
  for (int i = 0; i < ROWS * COLS; i++)
  {
    rotatedRenderBuffer_[i] = renderBuffer_[i];
  }

  rotate();

  return rotatedRenderBuffer_;
}

void Screen_::rotate()
{
  if (currentRotation == 1)
  {
    // 90° clockwise
    uint8_t temp[TOTAL_PIXELS];
    for (int row = 0; row < ROWS; row++)
    {
      for (int col = 0; col < COLS; col++)
      {
        temp[col * COLS + (ROWS - 1 - row)] = rotatedRenderBuffer_[row * COLS + col];
      }
    }
    memcpy(rotatedRenderBuffer_, temp, TOTAL_PIXELS);
  }
  else if (currentRotation == 2)
  {
    // 180°
    for (int i = 0; i < TOTAL_PIXELS / 2; i++)
    {
      swap(rotatedRenderBuffer_[i], rotatedRenderBuffer_[TOTAL_PIXELS - 1 - i]);
    }
  }
  else if (currentRotation == 3)
  {
    // 270° clockwise (or 90° counter-clockwise)
    uint8_t temp[TOTAL_PIXELS];
    for (int row = 0; row < ROWS; row++)
    {
      for (int col = 0; col < COLS; col++)
      {
        temp[(COLS - 1 - col) * COLS + row] = rotatedRenderBuffer_[row * COLS + col];
      }
    }
    memcpy(rotatedRenderBuffer_, temp, TOTAL_PIXELS);
  }
}

void Screen_::onScreenTimer()
{
  Screen._render();
}

ICACHE_RAM_ATTR void Screen_::_render()
{
  const auto buf = getRotatedRenderBuffer();

  // SPI data needs to be 32-bit aligned, round up before divide
  static unsigned long
      spi_bits[(ROWS * COLS + 8 * sizeof(unsigned long) - 1) / 8 / sizeof(unsigned long)] = {0};
  unsigned char *bits = (unsigned char *)spi_bits;
  memset(bits, 0, ROWS * COLS / 8);

  static unsigned char counter = 0;

  for (int idx = 0; idx < ROWS * COLS; idx++)
  {
    uint16_t scaledValue = ((uint16_t)buf[positions[idx]] * brightness_) / MAX_BRIGHTNESS;
    bits[idx >> 3] |= (scaledValue > counter ? 0x80 : 0) >> (idx & 7);
  }

  counter += ((MAX_BRIGHTNESS + 1) / GRAY_LEVELS);

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

void Screen_::drawRectangle(int x,
                            int y,
                            int width,
                            int height,
                            bool fill,
                            int ledStatus,
                            uint8_t brightness)
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

void Screen_::drawCharacter(int x,
                            int y,
                            const std::vector<int> &bits,
                            int bitCount,
                            uint8_t brightness)
{
  for (int i = 0; i < bits.size(); i += bitCount)
  {
    for (int j = 0; j < bitCount; j++)
    {
      setPixel(x + j, (y + (i / bitCount)), bits[i + j], brightness);
    }
  }
}

std::vector<int> Screen_::readBytes(const std::vector<int> &bytes)
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
}

void Screen_::drawNumbers(int x, int y, const std::vector<int> &numbers, uint8_t brightness)
{
  for (int i = 0; i < numbers.size(); i++)
  {
    drawCharacter(x + (i * 5), y, readBytes(smallNumbers[numbers.at(i)]), 4, brightness);
  }
}

void Screen_::drawBigNumbers(int x, int y, const std::vector<int> &numbers, uint8_t brightness)
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

void Screen_::scrollText(const std::string &text, int delayTime, uint8_t brightness, uint8_t fontid)
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
          uint8_t currentChar = (((text[strPos] - currentFont.offset) < currentFont.data.size()) &&
                                 (text[strPos] >= currentFont.offset))
                                    ? text[strPos]
                                    : currentFont.offset;

          Screen.drawCharacter(xPos,
                               4,
                               Screen.readBytes(currentFont.data[currentChar - currentFont.offset]),
                               8);
        }
      }
    }

#ifdef ESP32
    vTaskDelay(pdMS_TO_TICKS(delayTime));
#else
    delay(delayTime);
#endif
  }
}

void Screen_::scrollGraph(const std::vector<int> &graph,
                          int miny,
                          int maxy,
                          int delayTime,
                          uint8_t brightness)
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
#ifdef ESP32
    vTaskDelay(pdMS_TO_TICKS(delayTime));
#else
    delay(delayTime);
#endif
  }
}

Screen_ &Screen_::getInstance()
{
  static Screen_ instance;
  return instance;
}

Screen_ &Screen = Screen.getInstance();

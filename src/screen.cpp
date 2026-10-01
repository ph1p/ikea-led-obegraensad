#include "screen.h"
#include "constants.h"
#include <SPI.h>
#include <algorithm>
#ifdef ESP32
#include "hal/gpio_ll.h"
#endif

#define TIMER_INTERVAL_US 200

using namespace std;

static constexpr uint8_t reverseBits(uint8_t value, uint8_t bits)
{
  return bits == 0 ? 0 : (uint8_t)(((value & 1) << (bits - 1)) | reverseBits(value >> 1, bits - 1));
}

static constexpr uint8_t GRAY_LEVEL_BITS = __builtin_ctz(GRAY_LEVELS);
static constexpr uint8_t GRAY_LEVEL_STEP = (MAX_BRIGHTNESS + 1) / GRAY_LEVELS;

struct PwmThresholds
{
  uint8_t values[GRAY_LEVELS];
  constexpr PwmThresholds() : values()
  {
    for (uint8_t i = 0; i < GRAY_LEVELS; i++)
    {
      values[i] = reverseBits(i, GRAY_LEVEL_BITS) * GRAY_LEVEL_STEP;
    }
  }
};

static const PwmThresholds pwmThresholds;

#ifdef ESP32
static portMUX_TYPE frameMux = portMUX_INITIALIZER_UNLOCKED;
#define SCREEN_LOCK() portENTER_CRITICAL(&frameMux)
#define SCREEN_UNLOCK() portEXIT_CRITICAL(&frameMux)
#else
#define SCREEN_LOCK() noInterrupts()
#define SCREEN_UNLOCK() interrupts()
#endif

uint8_t Screen_::getCurrentBrightness() const
{
  return brightness_;
}

uint8_t Screen_::getBaseBrightness() const
{
  return baseBrightness_;
}

void Screen_::setBaseBrightness(uint8_t brightness, bool shouldStore)
{
  baseBrightness_ = brightness;

#ifdef ENABLE_STORAGE
  if (shouldStore)
  {
    storage.begin("led-wall");
    storage.putUInt("brightness", brightness);
    storage.end();
  }
#endif
}

void Screen_::setDisplayedBrightness(uint8_t brightness)
{
  brightness_ = brightness;

#ifndef ESP8266
  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, LOW);
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

  setBaseBrightness(storage.getUInt("brightness", MAX_BRIGHTNESS));
  setDisplayedBrightness(getBaseBrightness());
  setCurrentRotation(storage.getUInt("rotation", 0));
  storage.end();
#endif
}

void Screen_::persist()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall");
  storage.putBytes("data", renderBuffer_, ROWS * COLS);
  storage.putUInt("brightness", baseBrightness_);
  storage.putUInt("rotation", currentRotation);
  storage.end();
#endif
}
// STORAGE END

void Screen_::setup()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  setBaseBrightness(storage.getUInt("brightness", MAX_BRIGHTNESS));
  setDisplayedBrightness(getBaseBrightness());
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

void Screen_::buildPixelMap(uint8_t rotation)
{
  // source pixel for every rotated position
  uint8_t source[TOTAL_PIXELS];
  for (int row = 0; row < ROWS; row++)
  {
    for (int col = 0; col < COLS; col++)
    {
      int target;
      switch (rotation)
      {
      case 1: // 90° clockwise
        target = col * COLS + (ROWS - 1 - row);
        break;
      case 2: // 180°
        target = TOTAL_PIXELS - 1 - (row * COLS + col);
        break;
      case 3: // 270° clockwise (or 90° counter-clockwise)
        target = (COLS - 1 - col) * COLS + row;
        break;
      default:
        target = row * COLS + col;
        break;
      }
      source[target] = row * COLS + col;
    }
  }

  for (int idx = 0; idx < TOTAL_PIXELS; idx++)
  {
    pixelMap_[idx] = source[positions[idx]];
  }
  pixelMapRotation_ = rotation;
}

void Screen_::present()
{
  const bool updating = currentStatus == UPDATE;
  // the update screen is drawn unrotated and without grays
  const uint8_t rotation = updating ? 0 : currentRotation;
  const uint8_t brightness = brightness_;
  const int16_t key = (updating << 10) | (rotation << 8) | brightness;

  if (key == presentedKey_ && memcmp(renderBuffer_, presentedBuffer_, TOTAL_PIXELS) == 0)
  {
    return;
  }

  if (rotation != pixelMapRotation_)
  {
    buildPixelMap(rotation);
  }

  uint8_t frame[TOTAL_PIXELS];
  for (int idx = 0; idx < TOTAL_PIXELS; idx++)
  {
    const uint8_t pixelValue = renderBuffer_[pixelMap_[idx]];
    if (updating)
    {
      frame[idx] = pixelValue > 0 ? MAX_BRIGHTNESS : 0;
      continue;
    }
    uint8_t scaledValue = ((uint16_t)pixelValue * brightness) / MAX_BRIGHTNESS;
    frame[idx] = (brightness > 0 && pixelValue > 0 && scaledValue == 0) ? 1 : scaledValue;
  }

  // withdraw a pending frame, so the timer cannot swap to the back planes
  // while they are rewritten
  SCREEN_LOCK();
  planesReady_ = false;
  SCREEN_UNLOCK();

  for (int tick = 0; tick < GRAY_LEVELS; tick++)
  {
    const uint8_t counter = pwmThresholds.values[tick];
    uint8_t *bits = (uint8_t *)planes_[frontPlanes_ ^ 1][tick];
    for (int i = 0; i < TOTAL_PIXELS / 8; i++)
    {
      uint8_t byte = 0;
      for (int bit = 0; bit < 8; bit++)
      {
        // each pixel gets a fixed phase offset (a multiple of the threshold step,
        // spread over the cycle) so LEDs don't all switch on at the same time;
        // the on-time per pixel is unchanged, but the current peaks drop
        const int idx = i * 8 + bit;
        const uint8_t position = counter + (uint8_t)(idx * 37 * GRAY_LEVEL_STEP);
        byte = (byte << 1) | (frame[idx] > position);
      }
      bits[i] = byte;
    }
  }

  SCREEN_LOCK();
  planesReady_ = true;
  memcpy(presentedBuffer_, renderBuffer_, TOTAL_PIXELS);
  frameCounter_++;
  SCREEN_UNLOCK();
  presentedKey_ = key;
}

void Screen_::presentAndWait(uint32_t ms)
{
  present();
#ifdef ESP32
  vTaskDelay(pdMS_TO_TICKS(ms));
#else
  delay(ms);
#endif
}

uint32_t Screen_::copyPresentedFrame(uint8_t *dst)
{
  SCREEN_LOCK();
  memcpy(dst, presentedBuffer_, TOTAL_PIXELS);
  const uint32_t counter = frameCounter_;
  SCREEN_UNLOCK();
  return counter;
}

IRAM_ATTR void Screen_::onScreenTimer()
{
  Screen._render();
}

IRAM_ATTR void Screen_::_render()
{
  // the bit planes visit the PWM thresholds in bit-reversed order: a pixel
  // still gets the same number of on-ticks per cycle, but they are spread out
  // instead of bunched at the start, so dim pixels pulse several times per cycle
  static uint8_t tick = 0;

  // only swap frames between PWM cycles, a frame changing mid cycle shows up
  // as a short brightness glitch on the changed pixels
  if (tick == 0 && planesReady_)
  {
#ifdef ESP32
    portENTER_CRITICAL_ISR(&frameMux);
#endif
    if (planesReady_)
    {
      frontPlanes_ ^= 1;
      planesReady_ = false;
    }
#ifdef ESP32
    portEXIT_CRITICAL_ISR(&frameMux);
#endif
  }

  const uint8_t *bits = (const uint8_t *)planes_[frontPlanes_][tick];
  tick = (tick + 1) & (GRAY_LEVELS - 1);

#ifdef ESP32
  // digitalWrite() looks the pin up on every call, the register write keeps
  // the interrupt short and its timing steady
  gpio_ll_set_level(&GPIO, PIN_LATCH, 0);
  SPI.writeBytes(bits, TOTAL_PIXELS / 8);
  gpio_ll_set_level(&GPIO, PIN_LATCH, 1);
#else
  digitalWrite(PIN_LATCH, LOW);
  SPI.writeBytes(bits, TOTAL_PIXELS / 8);
  digitalWrite(PIN_LATCH, HIGH);
#endif
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

    presentAndWait(delayTime);
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
    presentAndWait(delayTime);
  }
}

Screen_ &Screen_::getInstance()
{
  static Screen_ instance;
  return instance;
}

Screen_ &Screen = Screen.getInstance();

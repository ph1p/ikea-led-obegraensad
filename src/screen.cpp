#include "screen.h"

using namespace std;

void Screen_::setRenderBuffer(const uint8_t *renderBuffer)
{
  for (int i = 0; i < ROWS * COLS; i++)
  {
    this->renderBuffer_[i] = renderBuffer[i];
  }
}

const uint8_t *Screen_::getRenderBuffer() const
{
  return this->renderBuffer_;
}

uint8_t Screen_::getBufferIndex(int index)
{
  return this->renderBuffer_[index];
}

void Screen_::loadFromStorage()
{
  this->clear();
  // https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
  storage.begin("led-wall", false);
  storage.getBytes("data", this->renderBuffer_, ROWS * COLS);
  storage.end();
  this->render();
}

void Screen_::rotate(int degree)
{
  uint8_t SIZE = 16;
  uint8_t tempPositions[ROWS * COLS];

  bool turnRight = degree > 0;
  int rotations = abs(floor(degree / 90));

  this->currentRotation = degree;
  if (this->currentRotation <= -360 || this->currentRotation >= 360)
  {
    this->currentRotation = 0;
  }

  if (rotations == 0)
  {
    this->currentRotation = 0;

    for (int i = 0; i < ROWS * COLS; i++)
    {
      this->positions[i] = defaultPositions[i];
    }
  }
  else
  {
    for (int i = 0; i < ROWS * COLS; i++)
    {
      tempPositions[i] = this->defaultPositions[i];
    }
    for (int r = 0; r < rotations; r++)
    {
      for (uint8_t i = 0; i < SIZE; i++)
      {
        for (uint8_t j = i; j < SIZE; j++)
        {
          uint8_t temp = tempPositions[i * SIZE + j];
          tempPositions[i * SIZE + j] = tempPositions[j * SIZE + i];
          tempPositions[j * SIZE + i] = temp;
        }
      }

      for (uint8_t i = 0; i < SIZE; i++)
      {
        uint8_t col1 = 0;
        uint8_t col2 = SIZE - 1;
        while (col1 < col2)
        {
          uint8_t index1 = i * SIZE + col1;
          uint8_t index2 = i * SIZE + col2;
          if (turnRight)
          {
            index1 = col1 * SIZE + i;
            index2 = col2 * SIZE + i;
          }

          uint8_t temp = tempPositions[index1];
          tempPositions[index1] = tempPositions[index2];
          tempPositions[index2] = temp;
          col1++;
          col2--;
        }
      }
    }

    for (int i = 0; i < ROWS * COLS; i++)
    {
      this->positions[i] = tempPositions[i];
    }
  }
};

void Screen_::clear()
{
  memset(this->renderBuffer_, 0, ROWS * COLS);
  this->render();
}

void Screen_::persist()
{
  storage.begin("led-wall", false);
  storage.putBytes("data", this->renderBuffer_, ROWS * COLS);
  storage.end();
}

void Screen_::setPixelAtIndex(uint8_t index, uint8_t value)
{
  if (index >= 0 && index < COLS * ROWS)
  {
    this->renderBuffer_[index] = value;
  }
}

void Screen_::setPixel(uint8_t x, uint8_t y, uint8_t value)
{
  int max_x = x;
  if (x >= ROWS)
  {
    max_x = 15;
  }
  int max_y = y;
  if (y >= COLS)
  {
    max_y = 15;
  }

  this->renderBuffer_[max_y * 16 + max_x] = value;
}

int Screen_::findPosition(uint8_t count)
{
  uint8_t wantedpos = 0;
  for (uint8_t i = 0; i < ROWS * COLS; i++)
  {
    if (count == this->positions[i])
    {
      wantedpos = i;
      break;
    }
  }

  return wantedpos;
}

void Screen_::render()
{
  for (uint8_t row = 0; row < ROWS; row++)
  {
    for (uint8_t col = 0; col < COLS; col++)
    {
      digitalWrite(PIN_DATA, this->renderBuffer_[this->findPosition(row * 16 + col)]);
      digitalWrite(PIN_CLOCK, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_CLOCK, LOW);
    }
  }

  digitalWrite(PIN_LATCH, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_LATCH, LOW);
  delayMicroseconds(10);
}

void Screen_::cacheCurrent()
{
  std::copy(this->renderBuffer_, this->renderBuffer_ + (COLS * ROWS), this->cache);
}

void Screen_::restoreCache()
{
  std::copy(this->cache, this->cache + (COLS * ROWS), this->renderBuffer_);
}

void Screen_::drawLine(int x1, int y1, int x2, int y2, int ledStatus)
{
  int dx = abs(x2 - x1);
  int sx = x1 < x2 ? 1 : -1;
  int dy = abs(y2 - y1);
  int sy = y1 < y2 ? 1 : -1;
  int error = (dx > dy ? dx : -dy) / 2;

  while (x1 != x2 || y1 != y2)
  {
    this->setPixel(x1, y1, ledStatus);

    int error2 = error;
    if (error2 > -dx)
    {
      error -= dy;
      x1 += sx;
      setPixel(x1, y1, ledStatus);
    }

    else if (error2 < dy)
    {
      error += dx;
      y1 += sy;
      setPixel(x1, y1, ledStatus);
    }
  };
};

void Screen_::drawRectangle(int x, int y, int width, int height, bool fill, int ledStatus)
{
  if (!fill)
  {
    this->drawLine(x, y, x + width, y, ledStatus);
    this->drawLine(x, y + 1, x, y + height - 1, ledStatus);
    this->drawLine(x + width, y + 1, x + width, y + height - 1, ledStatus);
    this->drawLine(x, y + height - 1, x + width, y + height - 1, ledStatus);
  }
  else
  {
    for (int i = x; i < x + width; i++)
    {
      this->drawLine(i, y, i, y + height - 1, ledStatus);
    }
  }
};

void Screen_::drawCharacter(int x, int y, std::vector<int> bits, int width)
{
  for (int i = 0; i < bits.size(); i += width)
  {
    for (int j = 0; j < width; j++)
    {
      this->setPixel(x + j, (y + (i / width)), bits[i + j]);
    }
  }
}

std::vector<int> Screen_::readBytes(std::string bytes, int width)
{
  std::vector<int> bits;
  int k = 0;

  for (int i = 0; i < bytes.length(); i++)
  {
    for (int j = width - 1; j >= 0; j--)
    {
      bits.push_back((bytes[i] >> j) & 1);
      k++;
    }
  }

  return bits;
};

void Screen_::drawNumbers(int x, int y, std::vector<int> numbers)
{
  for (int i = 0; i < numbers.size(); i++)
  {
    this->drawCharacter(x + (i * 5), y, this->readBytes(smallNumbers[numbers.at(i)], 4), 4);
  }
}

Screen_ &Screen_::getInstance()
{
  static Screen_ instance;
  return instance;
}

Screen_ &Screen = Screen.getInstance();
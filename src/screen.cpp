#include "screen.h"

void Screen_::rotate(uint8_t matrix[ROWS * COLS], int degree)
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
      matrix[i] = defaultPositions[i];
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
      matrix[i] = tempPositions[i];
    }
  }
};

void Screen_::clearScreenAndBuffer(uint8_t buffer[ROWS * COLS])
{
  memset(buffer, 0, ROWS * COLS);
  this->renderScreen(buffer);
}

void Screen_::setPixelAtIndex(uint8_t buffer[ROWS * COLS], uint8_t index, uint8_t value)
{
  if (index >= 0 && index < COLS * ROWS)
  {
    buffer[index] = value;
  }
}

void Screen_::setPixel(uint8_t buffer[ROWS * COLS], uint8_t x, uint8_t y, uint8_t value)
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

  buffer[max_y * 16 + max_x] = value;
}

int Screen_::findPosition(uint8_t count)
{
  uint8_t wantedpos = 0;
  for (uint8_t i = 0; i < sizeof(this->positions); i++)
  {
    if (count == this->positions[i])
    {
      wantedpos = i;
      break;
    }
  }

  return wantedpos;
}

void Screen_::drawLine(uint8_t buffer[ROWS * COLS], uint8_t line, bool isHorizontal)
{
  for (uint8_t i = 0; i < 16; i++)
  {
    buffer[!isHorizontal ? line * 16 + i : i * 16 + line] = 1;
  }
};

void Screen_::renderScreen(uint8_t data[ROWS * COLS])
{
  for (uint8_t row = 0; row < ROWS; row++)
  {
    for (uint8_t col = 0; col < COLS; col++)
    {
      digitalWrite(PIN_DATA, data[this->findPosition(row * 16 + col)]);
      delayMicroseconds(10);
      digitalWrite(PIN_CLOCK, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_CLOCK, LOW);
    }
  }

  digitalWrite(PIN_LATCH, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_LATCH, LOW);
}

Screen_ &Screen_::getInstance()
{
  static Screen_ instance;
  return instance;
}

Screen_ &Screen = Screen.getInstance();
#include "plugins/FacePlugin.h"

void FacePlugin::setPx(int x, int y, uint8_t brightness)
{
  if (x < 0 || x > 15 || y < 0 || y > 15)
  {
    return;
  }
  Screen.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y), 1, brightness);
}

void FacePlugin::drawBlock(int x, int y)
{
  for (int dy = 0; dy < 3; dy++)
  {
    for (int dx = 0; dx < 3; dx++)
    {
      setPx(x + dx, y + dy, BRIGHT_FULL);
    }
  }
}

void FacePlugin::drawOutline4x4(int x, int y)
{
  for (int i = 0; i < 4; i++)
  {
    setPx(x + i, y, BRIGHT_FULL);
    setPx(x + i, y + 3, BRIGHT_FULL);
    setPx(x, y + i, BRIGHT_FULL);
    setPx(x + 3, y + i, BRIGHT_FULL);
  }
}

void FacePlugin::drawPlus(int cx, int cy)
{
  setPx(cx, cy, BRIGHT_FULL);
  setPx(cx - 1, cy, BRIGHT_FULL);
  setPx(cx + 1, cy, BRIGHT_FULL);
  setPx(cx, cy - 1, BRIGHT_FULL);
  setPx(cx, cy + 1, BRIGHT_FULL);
}

void FacePlugin::clearTears()
{
  for (uint8_t i = 0; i < TEAR_MAX; i++)
  {
    tears[i].x = -1;
    tears[i].y = -1;
  }
}

void FacePlugin::updateGaze(bool allow)
{
  if (!allow)
  {
    return;
  }
  if (gazeTimer.isReady(gazeInterval))
  {
    targetGx = random(-2, 3);
    targetGy = random(-1, 2);
    gazeInterval = random(2800, 4801);
    gazeMoving = true;
  }
  if (easeTimer.isReady(90))
  {
    if (gx < targetGx)
    {
      gx++;
    }
    else if (gx > targetGx)
    {
      gx--;
    }
    if (gy < targetGy)
    {
      gy++;
    }
    else if (gy > targetGy)
    {
      gy--;
    }
    if (gazeMoving && gx == targetGx && gy == targetGy)
    {
      gazeTimer.reset();
      gazeMoving = false;
    }
  }
}

void FacePlugin::updateBlink(bool allow)
{
  if (!allow)
  {
    blinking = false;
    return;
  }
  if (!blinking && blinkTimer.isReady(blinkInterval))
  {
    blinking = true;
    blinkEndTimer.reset();
    blinkInterval = random(2800, 5501);
  }
  if (blinking && blinkEndTimer.isReady(130))
  {
    blinking = false;
  }
}

void FacePlugin::updateExpression()
{
  if (inExpression)
  {
    if (exprHoldTimer.isReady(holdInterval))
    {
      inExpression = false;
      expression = EXPR_NEUTRAL;
      clearTears();
      idleMouth = MOUTH_LINE;
      lastIdleMouth = MOUTH_LINE;
      mouthTimer.reset();
      mouthInterval = random(14000, 16001);
      exprTimer.reset();
      exprInterval = random(30000, 60001);
    }
    return;
  }
  if (!exprTimer.isReady(exprInterval))
  {
    return;
  }
  if (gx != 0 || gy != 0)
  {
    targetGx = 0;
    targetGy = 0;
    exprTimer.forceReady();
    return;
  }
  Expression picked;
  do
  {
    picked = static_cast<Expression>(random(1, EXPR_COUNT));
  } while (picked == lastExpression);
  lastExpression = picked;
  expression = picked;
  inExpression = true;
  holdInterval = random(1600, 3001);
  exprHoldTimer.reset();
}

void FacePlugin::updateIdleMouth(bool allow)
{
  if (!allow)
  {
    return;
  }
  if (mouthTimer.isReady(mouthInterval))
  {
    IdleMouth picked;
    do
    {
      picked = static_cast<IdleMouth>(random(0, IDLE_MOUTH_COUNT));
    } while (picked == lastIdleMouth);
    lastIdleMouth = picked;
    idleMouth = picked;
    mouthInterval = random(14000, 16001);
  }
}

void FacePlugin::updateTears(bool crying)
{
  if (crying && tearSpawnTimer.isReady(350))
  {
    for (uint8_t i = 0; i < TEAR_MAX; i++)
    {
      if (tears[i].x < 0)
      {
        tears[i].x = tearLeftNext ? 2 : 13;
        tears[i].y = 7;
        tearLeftNext = !tearLeftNext;
        break;
      }
    }
  }
  if (tearFallTimer.isReady(120))
  {
    for (uint8_t i = 0; i < TEAR_MAX; i++)
    {
      if (tears[i].x >= 0)
      {
        tears[i].y++;
        if (tears[i].y > 15)
        {
          tears[i].x = -1;
          tears[i].y = -1;
        }
      }
    }
  }
}

void FacePlugin::drawEyes()
{
  if (!inExpression)
  {
    if (blinking)
    {
      for (int dx = 0; dx < 3; dx++)
      {
        setPx(3 + gx + dx, 6 + gy, BRIGHT_FULL);
        setPx(10 + gx + dx, 6 + gy, BRIGHT_FULL);
      }
    }
    else
    {
      drawBlock(3 + gx, 5 + gy);
      drawBlock(10 + gx, 5 + gy);
    }
    return;
  }
  switch (expression)
  {
  case EXPR_LAUGH:
  {
    int lx = 3 + gx;
    int rx = 10 + gx;
    int oy = 5 + gy;
    setPx(lx, oy + 1, BRIGHT_FULL);
    setPx(lx + 1, oy, BRIGHT_FULL);
    setPx(lx + 2, oy + 1, BRIGHT_FULL);
    setPx(rx, oy + 1, BRIGHT_FULL);
    setPx(rx + 1, oy, BRIGHT_FULL);
    setPx(rx + 2, oy + 1, BRIGHT_FULL);
    break;
  }
  case EXPR_CRYING:
    drawBlock(3 + gx, 5 + gy);
    drawBlock(10 + gx, 5 + gy);
    break;
  case EXPR_SHOCKED:
    drawOutline4x4(2, 4);
    drawOutline4x4(10, 4);
    break;
  case EXPR_EXCITED:
    drawPlus(4 + gx, 6 + gy);
    drawPlus(11 + gx, 6 + gy);
    break;
  default:
    break;
  }
}

void FacePlugin::drawMouth()
{
  if (!inExpression)
  {
    switch (idleMouth)
    {
    case MOUTH_SMILE:
      for (int x = 5; x <= 10; x++)
      {
        setPx(x, 12, BRIGHT_FULL);
      }
      setPx(4, 11, BRIGHT_FULL);
      setPx(11, 11, BRIGHT_FULL);
      break;
    case MOUTH_OPEN_SMALL:
      for (int x = 6; x <= 9; x++)
      {
        setPx(x, 12, BRIGHT_FULL);
      }
      setPx(7, 13, BRIGHT_FULL);
      setPx(8, 13, BRIGHT_FULL);
      break;
    case MOUTH_LINE:
    default:
      for (int x = 5; x <= 10; x++)
      {
        setPx(x, 12, BRIGHT_FULL);
      }
      break;
    }
    return;
  }
  switch (expression)
  {
  case EXPR_LAUGH:
    for (int x = 6; x <= 9; x++)
    {
      setPx(x, 11, BRIGHT_FULL);
    }
    setPx(5, 12, BRIGHT_FULL);
    setPx(10, 12, BRIGHT_FULL);
    for (int x = 5; x <= 10; x++)
    {
      setPx(x, 13, BRIGHT_FULL);
    }
    break;
  case EXPR_CRYING:
    for (int x = 5; x <= 10; x++)
    {
      setPx(x, 12, BRIGHT_FULL);
    }
    setPx(4, 13, BRIGHT_FULL);
    setPx(11, 13, BRIGHT_FULL);
    break;
  case EXPR_SHOCKED:
    drawOutline4x4(6, 11);
    break;
  case EXPR_EXCITED:
    setPx(4, 11, BRIGHT_FULL);
    setPx(11, 11, BRIGHT_FULL);
    for (int x = 5; x <= 10; x++)
    {
      setPx(x, 12, BRIGHT_FULL);
    }
    break;
  default:
    break;
  }
}

void FacePlugin::drawTears()
{
  for (uint8_t i = 0; i < TEAR_MAX; i++)
  {
    if (tears[i].x >= 0)
    {
      setPx(tears[i].x, tears[i].y, BRIGHT_MID);
    }
  }
}

void FacePlugin::render()
{
  Screen.clear();
  drawEyes();
  drawMouth();
  drawTears();
}

void FacePlugin::setup()
{
  Screen.clear();
  gx = 0;
  gy = 0;
  targetGx = random(-2, 3);
  targetGy = random(-1, 2);
  gazeMoving = true;
  blinking = false;
  inExpression = false;
  expression = EXPR_NEUTRAL;
  lastExpression = EXPR_NEUTRAL;
  idleMouth = MOUTH_LINE;
  lastIdleMouth = MOUTH_LINE;
  clearTears();
  gazeInterval = random(2800, 4801);
  blinkInterval = random(2800, 5501);
  exprInterval = random(30000, 60001);
  holdInterval = random(1600, 3001);
  mouthInterval = random(14000, 16001);
  gazeTimer.reset();
  easeTimer.reset();
  blinkTimer.reset();
  exprTimer.reset();
  mouthTimer.reset();
}

void FacePlugin::loop()
{
  updateExpression();
  bool neutral = !inExpression;
  updateGaze(neutral);
  updateBlink(neutral);
  updateIdleMouth(neutral);
  updateTears(inExpression && expression == EXPR_CRYING);
  render();
}

const char *FacePlugin::getName() const
{
  return "Face";
}

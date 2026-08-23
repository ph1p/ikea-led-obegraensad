#pragma once

#include "PluginManager.h"
#include "timing.h"

class TetrisPlugin : public Plugin
{
private:
  static constexpr uint8_t PIECE_COUNT = 7;
  static constexpr uint8_t FIELD_W = 16;
  static constexpr uint8_t FIELD_H = 16;
  static constexpr uint8_t SPAWN_X = (FIELD_W - 4) / 2;
  static constexpr uint8_t SPAWN_Y = 0;
  static constexpr uint16_t GRAVITY_BASE_MS = 300;
  static constexpr uint16_t GRAVITY_STEP_MS = 12;
  static constexpr uint16_t GRAVITY_MIN_MS = 120;
  static constexpr uint8_t BLINK_INTERVAL_MS = 120;
  static constexpr uint8_t FLASH_INTERVAL_MS = 150;
  static constexpr uint8_t SEQ_TOGGLES = 6;

  // shapes

  static constexpr uint16_t PIECES[PIECE_COUNT][4] = {
      {0x00F0, 0x2222, 0x0F00, 0x1111},
      {0x0033, 0x0033, 0x0033, 0x0033},
      {0x0072, 0x0262, 0x0270, 0x0232},
      {0x0036, 0x0462, 0x0360, 0x0231},
      {0x0063, 0x0264, 0x0630, 0x0132},
      {0x0071, 0x0226, 0x0470, 0x0322},
      {0x0074, 0x0622, 0x0170, 0x0223}};
  static constexpr uint8_t BRIGHTNESS[PIECE_COUNT] = {255, 200, 151, 100, 75, 40, 10};

  enum GameState : uint8_t
  {
    RUNNING,
    CLEARING,
    GAMEOVER
  };

  NonBlockingDelay gravityTimer;
  NonBlockingDelay seqTimer;

  GameState gameState = RUNNING;
  uint8_t field[16][16];
  uint8_t pieceType = 0;
  uint8_t pieceRot = 0;
  int8_t pieceX = SPAWN_X;
  int8_t pieceY = SPAWN_Y;
  uint8_t targetRot = 0;
  int8_t targetX = SPAWN_X;
  uint16_t dropInterval = GRAVITY_BASE_MS;
  uint16_t linesTotal = 0;
  uint8_t clearRows[4];
  uint8_t clearRowCount = 0;
  uint8_t seqToggles = 0;
  bool blinkVisible = true;
  bool fieldVisible = true;

  // pieces

  static void cellOffset(uint8_t index, int8_t &dx, int8_t &dy);
  static bool collides(uint8_t type, uint8_t rot, int8_t px, int8_t py,
                       const uint8_t f[16][16]);

  // ai

  static void copyField(const uint8_t src[16][16], uint8_t dst[16][16]);
  static uint8_t columnHeight(const uint8_t f[16][16], uint8_t col);
  static uint16_t countHoles(const uint8_t f[16][16]);
  static uint16_t countFullRows(const uint8_t f[16][16]);
  void simulateDrop(uint8_t type, uint8_t rot, uint8_t left, uint8_t out[16][16]) const;
  static int32_t scorePlacement(const uint8_t f[16][16], uint16_t holesBefore);
  void planAi();

  // game logic

  void tickRunning();
  void dropStep();
  void lockPiece();
  void startClearing();
  bool isClearingRow(uint8_t y) const;
  void collapse();
  void updateSpeed();
  void spawnPiece();
  void startGameOver();
  void resetGame();

  // rendering

  void render();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};

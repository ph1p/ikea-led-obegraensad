#pragma once

#include "PluginManager.h"
#include "timing.h"

class TronPlugin : public Plugin
{
private:
  static constexpr uint8_t GRID_SIZE = 16;
  static constexpr uint8_t RIDER_COUNT = 2;
  static constexpr uint8_t MOVE_OPTION_COUNT = 3;
  static constexpr uint16_t TRAIL_MAX = TronPlugin::GRID_SIZE * TronPlugin::GRID_SIZE;
  static constexpr uint8_t STEP_INTERVAL_MS = 220;
  static constexpr uint8_t FLASH_INTERVAL_MS = 120;
  static constexpr uint8_t FLASH_TOGGLES = 6;
  static constexpr uint8_t REWIND_INTERVAL_MS = 25;
  static constexpr uint8_t BRIGHTNESS_RIDER_1 = 255;
  static constexpr uint8_t BRIGHTNESS_RIDER_2 = 60;

  enum GameState : uint8_t
  {
    RUNNING,
    FLASH,
    REWIND
  };

  enum RiderId : uint8_t
  {
    RIDER_ONE,
    RIDER_TWO
  };

  struct RiderState
  {
    int8_t x;
    int8_t y;
    int8_t dx;
    int8_t dy;
  };

  struct MoveOption
  {
    int8_t dx;
    int8_t dy;
    int8_t tx;
    int8_t ty;
  };

  NonBlockingDelay stepTimer;
  NonBlockingDelay seqTimer;

  GameState gameState = RUNNING;
  RiderState riders[TronPlugin::RIDER_COUNT];
  uint8_t trail[TronPlugin::GRID_SIZE][TronPlugin::GRID_SIZE];
  int8_t histX[TronPlugin::RIDER_COUNT][TronPlugin::TRAIL_MAX];
  int8_t histY[TronPlugin::RIDER_COUNT][TronPlugin::TRAIL_MAX];
  uint16_t histLen[TronPlugin::RIDER_COUNT];
  bool loser[TronPlugin::RIDER_COUNT];
  bool blinkVisible = true;
  uint8_t blinkToggles = 0;

  void resetBoard();
  void placeRider(uint8_t idx, int8_t x, int8_t y, int8_t dx, int8_t dy);
  void tickRiders();
  void chooseMove(uint8_t idx, bool &hasTarget, int8_t &targetX, int8_t &targetY,
                  int8_t otherPendingX, int8_t otherPendingY);
  void commitMove(uint8_t idx, int8_t targetX, int8_t targetY);
  void endRound(bool loserOne, bool loserTwo);
  void rewindTick();
  void render();
  bool cellFree(int8_t x, int8_t y) const;
  bool cellFreeForSearch(int8_t x, int8_t y, int8_t excludeAX, int8_t excludeAY,
                         int8_t excludeBX, int8_t excludeBY) const;
  uint16_t floodFillCount(int8_t startX, int8_t startY, int8_t excludeAX, int8_t excludeAY,
                          int8_t excludeBX, int8_t excludeBY) const;
  static uint8_t riderBrightness(uint8_t idx);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};

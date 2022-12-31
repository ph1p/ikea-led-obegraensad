// must be in same order as in backend/src/mode/mode.cpp listenOnButtonToChangeMode()
export enum MODE {
  NONE = "",
  STARS = "stars",
  LINES = "lines",
  BREAKOUT = "breakout",
  GAMEOFLIFE = "gameoflife",
  CIRCLE = "circle",
  CLOCK = "clock",
  ROBOTS = "robots",
  CUSTOM = "custom",
  UPDATE = "update",
  LOADING = "loading",
}

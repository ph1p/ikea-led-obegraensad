import { Setter } from "solid-js";

export enum MODE {
  NONE = 'draw',
  STARS = 'stars',
  LINES = 'lines',
  BREAKOUT = 'breakout',
  SNAKE = 'snake',
  GAMEOFLIFE = 'gameoflife',
  CIRCLE = 'circle',
  CLOCK = 'clock',
  BIGCLOCK = 'bigclock',
  CUSTOM = 'custom',
  UPDATE = 'update',
  LOADING = 'loading',
}

export interface Store {
  rotation: () => number;
  brightness: () => number;
  indexMatrix: () => number[];
  leds: () => number[];
  mode: () => MODE;

  setRotation: Setter<number>;
  setBrightness: Setter<number>;
  setIndexMatrix: Setter<number[]>;
  setLeds: Setter<number[]>;
  setMode: Setter<MODE>;
  send: (message: string) => void;
  connectionState: () => number;

  connectionStatus?: string;
  toast: (text: string, duration: number) => void;
}
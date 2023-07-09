import { Setter } from 'solid-js';

export enum SYSTEM_STATUS {
  NONE = 'draw',
  WSBINARY = 'wsbinary',
  // SYSTEM
  UPDATE = 'update',
  LOADING = 'loading',
}

export interface Store {
  rotation: () => number;
  brightness: () => number;
  indexMatrix: () => number[];
  leds: () => number[];
  plugins: () => { id: number; name: string }[];
  plugin: () => number;
  systemStatus: () => SYSTEM_STATUS;

  setRotation: Setter<number>;
  setBrightness: Setter<number>;
  setIndexMatrix: Setter<number[]>;
  setLeds: Setter<number[]>;
  setPlugin: Setter<number>;
  setSystemStatus: Setter<SYSTEM_STATUS>;
  send: (message: string | ArrayBuffer) => void;
  connectionState: () => number;

  connectionStatus?: string;
  toast: (text: string, duration: number) => void;
}

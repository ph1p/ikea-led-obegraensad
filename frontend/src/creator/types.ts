import type { Signal } from "solid-js";

// Type aliases for frame signals
export type FrameSignal = Signal<number[]>;
export type FrameSignals = FrameSignal[];

// Animation data structure
export interface AnimationData {
  version: number;
  frames: number[][];
  animationDelayMs?: number;
}

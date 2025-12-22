import type { Signal } from "solid-js";

export type FrameSignal = Signal<number[]>;
export type FrameSignals = FrameSignal[];

export interface AnimationData {
  version: number;
  frames: number[][];
  animationDelayMs?: number;
}

import { createEffect, createSignal, onCleanup, untrack } from "solid-js";

import type { FrameSignals } from "../types";

export function useAnimation(screenSignals: () => FrameSignals, animationDelayMs: () => number) {
  const [isPlaying, setIsPlaying] = createSignal(false);
  const [currentFrame, setCurrentFrame] = createSignal<number[]>([]);
  const [intervalId, setIntervalId] = createSignal(0);

  const togglePlay = () => {
    setIsPlaying(!isPlaying());
  };

  createEffect(() => {
    // Always clear any existing interval first (untracked to avoid dependency)
    const existingId = untrack(intervalId);
    if (existingId) {
      clearInterval(existingId);
      setIntervalId(0);
    }

    if (!isPlaying()) {
      return;
    }

    let i = 0;
    const run = () => {
      const signals = screenSignals();
      if (signals.length === 0) return;

      const [screen] = signals[i];
      setCurrentFrame([...screen()]);
      i++;
      if (i >= signals.length) {
        i = 0;
      }
    };
    run();
    const newIntervalId = setInterval(run, animationDelayMs()) as unknown as number;
    setIntervalId(newIntervalId);

    return () => {
      clearInterval(newIntervalId);
      setIntervalId(0);
    };
  });

  // Cleanup on unmount
  onCleanup(() => {
    clearInterval(intervalId());
  });

  return {
    isPlaying,
    currentFrame,
    togglePlay,
  };
}

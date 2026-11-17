import { createEffect, createSignal, untrack } from "solid-js";

import type { FrameSignals } from "../types";

export function useAnimation(screenSignals: () => FrameSignals, animationDelayMs: () => number) {
  const [isPlaying, setIsPlaying] = createSignal(false);
  const [currentFrame, setCurrentFrame] = createSignal<number[]>([]);

  const togglePlay = () => {
    setIsPlaying(!isPlaying());
  };

  createEffect(
    () => [isPlaying(), animationDelayMs()] as const,
    ([playing, delay]) => {
      if (!playing) return;

      let i = 0;
      const run = () => {
        const signals = untrack(screenSignals);
        if (signals.length === 0) return;
        if (i >= signals.length) i = 0;

        const [screen] = signals[i];
        setCurrentFrame([...untrack(screen)]);
        i++;
      };
      run();
      const id = setInterval(run, delay);

      return () => clearInterval(id);
    },
  );

  return {
    isPlaying,
    currentFrame,
    togglePlay,
  };
}

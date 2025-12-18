import { createSignal } from "solid-js";

import { STORAGE_KEY } from "../constants";
import type { AnimationData, FrameSignals } from "../types";

export function useAnimationStorage(
  createNewScreen: (initialData?: number[]) => FrameSignals[number],
) {
  const [hasLoadedFromStorage, setHasLoadedFromStorage] = createSignal(false);

  const saveToLocalStorage = (frames: number[][], animationDelayMs: number) => {
    try {
      // Clear storage if no frames
      if (frames.length === 0) {
        localStorage.removeItem(STORAGE_KEY);
        return;
      }

      const data: AnimationData = {
        version: 1,
        frames,
        animationDelayMs,
      };
      localStorage.setItem(STORAGE_KEY, JSON.stringify(data));
    } catch (error) {
      console.error("Failed to save to local storage:", error);
    }
  };

  const loadFromLocalStorage = (): {
    frames: FrameSignals;
    animationDelayMs?: number;
  } | null => {
    try {
      const stored = localStorage.getItem(STORAGE_KEY);
      if (!stored) {
        setHasLoadedFromStorage(true);
        return null;
      }

      const data: AnimationData = JSON.parse(stored);
      if (!data.frames || !Array.isArray(data.frames) || data.frames.length === 0) {
        setHasLoadedFromStorage(true);
        return null;
      }

      const newSignals = data.frames.map((frame: number[]) =>
        createNewScreen(frame),
      ) as FrameSignals;

      setHasLoadedFromStorage(true);

      return {
        frames: newSignals,
        animationDelayMs: data.animationDelayMs,
      };
    } catch (error) {
      console.error("Failed to load from local storage:", error);
      setHasLoadedFromStorage(true);
      return null;
    }
  };

  return {
    saveToLocalStorage,
    loadFromLocalStorage,
    hasLoadedFromStorage,
  };
}

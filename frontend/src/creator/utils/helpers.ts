import { createSignal } from "solid-js";

import { FOCUS_SCROLL_DELAY_MS, MATRIX_SIZE, TIMELINE_SCROLL_DELAY_MS } from "../constants";
import type { FrameSignals } from "../types";

export function createNewScreen(initialData?: number[]): FrameSignals[number] {
  const [screen, setScreen] = createSignal(initialData || new Array(MATRIX_SIZE).fill(0));
  return [screen, setScreen] as const;
}

export function scrollTimelineToFrame(
  ref: HTMLDivElement | undefined,
  frameIndex: number,
  totalFrames: number,
) {
  if (ref && frameIndex >= 0 && frameIndex < totalFrames) {
    setTimeout(() => {
      const timelineButtons = ref.querySelectorAll("button");
      if (timelineButtons[frameIndex]) {
        timelineButtons[frameIndex].scrollIntoView({
          behavior: "smooth",
          inline: "center",
          block: "nearest",
        });
      }
    }, TIMELINE_SCROLL_DELAY_MS);
  }
}

export function focusFrame(
  setFocusedFrameIndex: (index: number) => void,
  scrollToFrame: (index: number) => void,
  index: number,
  delay = FOCUS_SCROLL_DELAY_MS,
) {
  setTimeout(() => {
    setFocusedFrameIndex(index);
    scrollToFrame(index);
  }, delay);
}

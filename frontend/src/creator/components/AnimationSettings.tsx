import type { Component } from "solid-js";

import {
  FRAME_DELAY_STEP_MS,
  MAX_FRAME_DELAY_INPUT_MS,
  MAX_FRAME_DELAY_MS,
  MIN_FRAME_DELAY_MS,
} from "../constants";

interface AnimationSettingsProps {
  frameDurationId: string;
  animationDelayMs: number;
  onDelayChange: (value: number) => void;
  totalFrames: number;
}

export const AnimationSettings: Component<AnimationSettingsProps> = (props) => {
  const fps = () => 1000 / props.animationDelayMs;
  const duration = () => (props.totalFrames * props.animationDelayMs) / 1000;

  return (
    <div class="space-y-3">
      <div class="space-y-2">
        <label
          for={props.frameDurationId}
          class="text-sm font-semibold text-gray-700 uppercase tracking-wide"
        >
          Frame Duration
        </label>
        <div class="flex items-center gap-2">
          <input
            id={props.frameDurationId}
            type="range"
            min={MIN_FRAME_DELAY_MS}
            max={MAX_FRAME_DELAY_MS}
            step={FRAME_DELAY_STEP_MS}
            value={props.animationDelayMs}
            onInput={(e) => props.onDelayChange(Number.parseInt(e.currentTarget.value, 10))}
            class="flex-1 h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer accent-blue-600"
          />
          <input
            type="number"
            min={MIN_FRAME_DELAY_MS}
            max={MAX_FRAME_DELAY_INPUT_MS}
            step={FRAME_DELAY_STEP_MS}
            value={props.animationDelayMs}
            onInput={(e) => {
              const value = Number.parseInt(e.currentTarget.value, 10);
              if (
                !Number.isNaN(value) &&
                value >= MIN_FRAME_DELAY_MS &&
                value <= MAX_FRAME_DELAY_INPUT_MS
              ) {
                props.onDelayChange(value);
              }
            }}
            class="w-16 px-2 py-1 text-xs border border-gray-300 rounded focus:outline-none focus:ring-2 focus:ring-blue-500"
          />
          <span class="text-xs text-gray-500 whitespace-nowrap">ms</span>
        </div>
      </div>

      <div class="text-xs text-gray-500 space-y-1">
        <p>
          <strong>Frames:</strong> {props.totalFrames}
        </p>
        <p>
          <strong>Speed:</strong> {fps().toFixed(1)} FPS ({props.animationDelayMs}ms/frame)
        </p>
        <p>
          <strong>Duration:</strong> {duration().toFixed(1)}s
        </p>
      </div>
    </div>
  );
};

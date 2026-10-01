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
        <label for={props.frameDurationId} class="section-title block">
          Frame Duration
        </label>
        <div class="flex items-center gap-3">
          <input
            id={props.frameDurationId}
            type="range"
            min={MIN_FRAME_DELAY_MS}
            max={MAX_FRAME_DELAY_MS}
            step={FRAME_DELAY_STEP_MS}
            value={props.animationDelayMs}
            onInput={(e) => props.onDelayChange(Number.parseInt(e.currentTarget.value, 10))}
            class="flex-1"
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
            aria-label="Frame duration in milliseconds"
            class="input w-18 px-2 py-1.5 text-xs tabular-nums"
          />
          <span class="text-xs text-muted">ms</span>
        </div>
      </div>

      <dl class="grid grid-cols-3 gap-2 text-center">
        <div class="rounded-xl border border-line bg-canvas/40 p-2">
          <dt class="text-[11px] text-muted">Frames</dt>
          <dd class="text-sm font-semibold tabular-nums">{props.totalFrames}</dd>
        </div>
        <div class="rounded-xl border border-line bg-canvas/40 p-2">
          <dt class="text-[11px] text-muted">FPS</dt>
          <dd class="text-sm font-semibold tabular-nums">{fps().toFixed(1)}</dd>
        </div>
        <div class="rounded-xl border border-line bg-canvas/40 p-2">
          <dt class="text-[11px] text-muted">Length</dt>
          <dd class="text-sm font-semibold tabular-nums">{duration().toFixed(1)}s</dd>
        </div>
      </dl>
    </div>
  );
};

import type { Component } from "solid-js";
import { For } from "solid-js";

import type { FrameSignals } from "../types";

interface FrameTimelineProps {
  screenSignals: FrameSignals;
  focusedFrameIndex: number;
  onFrameClick: (index: number) => void;
  ref?: (el: HTMLDivElement) => void;
}

export const FrameTimeline: Component<FrameTimelineProps> = (props) => {
  return (
    <div class="card p-3">
      <div ref={props.ref} class="flex gap-2 overflow-x-auto" style="scrollbar-width: thin;">
        <For each={props.screenSignals}>
          {([screen], index) => (
            <button
              type="button"
              onClick={() => props.onFrameClick(index())}
              aria-current={props.focusedFrameIndex === index() ? "true" : undefined}
              class={`flex shrink-0 cursor-pointer flex-col items-center rounded-lg border p-1.5 transition ${
                props.focusedFrameIndex === index()
                  ? "border-accent/70 bg-raised"
                  : "border-line opacity-60 hover:opacity-100"
              }`}
              title={`Frame ${index() + 1}`}
            >
              <div class="grid h-12 w-8 grid-cols-16 gap-0 overflow-hidden rounded-sm bg-black">
                <For each={screen()}>
                  {(pixel) => <div class={`w-0.5 h-0.5 ${pixel ? "bg-white" : "bg-white/5"}`} />}
                </For>
              </div>
              <div class="mt-1 w-full text-center text-[11px] tabular-nums text-muted">
                {index() + 1}
              </div>
            </button>
          )}
        </For>
      </div>
    </div>
  );
};

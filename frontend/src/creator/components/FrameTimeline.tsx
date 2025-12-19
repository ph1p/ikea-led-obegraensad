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
    <div class="bg-white p-4 rounded-2xl shadow-lg">
      <div ref={props.ref} class="flex gap-2 overflow-x-auto pb-2" style="scrollbar-width: thin;">
        <For each={props.screenSignals}>
          {([screen], index) => (
            <button
              type="button"
              onClick={() => props.onFrameClick(index())}
              class={`shrink-0 p-2 rounded border-2 transition-all hover:border-gray-700 border-gray-400 ${
                props.focusedFrameIndex === index() ? "" : "opacity-50"
              }`}
              title={`Frame ${index() + 1}`}
            >
              <div class="w-16 h-16 grid grid-cols-16 gap-0">
                <For each={screen()}>
                  {(pixel) => <div class={`w-1 h-1 ${pixel ? "bg-white" : "bg-gray-900"}`} />}
                </For>
              </div>
              <div class="text-xs text-center mt-1 text-gray-400">{index() + 1}</div>
            </button>
          )}
        </For>
      </div>
    </div>
  );
};

import type { Component } from "solid-js";

import { Button } from "../../components/button";

interface FrameControlsProps {
  focusedFrameIndex: number;
  totalFrames: number;
  onMoveLeft: () => void;
  onMoveRight: () => void;
  onDuplicate: () => void;
  onEmpty: () => void;
  onRemove: () => void;
}

export const FrameControls: Component<FrameControlsProps> = (props) => {
  return (
    <header class="flex w-[min(100%,700px,(100dvh-16rem)*9/13)] items-center justify-between gap-3 max-lg:w-[min(100%,55dvh*9/13)]">
      <div class="flex items-center gap-1 rounded-xl border border-line bg-surface/80 p-1">
        <Button
          widthAuto
          title="Move frame left"
          aria-label="Move frame left"
          disabled={props.focusedFrameIndex === 0}
          onClick={props.onMoveLeft}
          class="btn-ghost btn-icon size-9"
        >
          <i class="fa-solid fa-arrow-left" />
        </Button>
        <Button
          widthAuto
          title="Move frame right"
          aria-label="Move frame right"
          disabled={props.focusedFrameIndex === props.totalFrames - 1}
          onClick={props.onMoveRight}
          class="btn-ghost btn-icon size-9"
        >
          <i class="fa-solid fa-arrow-right" />
        </Button>
        <span class="mx-1 h-5 w-px bg-line" />
        <Button
          widthAuto
          title="Duplicate frame"
          aria-label="Duplicate frame"
          onClick={props.onDuplicate}
          class="btn-ghost btn-icon size-9"
        >
          <i class="fa-solid fa-copy" />
        </Button>
        <Button
          widthAuto
          title="Empty frame"
          aria-label="Empty frame"
          onClick={props.onEmpty}
          class="btn-ghost btn-icon size-9"
        >
          <i class="fa-solid fa-eraser" />
        </Button>
        <Button
          widthAuto
          title="Remove frame"
          aria-label="Remove frame"
          onClick={props.onRemove}
          class="btn-ghost btn-danger btn-icon size-9"
        >
          <i class="fa-solid fa-trash" />
        </Button>
      </div>
      <div class="text-lg tabular-nums">
        <span class="font-semibold">{props.focusedFrameIndex + 1}</span>
        <span class="text-muted"> / {props.totalFrames}</span>
      </div>
    </header>
  );
};

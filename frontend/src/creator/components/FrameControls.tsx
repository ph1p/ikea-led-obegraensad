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
    <header class="flex justify-between items-center mb-4">
      <div class="flex gap-2">
        <Button
          widthAuto
          disabled={props.focusedFrameIndex === 0}
          onClick={props.onMoveLeft}
          class="hover:bg-gray-700 transition-colors w-auto"
        >
          <i class="fa-solid fa-arrow-left mr-1" />
        </Button>

        <Button
          widthAuto
          disabled={props.focusedFrameIndex === props.totalFrames - 1}
          onClick={props.onMoveRight}
          class="hover:bg-gray-700 transition-colors w-auto"
        >
          <i class="fa-solid fa-arrow-right mr-1" />
        </Button>

        <Button
          widthAuto
          onClick={props.onDuplicate}
          class="hover:bg-gray-700 transition-colors w-auto"
        >
          <i class="fa-solid fa-copy mr-1" />
          <span class="text-xs">Duplicate</span>
        </Button>

        <Button
          widthAuto
          onClick={props.onEmpty}
          class="hover:bg-gray-700 transition-colors w-auto"
        >
          <i class="fa-solid fa-eraser mr-1" />
          <span class="text-xs">Empty</span>
        </Button>

        <Button
          widthAuto
          onClick={props.onRemove}
          class="hover:bg-gray-700 transition-colors w-auto"
        >
          <i class="fa-solid fa-trash mr-1" />
          <span class="text-xs">Remove</span>
        </Button>
      </div>
      <div class="text-center text-2xl text-white flex items-center ml-2">
        <span class="font-bold">{props.focusedFrameIndex + 1}</span>
        <span class="opacity-50">/{props.totalFrames}</span>
      </div>
    </header>
  );
};

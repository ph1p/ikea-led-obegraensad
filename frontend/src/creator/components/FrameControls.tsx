import type { Component } from "solid-js";

import { Button } from "../../components/button";
import { Tooltip } from "../../components/tooltip";

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
        <Tooltip text="Move frame left">
          <Button
            widthAuto
            disabled={props.focusedFrameIndex === 0}
            onClick={props.onMoveLeft}
            class="hover:bg-gray-700 transition-colors w-auto"
          >
            <i class="fa-solid fa-arrow-left" />
          </Button>
        </Tooltip>

        <Tooltip text="Move frame right">
          <Button
            widthAuto
            disabled={props.focusedFrameIndex === props.totalFrames - 1}
            onClick={props.onMoveRight}
            class="hover:bg-gray-700 transition-colors w-auto"
          >
            <i class="fa-solid fa-arrow-right" />
          </Button>
        </Tooltip>

        <Tooltip text="Duplicate frame">
          <Button
            widthAuto
            onClick={props.onDuplicate}
            class="hover:bg-gray-700 transition-colors w-auto"
          >
            <i class="fa-solid fa-copy" />
          </Button>
        </Tooltip>

        <Tooltip text="Empty current frame">
          <Button
            widthAuto
            onClick={props.onEmpty}
            class="hover:bg-gray-700 transition-colors w-auto"
          >
            <i class="fa-solid fa-eraser" />
          </Button>
        </Tooltip>

        <Tooltip text="Remove current frame">
          <Button
            widthAuto
            onClick={props.onRemove}
            class="hover:bg-gray-700 transition-colors w-auto"
          >
            <i class="fa-solid fa-trash" />
          </Button>
        </Tooltip>
      </div>
      <div class="text-center text-2xl text-white flex items-center">
        <span class="font-bold">{props.focusedFrameIndex + 1}</span>
        <span class="opacity-50">/{props.totalFrames}</span>
      </div>
    </header>
  );
};

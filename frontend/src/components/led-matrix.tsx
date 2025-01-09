import { Component, createEffect, onCleanup, onMount } from "solid-js";
import { createVisibilityObserver } from "@solid-primitives/intersection-observer";
import { Accessor } from "solid-js";

interface Props {
  disabled?: boolean;
  onSetLed?: (data: { index: number; status: number }) => void;
  onSetMatrix?: (data: number[]) => void;
  data: Accessor<number[]>;
  indexData: Accessor<number[]>;
}

export const LedMatrix: Component<Props> = (props) => {
  let canvasRef: HTMLCanvasElement | undefined;
  let containerRef: HTMLDivElement | undefined;
  let isDrawing = false;
  let currentDrawValue = 255;

  const MATRIX_SIZE = 16;
  const LED_COLORS = {
    OFF: "#333333",
    BACKGROUND: "#111111",
  };

  const useVisibilityObserver = createVisibilityObserver({ threshold: 0.9 });
  const visible = useVisibilityObserver(() => containerRef);

  const getLedColor = (brightness: number) => {
    if (brightness <= 0) return LED_COLORS.OFF;

    const intensity = Math.min(255, Math.max(0, brightness));
    return `rgb(${intensity}, ${intensity}, ${intensity})`;
  };

  const drawMatrix = (data: number[], indexData: number[]) => {
    if (!canvasRef) return;

    const ctx = canvasRef.getContext("2d")!;
    const canvas = canvasRef;
    const cellSize = canvas.width / MATRIX_SIZE;
    const padding = 4;

    ctx.fillStyle = LED_COLORS.BACKGROUND;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    for (let y = 0; y < MATRIX_SIZE; y++) {
      for (let x = 0; x < MATRIX_SIZE; x++) {
        const index = y * MATRIX_SIZE + x;
        const mappedIndex = indexData[index];
        const brightness = data[mappedIndex];

        ctx.fillStyle = getLedColor(brightness);
        ctx.fillRect(
          x * cellSize + padding,
          y * cellSize + padding,
          cellSize - padding * 2,
          cellSize - padding * 2,
        );
      }
    }
  };

  const handlePointerEvent = (e: PointerEvent) => {
    if (!canvasRef || props.disabled) return null;

    const rect = canvasRef.getBoundingClientRect();
    const scaleX = canvasRef.width / rect.width;
    const scaleY = canvasRef.height / rect.height;

    const x = Math.floor(
      ((e.clientX - rect.left) * scaleX) / (canvasRef.width / MATRIX_SIZE),
    );
    const y = Math.floor(
      ((e.clientY - rect.top) * scaleY) / (canvasRef.height / MATRIX_SIZE),
    );

    if (x >= 0 && x < MATRIX_SIZE && y >= 0 && y < MATRIX_SIZE) {
      const index = y * MATRIX_SIZE + x;
      const mappedIndex = props.indexData()[index];
      return { index, mappedIndex };
    }
    return null;
  };

  const handlePointerDown = (e: PointerEvent) => {
    if (props.disabled) return;
    e.preventDefault();

    const position = handlePointerEvent(e);
    if (!position) return;

    isDrawing = true;
    currentDrawValue = props.data()[position.mappedIndex] > 0 ? 0 : 255;
    props.onSetLed?.({ index: position.mappedIndex, status: currentDrawValue });

    const newState = props
      .data()
      .map((led, i) =>
        i === position.mappedIndex ? Number(currentDrawValue) : led,
      );

    props.onSetMatrix?.(newState);
  };

  const handlePointerMove = (e: PointerEvent) => {
    if (!isDrawing || props.disabled) return;
    e.preventDefault();

    const position = handlePointerEvent(e);
    if (!position) return;

    if (props.data()[position.mappedIndex] !== currentDrawValue) {
      props.onSetLed?.({
        index: position.mappedIndex,
        status: currentDrawValue,
      });

      const newState = props
        .data()
        .map((led, i) =>
          i === position.mappedIndex ? Number(currentDrawValue) : led,
        );

      props.onSetMatrix?.(newState);
    }
  };

  const handlePointerUp = () => {
    if (isDrawing && props.onSetMatrix) {
      props.onSetMatrix(props.data());
    }
    isDrawing = false;
  };

  onMount(() => {
    if (!canvasRef) return;

    canvasRef.width = 400;
    canvasRef.height = 400;

    drawMatrix(props.data(), props.indexData());

    canvasRef.addEventListener("pointerdown", handlePointerDown);
    canvasRef.addEventListener("pointermove", handlePointerMove);
    canvasRef.addEventListener("pointerup", handlePointerUp);
    canvasRef.addEventListener("pointerleave", handlePointerUp);
  });

  onCleanup(() => {
    if (!canvasRef) return;

    canvasRef.removeEventListener("pointerdown", handlePointerDown);
    canvasRef.removeEventListener("pointermove", handlePointerMove);
    canvasRef.removeEventListener("pointerup", handlePointerUp);
    canvasRef.removeEventListener("pointerleave", handlePointerUp);
  });

  createEffect(() => {
    const data = props.data();
    const indexData = props.indexData();

    if (canvasRef && data.length && indexData.length) {
      drawMatrix(data, indexData);
    }
  });

  return (
    <div
      ref={containerRef}
      class={`
        p-4 bg-[#111111]
        shadow-lg
        relative mx-auto flex-none inline-block
        transition-opacity duration-300
        ${visible() ? "opacity-100" : "opacity-50"}
        ${props.disabled ? "opacity-30" : ""}
        w-full h-[calc(100vh*0.8)] aspect-[3/4]
      `}
    >
      <canvas
        ref={canvasRef}
        class="w-full h-full self-center"
        style={{
          "image-rendering": "pixelated",
        }}
      />
    </div>
  );
};

export default LedMatrix;

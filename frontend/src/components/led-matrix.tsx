import { createVisibilityObserver } from "@solid-primitives/intersection-observer";
import { type Component, createEffect, createSignal, onCleanup, onMount } from "solid-js";

interface Props {
  disabled?: boolean;
  onSetLed?: (data: { index: number; status: number }) => void;
  onSetMatrix?: (data: number[]) => void;
  data: number[];
  indexData: number[];
  brightness: number;
}

export const LedMatrix: Component<Props> = (props) => {
  let canvasRef: HTMLCanvasElement | undefined;
  let containerRef: HTMLDivElement | undefined;
  const [isDrawing, setIsDrawing] = createSignal(false);
  let currentDrawValue = 255;

  const MATRIX_SIZE = 16;
  const LED_COLORS = {
    OFF: "#000000",
    BACKGROUND: "#111111",
  };

  const useVisibilityObserver = createVisibilityObserver({ threshold: 0.9 });
  const visible = useVisibilityObserver(() => containerRef);

  const getLedColor = (ledBrightness: number) => {
    const brightnessFactor = props.brightness / 255;
    const intensity = Math.round(Math.min(255, Math.max(0, ledBrightness * brightnessFactor)));
    const displayIntensity = Math.round(51 + (intensity * 204) / 255);
    return `rgb(${displayIntensity}, ${displayIntensity}, ${displayIntensity})`;
  };

  const drawMatrix = (data: number[], indexData: number[]) => {
    if (!canvasRef) return;

    const ctx = canvasRef.getContext("2d");
    const logicalSize = 400;
    const cellSize = logicalSize / MATRIX_SIZE;
    const padding = 4;

    if (!ctx) return;

    ctx.fillStyle = LED_COLORS.BACKGROUND;
    ctx.fillRect(0, 0, logicalSize, logicalSize);

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
    const logicalSize = 400;
    const scaleX = logicalSize / rect.width;
    const scaleY = logicalSize / rect.height;

    const x = Math.floor(((e.clientX - rect.left) * scaleX) / (logicalSize / MATRIX_SIZE));
    const y = Math.floor(((e.clientY - rect.top) * scaleY) / (logicalSize / MATRIX_SIZE));

    if (x >= 0 && x < MATRIX_SIZE && y >= 0 && y < MATRIX_SIZE) {
      const index = y * MATRIX_SIZE + x;
      const mappedIndex = props.indexData[index];
      return { index, mappedIndex };
    }
    return null;
  };

  const handlePointerDown = (e: PointerEvent) => {
    if (props.disabled) return;

    const position = handlePointerEvent(e);
    if (!position) return;

    e.preventDefault();

    setIsDrawing(true);
    currentDrawValue = props.data[position.mappedIndex] > 0 ? 0 : 255;
    props.onSetLed?.({ index: position.mappedIndex, status: currentDrawValue });

    const newState = props.data.map((led, i) =>
      i === position.mappedIndex ? Number(currentDrawValue) : led,
    );

    props.onSetMatrix?.(newState);
  };

  const handlePointerMove = (e: PointerEvent) => {
    if (!isDrawing() || props.disabled) return;
    e.preventDefault();

    const position = handlePointerEvent(e);
    if (!position) return;

    if (props.data[position.mappedIndex] !== currentDrawValue) {
      props.onSetLed?.({
        index: position.mappedIndex,
        status: currentDrawValue,
      });

      const newState = props.data.map((led, i) =>
        i === position.mappedIndex ? Number(currentDrawValue) : led,
      );

      props.onSetMatrix?.(newState);
    }
  };

  const handlePointerUp = () => {
    if (isDrawing() && props.onSetMatrix) {
      props.onSetMatrix(props.data);
    }
    setIsDrawing(false);
  };

  onMount(() => {
    if (!canvasRef) return;

    const dpr = window.devicePixelRatio || 1;
    const size = 400;
    canvasRef.width = size * dpr;
    canvasRef.height = size * dpr;

    const ctx = canvasRef.getContext("2d");
    if (ctx) {
      ctx.scale(dpr, dpr);
    }

    drawMatrix(props.data, props.indexData);

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
    const data = props.data;
    const indexData = props.indexData;

    if (canvasRef && data.length && indexData.length) {
      drawMatrix(data, indexData);
    }
  });

  return (
    <div
      class="p-4 bg-[#111111] shadow-lg mx-auto"
      style={{
        width: "min(100%, 700px, (100vh - 16rem) * 9 / 13)",
      }}
    >
      <div
        ref={containerRef}
        class={`
          relative
          transition-all duration-300
          ${visible() ? "opacity-100" : "opacity-50"}
          ${props.disabled ? "opacity-30" : ""}
          ${isDrawing() ? "ring-2 ring-blue-500/50" : ""}
          max-w-full max-h-full
          aspect-9/13
        `}
      >
        <canvas
          ref={canvasRef}
          class="w-full"
          style={{
            "image-rendering": "pixelated",
            "touch-action": "none",
            "aspect-ratio": "9 / 13",
            height: "auto",
          }}
        />
      </div>
    </div>
  );
};

export default LedMatrix;

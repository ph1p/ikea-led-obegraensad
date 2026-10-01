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
    OFF: "#17171b",
    BACKGROUND: "#050506",
  };

  const useVisibilityObserver = createVisibilityObserver({ threshold: 0.9 });
  const visible = useVisibilityObserver(() => containerRef);

  const LOGICAL_SIZE = 400;
  const CELL_SIZE = LOGICAL_SIZE / MATRIX_SIZE;
  const PADDING = 4;
  const LED_SIZE = CELL_SIZE - PADDING * 2;
  const GLOW = 10;

  let ctx: CanvasRenderingContext2D | null = null;
  let dpr = 1;
  // a blurred shadow per LED is the expensive part of a frame, so each
  // intensity is rendered once into a sprite and stamped with drawImage
  const glowSprites = new Map<number, HTMLCanvasElement>();

  const glowSprite = (intensity: number) => {
    let sprite = glowSprites.get(intensity);
    if (sprite) return sprite;

    sprite = document.createElement("canvas");
    const size = LED_SIZE + GLOW * 4;
    sprite.width = sprite.height = Math.ceil(size * dpr);
    const spriteCtx = sprite.getContext("2d");
    if (spriteCtx) {
      spriteCtx.scale(dpr, dpr);
      spriteCtx.shadowBlur = GLOW * dpr;
      spriteCtx.shadowColor = `rgb(255 255 255 / ${(intensity / 255) * 0.55})`;
      spriteCtx.fillStyle = `rgb(${intensity}, ${intensity}, ${intensity})`;
      spriteCtx.fillRect(GLOW * 2, GLOW * 2, LED_SIZE, LED_SIZE);
    }
    glowSprites.set(intensity, sprite);
    return sprite;
  };

  let frame = new Uint8ClampedArray(MATRIX_SIZE * MATRIX_SIZE);
  let drawnFrame: Uint8ClampedArray | null = null;
  let pendingDraw = 0;

  const drawMatrix = () => {
    pendingDraw = 0;
    if (!ctx) return;
    if (drawnFrame && drawnFrame.every((value, i) => value === frame[i])) return;
    drawnFrame = frame;

    ctx.fillStyle = LED_COLORS.BACKGROUND;
    ctx.fillRect(0, 0, LOGICAL_SIZE, LOGICAL_SIZE);

    ctx.fillStyle = LED_COLORS.OFF;
    for (let i = 0; i < frame.length; i++) {
      if (frame[i] === 0) {
        const x = i % MATRIX_SIZE;
        const y = (i - x) / MATRIX_SIZE;
        ctx.fillRect(x * CELL_SIZE + PADDING, y * CELL_SIZE + PADDING, LED_SIZE, LED_SIZE);
      }
    }

    // second pass so the glow of lit LEDs sits on top of their dark neighbours
    const spriteSize = LED_SIZE + GLOW * 4;
    for (let i = 0; i < frame.length; i++) {
      if (frame[i] > 0) {
        const x = i % MATRIX_SIZE;
        const y = (i - x) / MATRIX_SIZE;
        ctx.drawImage(
          glowSprite(frame[i]),
          x * CELL_SIZE + PADDING - GLOW * 2,
          y * CELL_SIZE + PADDING - GLOW * 2,
          spriteSize,
          spriteSize,
        );
      }
    }
  };

  const handlePointerEvent = (e: PointerEvent) => {
    if (!canvasRef || props.disabled) return null;

    const rect = canvasRef.getBoundingClientRect();
    const scaleX = LOGICAL_SIZE / rect.width;
    const scaleY = LOGICAL_SIZE / rect.height;

    const x = Math.floor(((e.clientX - rect.left) * scaleX) / CELL_SIZE);
    const y = Math.floor(((e.clientY - rect.top) * scaleY) / CELL_SIZE);

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

    dpr = window.devicePixelRatio || 1;
    canvasRef.width = LOGICAL_SIZE * dpr;
    canvasRef.height = LOGICAL_SIZE * dpr;

    ctx = canvasRef.getContext("2d");
    ctx?.scale(dpr, dpr);
    drawMatrix();
  });

  onCleanup(() => cancelAnimationFrame(pendingDraw));

  createEffect(() => {
    if (!canvasRef) return;
    if (props.disabled) {
      setIsDrawing(false);
      return;
    }

    canvasRef.addEventListener("pointerdown", handlePointerDown);
    canvasRef.addEventListener("pointermove", handlePointerMove);
    canvasRef.addEventListener("pointerup", handlePointerUp);
    canvasRef.addEventListener("pointerleave", handlePointerUp);

    onCleanup(() => {
      canvasRef?.removeEventListener("pointerdown", handlePointerDown);
      canvasRef?.removeEventListener("pointermove", handlePointerMove);
      canvasRef?.removeEventListener("pointerup", handlePointerUp);
      canvasRef?.removeEventListener("pointerleave", handlePointerUp);
    });
  });

  // the live preview can push frames faster than the display refreshes, so
  // the effect only snapshots the intensities and one draw runs per frame
  createEffect(() => {
    const data = props.data;
    const indexData = props.indexData;
    if (!data.length || !indexData.length) return;

    const scale = props.brightness / 255;
    // clamped array rounds and clamps on write
    const next = new Uint8ClampedArray(MATRIX_SIZE * MATRIX_SIZE);
    for (let i = 0; i < next.length; i++) {
      next[i] = data[indexData[i]] * scale;
    }
    frame = next;

    if (!pendingDraw) {
      pendingDraw = requestAnimationFrame(drawMatrix);
    }
  });

  return (
    <div class="mx-auto w-[min(100%,700px,(100dvh-16rem)*9/13)] rounded-3xl bg-[#050506] p-3 shadow-2xl shadow-black/40 ring-1 ring-line max-lg:w-[min(100%,55dvh*9/13)] sm:p-4">
      <div
        ref={containerRef}
        class={`
          relative
          transition-all duration-300
          ${visible() ? "opacity-100" : "opacity-50"}
          ${isDrawing() ? "ring-2 ring-accent/40 rounded-lg" : ""}
          max-w-full max-h-full
          aspect-9/13
        `}
      >
        <canvas
          ref={canvasRef}
          class="w-full"
          style={{
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

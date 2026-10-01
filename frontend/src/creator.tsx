import type { Component } from "solid-js";
import {
  createEffect,
  createMemo,
  createSignal,
  createUniqueId,
  onCleanup,
  onMount,
  Show,
} from "solid-js";

import { Button } from "./components/button";
import { Layout } from "./components/layout/layout";
import { LedMatrix } from "./components/led-matrix";
import { SidebarSection } from "./components/layout/sidebar";
import { ScreenInfo } from "./components/screen-info";
import { useStore } from "./contexts/store";
import { useToast } from "./contexts/toast";
import { AnimationSettings } from "./creator/components/AnimationSettings";
import { FrameControls } from "./creator/components/FrameControls";
import { FrameTimeline } from "./creator/components/FrameTimeline";
import { KeyboardShortcutsHelp } from "./creator/components/KeyboardShortcutsHelp";
import {
  DEFAULT_FRAME_DELAY_MS,
  HISTORY_DEBOUNCE_MS,
  INITIAL_HISTORY_SAVE_DELAY_MS,
} from "./creator/constants";
import { useAnimation } from "./creator/hooks/useAnimation";
import { useAnimationStorage } from "./creator/hooks/useAnimationStorage";
import { useHistory } from "./creator/hooks/useHistory";
import type { FrameSignals } from "./creator/types";
import { createNewScreen, focusFrame, scrollTimelineToFrame } from "./creator/utils/helpers";
import { createKeyboardHandler } from "./creator/utils/keyboard";
import { matrixToHexArray } from "./helpers";

export const Creator: Component = () => {
  const [store, actions] = useStore();
  const { toast } = useToast();
  const isAnimationPluginActive = createMemo<boolean>(
    () => store?.plugins.find((p) => p.name.includes("Animation"))?.id === store?.plugin,
    true,
  );

  const frameDurationId = createUniqueId();
  const [screenSignals, setScreenSignals] = createSignal<FrameSignals>([]);
  const [animationDelayMs, setAnimationDelayMs] = createSignal(DEFAULT_FRAME_DELAY_MS);
  const [focusedFrameIndex, setFocusedFrameIndex] = createSignal(0);

  const { saveToLocalStorage, loadFromLocalStorage, hasLoadedFromStorage } =
    useAnimationStorage(createNewScreen);

  const { saveToHistory, undo, redo, canUndo, canRedo, isUndoRedoing } = useHistory<number[][]>({
    onUndo: () => toast("Undo", 1000),
    onRedo: () => toast("Redo", 1000),
  });

  const { isPlaying, currentFrame, togglePlay } = useAnimation(screenSignals, animationDelayMs);

  let timelineRef!: HTMLDivElement;

  const currentFrameSignals = () => {
    const index = focusedFrameIndex();
    const signals = screenSignals();
    if (index >= 0 && index < signals.length) {
      return signals[index];
    }
    return null;
  };

  const scrollToFrame = (frameIndex: number) => {
    scrollTimelineToFrame(timelineRef, frameIndex, screenSignals().length);
  };

  const handleAddScreen = () => {
    setScreenSignals((signals): FrameSignals => {
      const lastScreen = signals.length > 0 ? signals[signals.length - 1][0]() : undefined;
      const newSignal = createNewScreen(lastScreen ? [...lastScreen] : undefined);
      return [...signals, newSignal];
    });
    const newIndex = screenSignals().length - 1;
    focusFrame(setFocusedFrameIndex, scrollToFrame, newIndex);
  };

  const handleRemoveScreen = (index: number) => {
    const totalFrames = screenSignals().length;
    if (totalFrames === 0) return;

    setScreenSignals((signals) => signals.filter((_, i) => i !== index));

    setTimeout(() => {
      const newTotal = totalFrames - 1;
      if (newTotal === 0) return;

      if (index >= newTotal) {
        focusFrame(setFocusedFrameIndex, scrollToFrame, newTotal - 1, 0);
      } else {
        focusFrame(setFocusedFrameIndex, scrollToFrame, index, 0);
      }
    }, 50);
  };

  const handleEmptyMatrix = (index: number) => {
    setScreenSignals((signals) => {
      if (index !== -1) {
        const [_, setScreen] = signals[index];
        setScreen(new Array(256).fill(0));
      }
      return signals;
    });
  };

  const handleDuplicateFrame = (index: number) => {
    setScreenSignals((signals) => {
      const frameToDuplicate = signals[index][0]();
      const newSignal = createNewScreen([...frameToDuplicate]);
      const newSignals = [...signals];
      newSignals.splice(index + 1, 0, newSignal);
      return newSignals;
    });

    focusFrame(setFocusedFrameIndex, scrollToFrame, index + 1);
  };

  const handleMoveFrame = (index: number, direction: "up" | "down") => {
    const newIndex = direction === "up" ? index - 1 : index + 1;
    if (newIndex < 0 || newIndex >= screenSignals().length) return;

    setScreenSignals((signals) => {
      const newSignals = [...signals];
      [newSignals[index], newSignals[newIndex]] = [newSignals[newIndex], newSignals[index]];
      return newSignals;
    });

    focusFrame(setFocusedFrameIndex, scrollToFrame, newIndex);
  };

  const handlePreviousFrame = () => {
    const prevIndex = focusedFrameIndex() - 1;
    if (prevIndex >= 0) {
      setFocusedFrameIndex(prevIndex);
      scrollToFrame(prevIndex);
    }
  };

  const handleNextFrame = () => {
    const nextIndex = focusedFrameIndex() + 1;
    if (nextIndex < screenSignals().length) {
      setFocusedFrameIndex(nextIndex);
      scrollToFrame(nextIndex);
    }
  };

  const handleUploadData = () => {
    if (isAnimationPluginActive()) {
      const screens = screenSignals().map(([screen]) => screen());

      actions.send(
        JSON.stringify({
          event: "upload",
          screens: screens.length,
          frameDelay: animationDelayMs(),
          data: screens.map((screen) => matrixToHexArray(screen.map((s) => (s > 0 ? 1 : 0)))),
        }),
      );

      toast("Data uploaded successfully!", 3000);
    } else {
      toast('Set plugin to "Animation"!', 3000);
    }
  };

  const handleExportJSON = () => {
    const screens = screenSignals().map(([screen]) => screen());
    const data = {
      version: 1,
      frames: screens,
    };

    const element = document.createElement("a");
    element.setAttribute(
      "href",
      `data:application/json;charset=utf-8,${encodeURIComponent(JSON.stringify(data, null, 2))}`,
    );
    element.setAttribute("download", "animation.json");
    element.style.display = "none";
    document.body.appendChild(element);
    element.click();
    element.remove();
    toast("Animation exported as JSON", 2000);
  };

  const handleImportJSON = () => {
    const input = document.createElement("input");
    input.type = "file";
    input.accept = ".json";
    input.onchange = (e: Event) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (!file) return;

      const reader = new FileReader();
      reader.onload = (event) => {
        try {
          const data = JSON.parse(event.target?.result as string);

          if (!data.frames || !Array.isArray(data.frames)) {
            toast("Invalid animation file format", 3000);
            return;
          }

          const newSignals = data.frames.map((frame: number[]) =>
            createNewScreen(frame),
          ) as FrameSignals;

          setScreenSignals(newSignals);
          toast(`Loaded ${data.frames.length} frames`, 2000);
          const lastIndex = newSignals.length - 1;
          focusFrame(setFocusedFrameIndex, scrollToFrame, lastIndex);
        } catch (error) {
          console.error("Failed to import animation:", error);
          toast("Failed to import animation", 3000);
        }
      };
      reader.readAsText(file);
    };
    input.click();
  };

  const handleSwitchToAnimationPlugin = () => {
    const animationPlugin = store?.plugins.find((p) => p.name.includes("Animation"));
    if (animationPlugin) {
      actions.send(
        JSON.stringify({
          event: "plugin",
          plugin: animationPlugin.id,
        }),
      );
      toast("Switched to Animation mode", 2000);
    }
  };

  const handleUndo = () => {
    const previousState = undo();
    if (!previousState) return;

    const newSignals = previousState.map((frame) => createNewScreen(frame)) as FrameSignals;
    setScreenSignals(newSignals);

    if (focusedFrameIndex() >= newSignals.length) {
      setFocusedFrameIndex(Math.max(0, newSignals.length - 1));
    }
  };

  const handleRedo = () => {
    const nextState = redo();
    if (!nextState) return;

    const newSignals = nextState.map((frame) => createNewScreen(frame)) as FrameSignals;
    setScreenSignals(newSignals);

    if (focusedFrameIndex() >= newSignals.length) {
      setFocusedFrameIndex(Math.max(0, newSignals.length - 1));
    }
  };

  const handleKeyDown = createKeyboardHandler(
    {
      onUndo: handleUndo,
      onRedo: handleRedo,
      onTogglePlay: togglePlay,
      onPreviousFrame: handlePreviousFrame,
      onNextFrame: handleNextFrame,
      onDeleteFrame: () => {
        if (screenSignals().length > 0) {
          handleRemoveScreen(focusedFrameIndex());
        }
      },
      onDuplicateFrame: () => {
        if (screenSignals().length > 0) {
          handleDuplicateFrame(focusedFrameIndex());
        }
      },
    },
    {
      hasFrames: () => screenSignals().length > 0,
      isPlaying,
    },
  );

  onMount(() => {
    const loaded = loadFromLocalStorage();
    if (loaded) {
      setScreenSignals(loaded.frames);
      if (loaded.animationDelayMs) {
        setAnimationDelayMs(loaded.animationDelayMs);
      }
    }

    setTimeout(() => {
      if (screenSignals().length > 0) {
        const currentState = screenSignals().map(([screen]) => [...screen()]);
        saveToHistory(currentState);
      }
    }, INITIAL_HISTORY_SAVE_DELAY_MS);
  });

  createEffect(() => {
    const signals = screenSignals();

    for (const [screen] of signals) {
      screen();
    }

    if (hasLoadedFromStorage() && signals.length > 0) {
      const timeoutId = setTimeout(() => {
        if (!isUndoRedoing()) {
          const currentState = signals.map(([screen]) => [...screen()]);
          saveToHistory(currentState);
        }
      }, HISTORY_DEBOUNCE_MS);

      onCleanup(() => clearTimeout(timeoutId));
    }
  });

  createEffect(() => {
    const signals = screenSignals();
    const focused = focusedFrameIndex();

    if (signals.length === 0) {
      setFocusedFrameIndex(0);
    } else if (focused >= signals.length) {
      setFocusedFrameIndex(signals.length - 1);
    } else if (focused < 0) {
      setFocusedFrameIndex(0);
    }
  });

  createEffect(() => {
    const signals = screenSignals();
    const delay = animationDelayMs();

    if (!hasLoadedFromStorage()) return;

    const frames = signals.map(([screen]) => screen());
    saveToLocalStorage(frames, delay);
  });

  createEffect(() => {
    window.addEventListener("keydown", handleKeyDown);
    onCleanup(() => {
      window.removeEventListener("keydown", handleKeyDown);
    });
  });

  return (
    <Layout
      content={
        <div class="flex min-h-full flex-col gap-4 p-4 lg:h-full lg:p-6">
          {screenSignals().length ? (
            <>
              {/* Main Frame Display */}
              <div class="flex min-h-0 flex-1 items-center justify-center">
                <Show
                  when={!isPlaying()}
                  fallback={
                    <div class="animate-fade-in w-full h-full flex items-center justify-center">
                      <LedMatrix
                        data={currentFrame()}
                        indexData={store.indexMatrix}
                        brightness={store.brightness || 255}
                      />
                    </div>
                  }
                >
                  <Show when={currentFrameSignals()}>
                    {(frameSignals) => (
                      <div class="flex h-full w-full flex-col items-center justify-center gap-4">
                        <FrameControls
                          focusedFrameIndex={focusedFrameIndex()}
                          totalFrames={screenSignals().length}
                          onMoveLeft={() => handleMoveFrame(focusedFrameIndex(), "up")}
                          onMoveRight={() => handleMoveFrame(focusedFrameIndex(), "down")}
                          onDuplicate={() => handleDuplicateFrame(focusedFrameIndex())}
                          onEmpty={() => handleEmptyMatrix(focusedFrameIndex())}
                          onRemove={() => handleRemoveScreen(focusedFrameIndex())}
                        />
                        <LedMatrix
                          data={frameSignals()[0]()}
                          indexData={store.indexMatrix}
                          brightness={store.brightness || 255}
                          onSetLed={(data) => {
                            const [screen, setScreen] = frameSignals();
                            const currentScreen = [...screen()];
                            currentScreen[data.index] = Number(data.status);
                            setScreen(currentScreen);
                          }}
                          onSetMatrix={(data) => {
                            const [_, setScreen] = frameSignals();
                            setScreen([...data]);
                          }}
                        />
                      </div>
                    )}
                  </Show>
                </Show>
              </div>

              <div class="shrink-0">
                <FrameTimeline
                  screenSignals={screenSignals()}
                  focusedFrameIndex={focusedFrameIndex()}
                  onFrameClick={setFocusedFrameIndex}
                  ref={(el) => {
                    timelineRef = el;
                  }}
                />
              </div>
            </>
          ) : (
            <ScreenInfo>
              <div class="mx-auto mb-4 grid size-14 place-items-center rounded-2xl bg-raised text-xl text-accent ring-1 ring-line">
                <i class="fa-solid fa-wand-magic-sparkles" />
              </div>
              <h2 class="mb-2 text-2xl font-semibold">Create something awesome</h2>
              <p class="mb-6 text-sm text-muted">Add a frame to start drawing your animation.</p>
              <Button widthAuto class="btn-accent" onClick={handleAddScreen}>
                <i class="fa-solid fa-plus" />
                Add first frame
              </Button>
            </ScreenInfo>
          )}
        </div>
      }
      sidebar={
        <div class="space-y-7">
          <SidebarSection title="Frames">
            <Show
              when={!isPlaying()}
              fallback={
                <Button disabled={screenSignals().length === 0} onClick={togglePlay}>
                  <i class="fa-solid fa-stop" />
                  Stop
                </Button>
              }
            >
              <div class="grid grid-cols-2 gap-2">
                <Button onClick={handleAddScreen}>
                  <i class="fa-solid fa-plus" />
                  Add
                </Button>
                <Button
                  class="btn-primary"
                  disabled={screenSignals().length === 0}
                  onClick={togglePlay}
                >
                  <i class="fa-solid fa-play" />
                  Play
                </Button>
                <Button disabled={!canUndo()} onClick={handleUndo}>
                  <i class="fa-solid fa-rotate-left" />
                  Undo
                </Button>
                <Button disabled={!canRedo()} onClick={handleRedo}>
                  <i class="fa-solid fa-rotate-right" />
                  Redo
                </Button>
              </div>
            </Show>
          </SidebarSection>

          <Show when={!isPlaying()}>
            <SidebarSection title="File">
              <div class="grid grid-cols-2 gap-2">
                <Button onClick={handleImportJSON}>
                  <i class="fa-solid fa-file-import" />
                  Import
                </Button>
                <Button disabled={screenSignals().length === 0} onClick={handleExportJSON}>
                  <i class="fa-solid fa-file-export" />
                  Export
                </Button>
              </div>
            </SidebarSection>
          </Show>

          <SidebarSection title="Device">
            <Show
              when={isAnimationPluginActive()}
              fallback={
                <>
                  <p class="text-sm text-muted">
                    The "Animation" plugin needs to be active to upload.
                  </p>
                  <Button onClick={handleSwitchToAnimationPlugin}>
                    <i class="fa-solid fa-power-off" />
                    Activate Animation
                  </Button>
                </>
              }
            >
              <Button
                class="btn-accent"
                disabled={screenSignals().length === 0 || isPlaying()}
                onClick={handleUploadData}
              >
                <i class="fa-solid fa-upload" />
                Upload to device
              </Button>
            </Show>
          </SidebarSection>

          <Show when={screenSignals().length > 0}>
            <AnimationSettings
              frameDurationId={frameDurationId}
              animationDelayMs={animationDelayMs()}
              onDelayChange={setAnimationDelayMs}
              totalFrames={screenSignals().length}
            />
          </Show>

          <KeyboardShortcutsHelp />
        </div>
      }
    />
  );
};

export default Creator;

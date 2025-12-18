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
import { ScreenInfo } from "./components/screen-info";
import { Tooltip } from "./components/tooltip";
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

  // Storage hook
  const { saveToLocalStorage, loadFromLocalStorage, hasLoadedFromStorage } =
    useAnimationStorage(createNewScreen);

  // History hook
  const { saveToHistory, undo, redo, canUndo, canRedo, isUndoRedoing } = useHistory<number[][]>({
    onUndo: () => toast("Undo", 1000),
    onRedo: () => toast("Redo", 1000),
  });

  // Animation hook
  const { isPlaying, currentFrame, togglePlay } = useAnimation(screenSignals, animationDelayMs);

  let timelineRef!: HTMLDivElement;

  // Get current focused frame safely with bounds checking
  const currentFrameSignals = () => {
    const index = focusedFrameIndex();
    const signals = screenSignals();
    if (index >= 0 && index < signals.length) {
      return signals[index];
    }
    return null;
  };

  // Helper to scroll timeline
  const scrollToFrame = (frameIndex: number) => {
    scrollTimelineToFrame(timelineRef, frameIndex, screenSignals().length);
  };

  // Frame manipulation handlers
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

  // Navigation handlers
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

  // Import/Export handlers
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

  // History integration with undo/redo
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

  // Keyboard handler setup
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

  // Load from local storage on mount
  onMount(() => {
    const loaded = loadFromLocalStorage();
    if (loaded) {
      setScreenSignals(loaded.frames);
      if (loaded.animationDelayMs) {
        setAnimationDelayMs(loaded.animationDelayMs);
      }
    }

    // Save initial state to history after loading
    setTimeout(() => {
      if (screenSignals().length > 0) {
        const currentState = screenSignals().map(([screen]) => [...screen()]);
        saveToHistory(currentState);
      }
    }, INITIAL_HISTORY_SAVE_DELAY_MS);
  });

  // Save to history when frames change (with debouncing)
  createEffect(() => {
    const signals = screenSignals();

    // Track all frame data as dependencies
    for (const [screen] of signals) {
      screen();
    }

    if (hasLoadedFromStorage() && signals.length > 0) {
      // Debounce: save to history after a short delay
      const timeoutId = setTimeout(() => {
        // Check isUndoRedoing again when timeout fires, not just when scheduling
        if (!isUndoRedoing()) {
          const currentState = signals.map(([screen]) => [...screen()]);
          saveToHistory(currentState);
        }
      }, HISTORY_DEBOUNCE_MS);

      onCleanup(() => clearTimeout(timeoutId));
    }
  });

  // Ensure focusedFrameIndex stays in bounds when frames change
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

  // Auto-save to local storage when frames or delay changes
  createEffect(() => {
    const signals = screenSignals();
    const delay = animationDelayMs();

    if (!hasLoadedFromStorage()) return;

    const frames = signals.map(([screen]) => screen());
    saveToLocalStorage(frames, delay);
  });

  // Keyboard event listener
  createEffect(() => {
    window.addEventListener("keydown", handleKeyDown);
    onCleanup(() => {
      window.removeEventListener("keydown", handleKeyDown);
    });
  });

  return (
    <Layout
      content={
        <div class="h-full flex flex-col">
          {screenSignals().length ? (
            <>
              {/* Main Frame Display */}
              <div class="flex-1 flex flex-col items-center justify-center p-8">
                <Show
                  when={!isPlaying()}
                  fallback={
                    <div class="animate-fade-in">
                      <LedMatrix data={currentFrame()} indexData={store.indexMatrix} />
                    </div>
                  }
                >
                  <Show when={currentFrameSignals()}>
                    {(frameSignals) => (
                      <>
                        <div class="max-w-2xl">
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
                      </>
                    )}
                  </Show>
                </Show>
              </div>

              {/* Timeline at Bottom */}
              <FrameTimeline
                screenSignals={screenSignals()}
                focusedFrameIndex={focusedFrameIndex()}
                onFrameClick={setFocusedFrameIndex}
                ref={(el) => {
                  timelineRef = el;
                }}
              />
            </>
          ) : (
            <ScreenInfo>
              <h2 class="text-4xl">Create something awesome! 🙌.</h2>
            </ScreenInfo>
          )}
        </div>
      }
      sidebar={
        <>
          <div class="space-y-6">
            <div class="space-y-3">
              <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">Controls</h3>
              <div class="flex gap-2 items-center flex-wrap">
                <Show
                  when={!isPlaying()}
                  fallback={
                    <Tooltip text="Stop animation">
                      <Button
                        disabled={screenSignals().length === 0}
                        onClick={togglePlay}
                        class="hover:bg-red-600 transition-colors"
                      >
                        <i class="fa-solid fa-stop" />
                      </Button>
                    </Tooltip>
                  }
                >
                  <Tooltip text="Add new frame">
                    <Button
                      disabled={!isAnimationPluginActive()}
                      onClick={handleAddScreen}
                      class="hover:bg-green-600 transition-colors"
                    >
                      <i class="fa-solid fa-plus" />
                    </Button>
                  </Tooltip>

                  <Tooltip text="Play animation">
                    <Button
                      disabled={screenSignals().length === 0}
                      onClick={togglePlay}
                      class="hover:bg-green-600 transition-colors"
                    >
                      <i class="fa-solid fa-play" />
                    </Button>
                  </Tooltip>

                  <Tooltip text="Undo (Ctrl+Z)">
                    <Button
                      disabled={!canUndo()}
                      onClick={handleUndo}
                      class="hover:bg-gray-600 transition-colors"
                    >
                      <i class="fa-solid fa-undo" />
                    </Button>
                  </Tooltip>

                  <Tooltip text="Redo (Ctrl+Shift+Z)">
                    <Button
                      disabled={!canRedo()}
                      onClick={handleRedo}
                      class="hover:bg-gray-600 transition-colors"
                    >
                      <i class="fa-solid fa-redo" />
                    </Button>
                  </Tooltip>

                  <div class="w-full border-t border-gray-200 my-2" />

                  <Tooltip text="Import JSON">
                    <Button
                      onClick={handleImportJSON}
                      class="hover:bg-purple-600 transition-colors"
                    >
                      <i class="fa-solid fa-file-import mr-1" />
                      <span class="text-xs">Import</span>
                    </Button>
                  </Tooltip>

                  <Tooltip text="Export as JSON">
                    <Button
                      disabled={screenSignals().length === 0}
                      onClick={handleExportJSON}
                      class="hover:bg-purple-600 transition-colors"
                    >
                      <i class="fa-solid fa-file-export mr-1" />
                      <span class="text-xs">Export</span>
                    </Button>
                  </Tooltip>

                  <Tooltip text="Upload to device">
                    <Button
                      disabled={screenSignals().length === 0}
                      onClick={handleUploadData}
                      class="hover:bg-blue-700 transition-colors"
                    >
                      <i class="fa-solid fa-upload" />
                    </Button>
                  </Tooltip>
                </Show>
              </div>
            </div>

            <Show when={!isAnimationPluginActive()}>
              <div class="space-y-3">
                <p class="text-sm text-gray-500 font-bold">
                  The "Animation" plugin needs to be active to use this feature.
                </p>
                <Button
                  onClick={handleSwitchToAnimationPlugin}
                  class="hover:bg-green-600 transition-colors w-full"
                >
                  <i class="fa-solid fa-play mr-2" />
                  Switch to Animation Mode
                </Button>
              </div>
            </Show>

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

          <div class="mt-auto pt-6 border-t border-gray-200">
            <Tooltip text="Return to main editor">
              <a
                href="#/"
                class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
              >
                <i class="fa-solid fa-arrow-left mr-2" />
                Back
              </a>
            </Tooltip>
          </div>
        </>
      }
    />
  );
};

export default Creator;

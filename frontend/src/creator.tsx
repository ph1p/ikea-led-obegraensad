import { type Component, createEffect, createMemo, createSignal, For, Show } from "solid-js";

import { Button } from "./components/button";
import { Layout } from "./components/layout/layout";
import { LedMatrix } from "./components/led-matrix";
import { ScreenInfo } from "./components/screen-info";
import { Tooltip } from "./components/tooltip";
import { useStore } from "./contexts/store";
import { useToast } from "./contexts/toast";
import { chunkArray, matrixToHexArray } from "./helpers";

export const Creator: Component = () => {
  const [store, actions] = useStore();
  const { toast } = useToast();
  const isAnimationPluginActive = createMemo<boolean>(
    () => store?.plugins.find((p) => p.name.includes("Animation"))?.id === store?.plugin,
    true,
  );

  const [isPlaying, setIsPlaying] = createSignal(false);
  const [screenSignals, setScreenSignals] = createSignal<
    ReturnType<typeof createSignal<number[]>>[]
  >([]);
  const [currentFrame, setCurrentFrame] = createSignal<number[]>([]);
  const [intervalId, setIntervalId] = createSignal(0);

  let ref!: HTMLDivElement;

  const createNewScreen = (initialData?: number[]) => {
    const [screen, setScreen] = createSignal(initialData || new Array(256).fill(0));
    return [screen, setScreen] as const;
  };

  const scrollToEnd = () => {
    if (ref) {
      setTimeout(() => {
        ref?.scrollTo({
          top: 0,
          left: ref?.scrollWidth,
          behavior: "smooth",
        });
      }, 20);
    }
  };

  const handleAddScreen = () => {
    setScreenSignals((signals): ReturnType<typeof setScreenSignals> => {
      const lastScreen = signals.length > 0 ? signals[signals.length - 1][0]() : undefined;
      const newSignal = createNewScreen(lastScreen ? [...lastScreen] : undefined);
      scrollToEnd();
      return [...signals, newSignal as unknown as ReturnType<typeof createSignal<number[]>>];
    });
  };

  const handleRemoveScreen = (index: number) => {
    setScreenSignals((signals) => signals.filter((_, i) => i !== index));
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

  const handleUploadData = () => {
    if (isAnimationPluginActive()) {
      const screens = screenSignals().map(([screen]) => screen());

      actions.send(
        JSON.stringify({
          event: "upload",
          screens: screens.length,
          data: screens.map((screen) => matrixToHexArray(screen.map((s) => (s > 0 ? 1 : 0)))),
        }),
      );

      toast("Data uploaded successfully!", 3000);
    } else {
      toast('Set plugin to "Animation"!', 3000);
    }
  };

  const handleExportData = () => {
    const animation = [];
    const screens = screenSignals().map(([screen]) => screen());
    for (const screen of screens) {
      animation.push(
        chunkArray(screen, 8).map(
          (chunk) => `0x${parseInt(chunk.join(""), 2).toString(16).padStart(2, "0")}`,
        ),
      );
    }

    const element = document.createElement("a");
    element.setAttribute(
      "href",
      "data:text/plain;charset=utf-8," +
        encodeURIComponent(
          `std::vector<std::vector<int>> frames = ${JSON.stringify(animation)
            .replace(/"/g, "")
            .replace(/]/g, "}")
            .replace(/\[/g, "{")};

int size = frames.size();
int count = 0;
if (size > 0)
{
    std::vector<int> bits = Screen.readBytes(frames[count]);
    for (int i = 0; i < bits.size(); i++)
    {
        Screen.setPixelAtIndex(i, bits[i]);
    }
    Screen.render();
    count++;
    if (count >= size)
    {
        count = 0;
    }
    delay(400);
}`,
        ),
    );
    element.setAttribute("download", "animation.cpp");
    element.style.display = "none";
    document.body.appendChild(element);
    element.click();
    element.remove();
  };

  const handleTogglePlay = () => {
    setIsPlaying(!isPlaying());
  };

  createEffect(() => {
    if (!isPlaying()) {
      clearInterval(intervalId());
      setIntervalId(0);
      scrollToEnd();
      return;
    }

    let i = 0;
    const run = () => {
      const screen = screenSignals()[i][0]();
      setCurrentFrame([...screen]);
      i++;
      if (i >= screenSignals().length) {
        i = 0;
      }
    };
    run();
    const newIntervalId = setInterval(run, 400) as unknown as number;
    setIntervalId(newIntervalId);

    return () => {
      clearInterval(newIntervalId);
      setIntervalId(0);
      scrollToEnd();
    };
  });

  return (
    <Layout
      content={
        <div class="h-full relative">
          {screenSignals().length ? (
            <div
              ref={ref}
              class={`snap-x snap-mandatory flex flex-nowrap overflow-x-auto gap-[15vw] h-full items-center w-[calc(100vw-320px)] ${
                isPlaying() ? "justify-center" : "justify-center px-[calc(50vw-320px)]"
              }`}
            >
              <Show
                when={!isPlaying()}
                fallback={
                  <div class="animate-fade-in">
                    <LedMatrix data={currentFrame()} indexData={store.indexMatrix} />
                  </div>
                }
              >
                <For each={screenSignals()}>
                  {([screen, setScreen], index) => (
                    <div class="snap-center">
                      <header class="flex justify-between items-center mb-4">
                        <div class="flex gap-2">
                          <Tooltip text="Remove current frame">
                            <Button
                              widthAuto
                              onClick={() => handleRemoveScreen(index())}
                              class="hover:bg-red-600 transition-colors w-auto"
                            >
                              <i class="fa-solid fa-trash" />
                            </Button>
                          </Tooltip>

                          <Tooltip text="Empty current frame">
                            <Button
                              onClick={() => handleEmptyMatrix(index())}
                              class="hover:bg-red-600 transition-colors"
                            >
                              <i class="fa-solid fa-eraser" />
                            </Button>
                          </Tooltip>
                        </div>
                        <div class="text-center text-2xl text-white flex items-center">
                          <span class="font-bold">{index() + 1}</span>
                          <span class="opacity-50">/{screenSignals().length}</span>
                        </div>
                      </header>
                      <LedMatrix
                        data={screen()}
                        indexData={store.indexMatrix}
                        onSetLed={(data) => {
                          const currentScreen = [...screen()];
                          currentScreen[data.index] = Number(data.status);
                          setScreen(currentScreen);
                        }}
                        onSetMatrix={(data) => {
                          setScreen([...data]);
                        }}
                      />
                    </div>
                  )}
                </For>
              </Show>
            </div>
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
              <div class="flex gap-4 items-center">
                <Show
                  when={!isPlaying()}
                  fallback={
                    <Tooltip text="Stop animation">
                      <Button
                        disabled={screenSignals().length === 0}
                        onClick={handleTogglePlay}
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

                  <Tooltip text="Export animation">
                    <Button
                      disabled={screenSignals().length === 0}
                      onClick={handleExportData}
                      class="hover:bg-blue-700 transition-colors"
                    >
                      <i class="fa-solid fa-download" />
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

                  <Tooltip text="Play animation">
                    <Button
                      disabled={screenSignals().length === 0}
                      onClick={handleTogglePlay}
                      class="hover:bg-green-600 transition-colors"
                    >
                      <i class="fa-solid fa-play" />
                    </Button>
                  </Tooltip>
                </Show>
              </div>
            </div>

            <Show when={!isAnimationPluginActive()}>
              <p class="text-sm text-gray-500 font-bold">
                Activate the "Animation" plugin to use this feature!
              </p>
            </Show>
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

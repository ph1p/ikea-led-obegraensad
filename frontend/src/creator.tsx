import { Component, createEffect, createSignal, For, Show } from 'solid-js';
import { Button } from './components/button';
import CreatorSidebar from './components/creator-sidebar';
import { Layout } from './components/layout';
import { LedMatrix } from './components/led-matrix';
import { Tooltip } from './components/tooltip';
import { chunkArray, matrixToHexArray } from './helpers';
import { useStore } from './store';

export const Creator: Component = () => {
  const store = useStore();
  const [isPlaying, setIsPlaying] = createSignal(false);
  const [screenSignals, setScreenSignals] = createSignal<
    ReturnType<typeof createSignal<number[]>>[]
  >([]);
  const [currentFrame, setCurrentFrame] = createSignal<number[]>([]);
  const [intervalId, setIntervalId] = createSignal(0);

  let ref: HTMLDivElement;

  const createNewScreen = (initialData?: number[]) => {
    const [screen, setScreen] = createSignal(
      initialData || new Array(256).fill(0)
    );
    return [screen, setScreen] as const;
  };

  const scrollToEnd = () => {
    if (ref) {
      setTimeout(() => {
        ref?.scrollTo({
          top: 0,
          left: ref?.scrollWidth,
          behavior: 'smooth',
        });
      }, 20);
    }
  };

  const handleAddScreen = () => {
    setScreenSignals((signals) => {
      const lastScreen =
        signals.length > 0 ? signals[signals.length - 1][0]() : undefined;
      const newSignal = createNewScreen(
        lastScreen ? [...lastScreen] : undefined
      );
      scrollToEnd();
      return [...signals, newSignal];
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
    const screens = screenSignals().map(([screen]) => screen());

    store!.send(
      JSON.stringify({
        event: 'upload',
        screens: screens.length,
        data: screens.map((screen) =>
          matrixToHexArray(screen.map((s) => (s > 0 ? 1 : 0)))
        ),
      })
    );

    const id = store?.plugins().find((p) => p.name.includes('Animation'))?.id;
    if (id) {
      store!.send(
        JSON.stringify({
          event: 'plugin',
          plugin: id,
        })
      );
    }
  };

  const handleExportData = () => {
    const animation = [];
    const screens = screenSignals().map(([screen]) => screen());
    for (const screen of screens) {
      animation.push(
        chunkArray(screen, 8).map(
          (chunk) =>
            `0x${parseInt(chunk.join(''), 2).toString(16).padStart(2, '0')}`
        )
      );
    }

    const element = document.createElement('a');
    element.setAttribute(
      'href',
      'data:text/plain;charset=utf-8,' +
        encodeURIComponent(
          `std::vector<std::vector<int>> frames = ${JSON.stringify(animation)
            .replace(/"/g, '')
            .replace(/]/g, '}')
            .replace(/\[/g, '{')};

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
}`
        )
    );
    element.setAttribute('download', 'animation.cpp');
    element.style.display = 'none';
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
    const newIntervalId: any = setInterval(run, 400);
    setIntervalId(newIntervalId);

    return () => {
      clearInterval(newIntervalId);
      setIntervalId(0);
      scrollToEnd();
    };
  });

  const renderFrames = () => (
    <div
      ref={ref!}
      class="snap-x snap-mandatory flex flex-nowrap overflow-x-auto gap-[15vw] h-full w-[calc(100vw-320px)] items-center px-[calc(50vw-320px)]"
    >
      <Show
        when={!isPlaying()}
        fallback={
          <div class="animate-fade-in">
            <LedMatrix data={currentFrame} indexData={store!.indexMatrix} />
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
                data={screen}
                indexData={store!.indexMatrix}
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
  );

  return (
    <Show
      when={store!.connectionState() === 1}
      fallback={
        <Layout
          content={
            <div class="h-full relative">
              <div class="text-center text-2xl text-white h-full flex justify-center items-center animate-fade-in">
                {store?.connectionStatus}...
              </div>
            </div>
          }
          sidebarContent={
            <div class="bg-white p-6">
              <a href="#/" class="text-gray-700 hover:text-gray-900">
                Back
              </a>
            </div>
          }
        />
      }
    >
      <Layout
        content={
          <div class="h-full relative">
            {screenSignals().length ? (
              renderFrames()
            ) : (
              <div class="text-center text-2xl text-white h-full flex justify-center items-center animate-fade-in">
                Create something awesome! 🙌
              </div>
            )}
          </div>
        }
        sidebarContent={
          <CreatorSidebar
            hasScreens={screenSignals().length > 0}
            isPlaying={isPlaying()}
            onAddScreen={handleAddScreen}
            onExportData={handleExportData}
            onUploadData={handleUploadData}
            onTogglePlay={handleTogglePlay}
          />
        }
      />
    </Show>
  );
};

export default Creator;

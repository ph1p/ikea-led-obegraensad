import createWebsocket from '@solid-primitives/websocket';
import { createSignal, Index, onCleanup, onMount, Show } from 'solid-js';
import { Button } from './components/Button';
import { Layout } from './components/Layout';
import { LedMatrix } from './components/LedMatrix';
import {
  emptyScreen,
  screenCount,
  screenHeader,
  screensWrapper,
  wrapper,
} from './creator.css';
import { chunkArray, matrixToHexArray } from './helpers';
import { connectionInformation } from './index.css';
import { useStore } from './store';

export const Creator = () => {
  const store = useStore();

  const [isPlaying, setIsPlaying] = createSignal(false);
  const [screens, setScreens] = createSignal<number[][]>([]);
  const [currentFrame, setCurrentFrame] = createSignal<number[]>([]);
  const [intervalId, setIntervalId] = createSignal(0);

  let ref: HTMLDivElement;

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

  const addScreen = () => {
    setScreens((state) => {
      if (state.length > 0) {
        state.push([...state[state.length - 1]]);
      } else {
        state.push([...new Array(256)].fill(0));
      }

      scrollToEnd();
      return JSON.parse(JSON.stringify(state));
    });
  };

  const removeScreen = (index: number) => {
    setScreens((state) => state.filter((_, i) => i !== index));
  };

  const uploadData = () => {
    store!.send(
      JSON.stringify({
        event: 'upload',
        screens: screens().length,
        data: screens().map((screen) => matrixToHexArray(screen)),
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

  const exportData = () => {
    const animation = [];
    for (const screen of screens()) {
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

  onMount(() => {
    if (!isPlaying()) {
      clearInterval(intervalId());
      setIntervalId(0);
      scrollToEnd();
      return;
    }

    let i = 0;
    const run = () => {
      setCurrentFrame([...screens()[i]]);
      i++;
      if (i >= screens().length) {
        i = 0;
      }
    };
    run();
    const newIntervalId: any = setInterval(run, 400);
    setInterval(newIntervalId);

    return () => {
      clearInterval(newIntervalId);
      setIntervalId(0);
      scrollToEnd();
    };
  });

  return (
    <Show
      when={store!.connectionState() === 1}
      fallback={
        <Layout
          content={
            <div class={wrapper}>
              <div class={connectionInformation}>
                {store?.connectionStatus}...
              </div>
            </div>
          }
          footer={
            <>
              <a href="#/">back</a>
            </>
          }
        ></Layout>
      }
    >
      <Layout
        content={
          <div class={wrapper}>
            {screens().length ? (
              <div class={screensWrapper} ref={ref!}>
                <Show
                  when={!isPlaying()}
                  fallback={
                    <LedMatrix
                      data={currentFrame()}
                      indexData={store!.indexMatrix()}
                    />
                  }
                >
                  <Index each={screens()}>
                    {(screen, index) => (
                      <div>
                        <header class={screenHeader}>
                          <Button onClick={() => removeScreen(index)}>
                            <i class="fa-solid fa-trash"></i>
                          </Button>
                          <div class={screenCount}>
                            {index + 1}
                            <span>/{screens().length}</span>
                          </div>
                        </header>
                        <LedMatrix
                          data={screen()}
                          indexData={store!.indexMatrix()}
                          onSetLed={(data) => {
                            setScreens((state) => {
                              state[index][data.index] = Number(data.status);
                              return JSON.parse(JSON.stringify(state));
                            });
                          }}
                          onSetMatrix={(data) => {
                            setScreens((state) =>
                              state.map((s, i) => (i === index ? [...data] : s))
                            );
                          }}
                        />
                      </div>
                    )}
                  </Index>
                </Show>
              </div>
            ) : (
              <div class={emptyScreen}>Create something awesome! 🙌</div>
            )}
          </div>
        }
        footer={
          <>
            {!isPlaying() && (
              <>
                <Button onClick={addScreen}>
                  <i class="fa-solid fa-plus"></i>
                </Button>
                <Button onClick={exportData}>
                  <i class="fa-solid fa-download"></i>
                </Button>
                <Button onClick={uploadData}>
                  <i class="fa-solid fa-upload"></i>
                </Button>
              </>
            )}

            {screens.length > 0 && (
              <Button onClick={() => setIsPlaying(!isPlaying())}>
                <i class={`fa-solid fa-${isPlaying() ? 'stop' : 'play'}`}></i>
              </Button>
            )}

            {!isPlaying() && <a href="#/">back</a>}
          </>
        }
      />
    </Show>
  );
};

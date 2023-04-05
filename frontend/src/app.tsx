import { loadImageAndGetDataArray, rotateArray } from './helpers';
import { MODE } from './types';
import { controlColumn, wrapper } from './app.css';
import { LedMatrix } from './components/LedMatrix';
import { Button } from './components/Button';
import { Layout } from './components/Layout';
import { connectionInformation } from './index.css';
import { Component, createMemo, createSignal, Show } from 'solid-js';
import { useStore } from './store';

export const App: Component = () => {
  const store = useStore();
  const [triggerClear, setTriggerClear] = createSignal(false);

  const rotatedMatrix = createMemo(() =>
    rotateArray(store!.indexMatrix(), store!.rotation())
  );

  const wsMessage = (
    event:
      | 'persist'
      | 'load'
      | 'clear'
      | 'mode'
      | 'screen'
      | 'led'
      | 'persist-mode'
      | 'brightness',
    data?: any
  ) =>
    store?.send(
      JSON.stringify({
        event,
        ...data,
      })
    );

  const loadImage = () => {
    loadImageAndGetDataArray((data) => {
      store?.setLeds(() => store?.indexMatrix().map((index) => data[index]));
      wsMessage('screen', { data });
    });
  };

  const clear = () => {
    store?.setLeds([...new Array(256).fill(0)]);
    setTriggerClear(!triggerClear);
    wsMessage('clear');

    store?.toast(`Canvas cleared`, 1000);
  };

  const rotate = (turnRight = false) => {
    let currentRotation = store?.rotation() || 0;

    currentRotation = turnRight
      ? currentRotation > 3
        ? 1
        : currentRotation + 1
      : currentRotation <= 0
        ? 3
        : currentRotation - 1;

    store?.setRotation(currentRotation);

    store?.send(
      JSON.stringify({
        event: 'rotate',
        direction: turnRight ? 'right' : 'left',
      })
    );
  };

  const sendMode = (mode: MODE) => wsMessage('mode', { mode });

  const sendBrightness = (brightness: number) => wsMessage('brightness', { brightness });

  return (
    <Show
      when={store?.connectionState() === 1}
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
              <a href="#/creator">creator</a>
            </>
          }
        ></Layout>
      }
    >
      <Layout
        content={
          <div class={wrapper}>
            <div>
              <LedMatrix
                disabled={store?.mode() !== MODE.NONE}
                data={store?.leds() || []}
                indexData={rotatedMatrix()}
                onSetLed={(data) => {
                  wsMessage('led', data);
                }}
                onSetMatrix={(data) => {
                  store?.setLeds([...data]);
                }}
              />
            </div>
          </div>
        }
        footer={
          <>
            <div class={controlColumn}>
              <select
                onChange={(e) => {
                  const currentMode = e.currentTarget.value as MODE;
                  store?.setMode(currentMode);
                  sendMode(currentMode === 'draw' ? MODE.NONE : currentMode);

                  store?.toast('Mode changed', 1000);
                }}
                value={store?.mode()}
              >
                <option value={MODE.NONE}>draw</option>
                <option value={MODE.STARS}>stars</option>
                <option value={MODE.LINES}>lines</option>
                <option value={MODE.BREAKOUT}>breakout</option>
                <option value={MODE.SNAKE}>snake</option>
                <option value={MODE.GAMEOFLIFE}>game of life</option>
                <option value={MODE.CIRCLE}>circle</option>
                <option value={MODE.CLOCK}>clock</option>
                <option value={MODE.BIGCLOCK}>big clock</option>
                <option value={MODE.CUSTOM}>custom</option>
              </select>

              <Button
                onClick={() => {
                  wsMessage('persist-mode');
                  store?.toast(`Current mode set as default`, 1500);
                }}
              >
                set as default
              </Button>
            </div>

            <div class={controlColumn}>
              <Button onClick={() => rotate(false)}>
                <i class="fa-solid fa-rotate-left"></i>
              </Button>
              <Button onClick={() => rotate(true)}>
                <i class="fa-solid fa-rotate-right"></i>
              </Button>
            </div>

            <div class={controlColumn}>
              <input type="range" min="0" max="255" value={store?.brightness()} onInput={(e) => {
                const brightness = parseInt(e.currentTarget.value);
                store?.setBrightness(brightness);
                sendBrightness(brightness);
              }} />
            </div>

            <div class={controlColumn}>
              {store?.mode() === MODE.NONE && (
                <>
                  <Button onClick={() => loadImage()}>
                    <i class="fa-solid fa-file-import"></i>
                  </Button>
                  <Button onClick={clear}>
                    <i class="fa-solid fa-trash"></i>
                  </Button>
                  <Button
                    onClick={() => {
                      store?.toast(`Saved current state`, 1500);
                      wsMessage('persist');
                    }}
                  >
                    <i class="fa-solid fa-floppy-disk"></i>
                  </Button>
                  <Button
                    onClick={() => {
                      store?.toast(`Saved state loaded`, 1500);
                      wsMessage('load');
                    }}
                  >
                    <i class="fa-solid fa-refresh"></i>
                  </Button>
                </>
              )}

              <a href="#/creator">creator</a>
            </div>
          </>
        }
      />
    </Show>
  );
};

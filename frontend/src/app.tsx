import { loadImageAndGetDataArray, rotateArray } from './helpers';
import { MODE } from './types';
import { controlColumn, wrapper } from './app.css';
import { LedMatrix } from './components/LedMatrix';
import { Button } from './components/Button';
import { Layout } from './components/Layout';
import { connectionInformation } from './index.css';
import { Component, createMemo, createSignal, onCleanup, onMount, Show } from 'solid-js';
import createWebsocket from '@solid-primitives/websocket';

export const App: Component = () => {
  const [rotation, setRotation] = createSignal<number>(0);
  const [triggerClear, setTriggerClear] = createSignal(false);
  const [indexMatrix, setIndexMatrix] = createSignal<number[]>(
    [...new Array(256)].map((_, i) => i),
    { equals: false }
  );
  const [leds, setLeds] = createSignal<number[]>([...new Array(256)].fill(0), {
    equals: false,
  });
  const [mode, setMode] = createSignal<MODE>(MODE.NONE);

  const rotatedMatrix = createMemo(() =>
    rotateArray(indexMatrix(), rotation())
  );

  const [connect, disconnect, send, connectionState] = createWebsocket(
    `${
      import.meta.env.PROD
        ? window.location.href.replace('http', 'ws')
        : import.meta.env.VITE_WS_URL
    }ws`,
    (event) => {
      try {
        const json = JSON.parse(event.data);

        switch (json.event) {
          case 'mode':
            setMode(Object.values(MODE)[json.mode as number]);
            break;
          case 'info':
            setMode(Object.values(MODE)[json.mode as number]);
            setRotation(json.rotation);

            setIndexMatrix([...new Array(256)].map((_, i) => i));

            if (json.data) {
              setLeds(json.data);
            }

            break;
        }
      } catch {}
    },
    () => {},
    [],
    3,
    5000
  );

  const loadImage = () => {
    loadImageAndGetDataArray((data) => {
      setLeds(() => indexMatrix().map((index) => data[index]));
      wsMessage('screen', { data });
    });
  };

  const clear = () => {
    setLeds([...new Array(256).fill(0)]);
    setTriggerClear(!triggerClear);
    wsMessage('clear');
  };

  const rotate = (turnRight = false) => {
    let currentRotation = rotation();

    currentRotation = turnRight
      ? currentRotation > 3
        ? 1
        : currentRotation + 1
      : currentRotation <= 0
      ? 3
      : currentRotation - 1;

    setRotation(currentRotation);

    send(
      JSON.stringify({
        event: 'rotate',
        direction: turnRight ? 'right' : 'left',
      })
    );
  };

  const wsMessage = (
    event:
      | 'persist'
      | 'load'
      | 'clear'
      | 'mode'
      | 'screen'
      | 'led'
      | 'persist-mode',
    data?: any
  ) =>
    send(
      JSON.stringify({
        event,
        ...data,
      })
    );

  const sendMode = (mode: MODE) => wsMessage('mode', { mode });

  const connectionStatus = {
    0: 'Connecting',
    1: 'Open',
    2: 'Closing',
    3: 'Try to reconnect',
  }[connectionState()];

  onMount(() => {
    connect();
  });

  onCleanup(() => disconnect());

  return (
    <Show
      when={connectionState() === 1}
      fallback={
        <Layout
          content={
            <div class={wrapper}>
              <div class={connectionInformation}>{connectionStatus}...</div>
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
                disabled={mode() !== MODE.NONE}
                data={leds()}
                indexData={rotatedMatrix()}
                onSetLed={(data) => {
                  wsMessage('led', data);
                }}
                onSetMatrix={(data) => {
                  setLeds([...data]);
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
                  setMode(currentMode);
                  sendMode(currentMode === 'draw' ? MODE.NONE : currentMode);
                }}
                value={mode()}
              >
                <option value={MODE.NONE}>draw</option>
                <option value={MODE.STARS}>stars</option>
                <option value={MODE.LINES}>lines</option>
                <option value={MODE.BREAKOUT}>breakout</option>
                <option value={MODE.GAMEOFLIFE}>game of life</option>
                <option value={MODE.CIRCLE}>circle</option>
                <option value={MODE.CLOCK}>clock</option>
                <option value={MODE.CUSTOM}>custom</option>
              </select>

              <Button onClick={() => wsMessage('persist-mode')}>
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
              {mode() === MODE.NONE && (
                <>
                  <Button onClick={() => loadImage()}>
                    <i class="fa-solid fa-file-import"></i>
                  </Button>
                  <Button onClick={clear}>
                    <i class="fa-solid fa-trash"></i>
                  </Button>
                  <Button onClick={() => wsMessage('persist')}>
                    <i class="fa-solid fa-floppy-disk"></i>
                  </Button>
                  <Button onClick={() => wsMessage('load')}>
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

import { useEffect, useState } from 'preact/hooks';
import { ReadyState } from 'react-use-websocket/dist/lib/constants';
import { useWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import {
  loadImageAndGetDataArray,
  rotateArray,
  rotateArrayByDegree,
} from './helpers';
import { MODE } from './types';
import { controlColumn, wrapper } from './app.css';
import { LedMatrix } from './components/LedMatrix';
import { Button } from './components/Button';
import { Layout } from './components/Layout';
import { connectionInformation } from './main.css';

export function App() {
  const [rotation, setRotation] = useState<number>(0);
  const [triggerClear, setTriggerClear] = useState(false);
  const [indexMatrix, setIndexMatrix] = useState<number[]>(
    [...new Array(256)].map((_, i) => i)
  );
  const [leds, setLeds] = useState<number[]>([...new Array(256)].fill(0));
  const [mode, setMode] = useState<MODE>(MODE.NONE);
  const { sendMessage, readyState } = useWebSocket(
    `${
      (import.meta as any).PROD
        ? location.href.replace('http', 'ws')
        : import.meta.env.VITE_WS_URL
    }ws`,
    {
      share: true,
      shouldReconnect: () => true,
      reconnectAttempts: 10,
      reconnectInterval: 3000,
      retryOnError: true,
      onMessage: (event) => {
        try {
          const json = JSON.parse(event.data);

          switch (json.event) {
            case 'mode':
              setMode(Object.values(MODE)[json.mode as number]);
              break;
            case 'info':
              setMode(Object.values(MODE)[json.mode as number]);
              setRotation(json.rotation);

              setIndexMatrix([
                ...rotateArrayByDegree(
                  [...new Array(256)].map((_, i) => i),
                  json.rotation
                ),
              ]);
              if (json.data) {
                setLeds(rotateArrayByDegree(json.data, json.rotation));
              }

              break;
          }
        } catch {}
      },
    }
  );

  const loadImage = () => {
    loadImageAndGetDataArray((data) => {
      setLeds(() => indexMatrix.map((index) => data[index]));
      wsMessage('screen', { data });
    });
  };

  const clear = () => {
    setLeds([...new Array(256).fill(0)]);
    setTriggerClear(!triggerClear);
    wsMessage('clear');
  };

  const rotate = (turnRight = false) => {
    if (turnRight) {
      setRotation(rotation + 90);
    } else {
      setRotation(rotation - 90);
    }

    if (rotation <= -360 || rotation >= 360) {
      setRotation(0);
    }

    setLeds((state: number[]) => rotateArray(state, turnRight));
    setIndexMatrix((state: number[]) => rotateArray(state, turnRight));

    sendMessage(
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
    sendMessage(
      JSON.stringify({
        event,
        ...data,
      })
    );

  const sendMode = (mode: MODE) => wsMessage('mode', { mode });

  const connectionStatus = {
    [ReadyState.CONNECTING]: 'Connecting',
    [ReadyState.OPEN]: 'Open',
    [ReadyState.CLOSING]: 'Closing',
    [ReadyState.CLOSED]: 'Try to reconnect',
    [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
  }[readyState];

  if (readyState !== ReadyState.OPEN) {
    return (
      <Layout
        content={
          <div className={wrapper}>
            <div className={connectionInformation}>{connectionStatus}...</div>
          </div>
        }
        footer={
          <>
            <a href="#/creator">creator</a>
          </>
        }
      ></Layout>
    );
  }

  return (
    <Layout
      content={
        <div className={wrapper}>
          <div>
            <LedMatrix
              disabled={mode !== MODE.NONE}
              data={leds}
              indexData={indexMatrix}
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
          <div className={controlColumn}>
            <select
              onChange={(e) => {
                const currentMode = e.currentTarget.value as MODE;
                setMode(currentMode);
                sendMode(currentMode);
              }}
              value={mode}
            >
              <option value={MODE.NONE}>draw</option>
              <option value={MODE.STARS}>stars</option>
              <option value={MODE.LINES}>lines</option>
              <option value={MODE.BREAKOUT}>breakout</option>
              <option value={MODE.SNAKE}>snake</option>
              <option value={MODE.GAMEOFLIFE}>game of life</option>
              <option value={MODE.CIRCLE}>circle</option>
              <option value={MODE.CLOCK}>clock</option>
              <option value={MODE.CUSTOM}>custom</option>
            </select>

            <Button onClick={() => wsMessage('persist-mode')}>
              set as default
            </Button>
          </div>

          <div className={controlColumn}>
            <Button onClick={() => rotate(false)}>
              <i class="fa-solid fa-rotate-left"></i>
            </Button>
            <Button onClick={() => rotate(true)}>
              <i class="fa-solid fa-rotate-right"></i>
            </Button>
          </div>

          <div className={controlColumn}>
            {mode === MODE.NONE && (
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
  );
}

import { useState } from 'preact/hooks';
import { ReadyState } from 'react-use-websocket/dist/lib/constants';
import { useWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import {
  loadImageAndGetDataArray,
  rotateArray,
  rotateArrayByDegree,
} from './helpers';
import { MODE } from './types';
import {
  button,
  connectionInformation,
  controlRow,
  controls,
  grid,
  ledInner,
  ledScreen,
  ledWrapper,
  wrapper,
} from './app.css';

export function App() {
  const [rotation, setRotation] = useState<number>(0);
  const [triggerClear, setTriggerClear] = useState(false);
  const [isMouseDown, setMouseIsDown] = useState(false);
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

              setIndexMatrix(
                rotateArrayByDegree(
                  [...new Array(256)].map((_, i) => i),
                  json.rotation
                )
              );
              setLeds(rotateArrayByDegree(json.data, json.rotation));

              break;
          }
        } catch {}
      },
    }
  );

  const connectionStatus = {
    [ReadyState.CONNECTING]: 'Connecting',
    [ReadyState.OPEN]: 'Open',
    [ReadyState.CLOSING]: 'Closing',
    [ReadyState.CLOSED]: 'Try to reconnect',
    [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
  }[readyState];

  const setLed = (index: number) => {
    const rotatedIndex = indexMatrix[index];

    setLeds((state: number[]) => {
      const status = !state[rotatedIndex];
      wsMessage('led', {
        index: rotatedIndex,
        status,
      });
      return state.map((led, i) => (i === index ? Number(status) : led));
    });
  };

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

  if (readyState !== ReadyState.OPEN) {
    return (
      <div className={wrapper}>
        <div className={connectionInformation}>{connectionStatus}...</div>
      </div>
    );
  }

  return (
    <div className={wrapper}>
      <div>
        <div className={`${ledScreen} ${mode !== MODE.NONE ? 'disabled' : ''}`}>
          <div
            className={grid}
            onPointerUp={() => {
              setMouseIsDown(false);
            }}
            onPointerLeave={() => {
              setMouseIsDown(false);
            }}
          >
            {leds.map((led, ledIndex) => (
              <div
                key={ledIndex}
                className={ledWrapper}
                onPointerDown={() => {
                  setLed(ledIndex);
                  setMouseIsDown(true);
                }}
                onPointerEnter={() => {
                  if (isMouseDown) {
                    setLed(ledIndex);
                  }
                }}
              >
                <div className={`${ledInner} ${led ? 'active' : ''}`}></div>
              </div>
            ))}
          </div>
        </div>
        <div className={controls}>
          <div className={controlRow}>
            <button className={button} onClick={() => sendMode(MODE.STARS)}>
              stars
            </button>
            <button className={button} onClick={() => sendMode(MODE.LINES)}>
              lines
            </button>
            <button className={button} onClick={() => sendMode(MODE.BREAKOUT)}>
              breakout
            </button>
            <button
              className={button}
              onClick={() => sendMode(MODE.GAMEOFLIFE)}
            >
              game of life
            </button>
            <button className={button} onClick={() => sendMode(MODE.CIRCLE)}>
              circle
            </button>
            <button className={button} onClick={() => sendMode(MODE.CLOCK)}>
              clock
            </button>

            <button
              className={button}
              onClick={() => wsMessage('persist-mode')}
            >
              set mode as default
            </button>
          </div>

          <div className={controlRow}>
            <button className={button} onClick={() => rotate(false)}>
              rotate left
            </button>
            <button className={button} onClick={() => rotate(true)}>
              rotate right
            </button>
          </div>

          <div className={controlRow}>
            {mode === MODE.NONE ? (
              <>
                <button className={button} onClick={() => loadImage()}>
                  load image
                </button>
                <button className={button} onClick={clear}>
                  clear
                </button>
                <button className={button} onClick={() => wsMessage('persist')}>
                  persist
                </button>
                <button className={button} onClick={() => wsMessage('load')}>
                  load
                </button>
              </>
            ) : (
              <button className={button} onClick={() => sendMode(MODE.NONE)}>
                draw mode
              </button>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

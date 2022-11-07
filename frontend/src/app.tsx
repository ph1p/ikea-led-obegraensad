import { useMemo, useState } from 'preact/hooks';
import useWebSocket, { ReadyState } from 'react-use-websocket';
import './app.scss';

enum MODE {
  NONE = '',
  STARS = 'stars',
  LINES = 'lines',
  BREAKOUT = 'breakout',
  GAMEOFLIFE = 'gameoflife',
  UPDATE = 'update',
  LOADING = 'loading',
}

export function App() {
  const [triggerClear, setTriggerClear] = useState(false);
  const [isMouseDown, setMouseIsDown] = useState(false);
  const [removeLed, setRemoveLed] = useState(false);
  const [activeLeds, setActiveLeds] = useState<Record<number, number>>({});
  const [mode, setMode] = useState<MODE>(MODE.NONE);
  const { sendMessage, readyState } = useWebSocket(
    `${
      (import.meta as any).PROD
        ? location.href.replace('http', 'ws')
        : import.meta.env.VITE_WS_URL
    }ws`,
    {
      shouldReconnect: () => true,
      onMessage: (event) => {
        try {
          const json = JSON.parse(event.data);

          switch (json.event) {
            case 'mode':
              setMode(Object.values(MODE)[json.mode as number]);
              break;
            case 'info':
              setMode(Object.values(MODE)[json.mode as number]);

              const active: Record<number, number> = {};
              json.data.forEach((v: number, i: number) => (active[i] = v));
              setActiveLeds(active);

              break;
          }
        } catch {}
      },
    }
  );

  const connectionStatus = useMemo(
    () =>
      ({
        [ReadyState.CONNECTING]: 'Connecting',
        [ReadyState.OPEN]: 'Open',
        [ReadyState.CLOSING]: 'Closing',
        [ReadyState.CLOSED]: 'Closed',
        [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
      }[readyState]),
    [readyState]
  );

  const setLed = (index: number) => {
    const status = removeLed ? 0 : 1;
    setActiveLeds((state: any) => ({
      ...state,
      [index]: status,
    }));
    sendMessage(
      JSON.stringify({
        event: 'led',
        index: index,
        status: status,
      })
    );
  };

  const loadImage = () => {
    const wh = 16;
    const fp = document.createElement('input');
    fp.type = 'file';
    fp.click();
    fp.onchange = (e: Event) => {
      if (!e.target) return;
      const file = (e.target as HTMLInputElement).files?.[0];
      const reader = new FileReader();
      if (file) {
        reader.readAsDataURL(file);
      }
      reader.onload = () => {
        const uimg = new Image() as any;
        uimg.src = reader.result;
        uimg.width = wh;
        uimg.height = wh;
        uimg.onload = () => {
          const pxc = document.createElement('canvas');
          const pxctx = pxc.getContext('2d');
          pxc.width = wh;
          pxc.height = wh;
          pxctx?.drawImage(uimg, 0, 0, wh, wh);
          const imgData = pxctx?.getImageData(0, 0, wh, wh);
          if (imgData) {
            let index = 0;
            const data = [];
            const active: Record<number, number> = {};
            for (var i = 0; i < imgData.data.length; i += 4) {
              const isActive =
                imgData.data[i] + imgData.data[i + 1] + imgData.data[i + 2] <=
                383
                  ? 1
                  : 0;
              data.push(isActive);
              active[index] = isActive;

              index++;
            }

            setActiveLeds(active);

            sendMessage(
              JSON.stringify({
                event: 'screen',
                data,
              })
            );
          }
        };
      };
    };
  };

  const clear = () => {
    setActiveLeds({});
    setRemoveLed(false);
    setTriggerClear(!triggerClear);
    sendMessage(
      JSON.stringify({
        event: 'clear',
      })
    );
  };

  const sendMode = (mode: MODE) => {
    sendMessage(
      JSON.stringify({
        event: 'mode',
        mode,
      })
    );
  };

  const persist = () => {
    sendMessage(
      JSON.stringify({
        event: 'persist',
      })
    );
  };

  if (readyState !== ReadyState.OPEN) {
    return (
      <div className="wrapper">
        <div className="connection-information">{connectionStatus}...</div>
      </div>
    );
  }

  return (
    <div className="wrapper">
      <div>
        <div className={`box ${mode !== MODE.NONE ? 'disabled' : ''}`}>
          <div
            className="grid"
            onPointerUp={() => {
              setMouseIsDown(false);
            }}
            onPointerLeave={() => {
              setMouseIsDown(false);
            }}
          >
            {[...new Array(256)].map((_, k) => (
              <div
                key={k}
                className="led"
                onPointerDown={() => {
                  setLed(k);
                  setMouseIsDown(true);
                }}
                onClick={() => setLed(k)}
                onPointerOver={() => {
                  if (isMouseDown) {
                    setLed(k);
                  }
                }}
              >
                <div className={`inner ${activeLeds[k] ? 'active' : ''}`}></div>
              </div>
            ))}
          </div>
          <div className="cable">
            <span></span>
            <span></span>
            <span></span>
          </div>
        </div>
        <div className="controls">
          <div>
            <button onClick={() => sendMode(MODE.STARS)}>stars</button>
            <button onClick={() => sendMode(MODE.LINES)}>lines</button>
            <button onClick={() => sendMode(MODE.BREAKOUT)}>breakout</button>
            <button onClick={() => sendMode(MODE.GAMEOFLIFE)}>
              game of life
            </button>
          </div>

          <div>
            {mode === MODE.NONE ? (
              <>
                <button onClick={() => loadImage()}>load image</button>
                <button onClick={clear}>clear</button>
                <button onClick={persist}>persist</button>
                <button onClick={() => setRemoveLed(!removeLed)}>
                  {removeLed ? 'pencil' : 'eraser'}
                </button>
              </>
            ) : (
              <button onClick={() => sendMode(MODE.NONE)}>draw mode</button>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

import createWebsocket from '@solid-primitives/websocket';
import {
  createContext,
  createSignal,
  onCleanup,
  onMount,
  ParentComponent,
  Show,
  useContext,
} from 'solid-js';
import { toastNotificationStyle } from './index.css';
import { MODE, Store } from './types';

const StoreContext = createContext<Store>();

export const StoreProvider: ParentComponent = (props) => {
  const [toastNotification, setToastNotification] = createSignal<{
    text: string;
    duration: number;
  } | null>(null);
  const [rotation, setRotation] = createSignal(0);
  const [indexMatrix, setIndexMatrix] = createSignal(
    [...new Array(256)].map((_, i) => i)
  );
  const [leds, setLeds] = createSignal([...new Array(256)].fill(0));
  const [mode, setMode] = createSignal<MODE>(MODE.NONE);

  const [connect, disconnect, send, connectionState] = createWebsocket(
    `${
      import.meta.env.PROD
        ? `ws://${window.location.host}/`
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

  onMount(() => connect());
  onCleanup(() => disconnect());

  const connectionStatus = {
    0: 'Connecting',
    1: 'Open',
    2: 'Closing',
    3: 'Try to reconnect',
  }[connectionState()];

  const toast = (text: string, duration: number) => {
    setToastNotification({
      text,
      duration,
    });
    setTimeout(() => setToastNotification(null), duration);
  };

  const store = {
    rotation,
    indexMatrix,
    leds,
    mode,
    setRotation,
    setIndexMatrix,
    setLeds,
    setMode,
    send,
    connectionState,
    connectionStatus,
    toast,
  };

  return (
    <StoreContext.Provider value={store}>
      <Show when={!!toastNotification()}>
        <div class={toastNotificationStyle}>{toastNotification()?.text}</div>
      </Show>
      {props.children}
    </StoreContext.Provider>
  );
};

export const useStore = () => useContext(StoreContext);

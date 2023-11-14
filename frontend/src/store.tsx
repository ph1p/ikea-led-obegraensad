import { createEventSignal } from '@solid-primitives/event-listener';
import {
  createReconnectingWS,
  createWSState,
} from '@solid-primitives/websocket';
import {
  ParentComponent,
  Show,
  batch,
  createContext,
  createEffect,
  createSignal,
  useContext,
} from 'solid-js';
import { toastNotificationStyle } from './index.css';
import { SYSTEM_STATUS, Store } from './types';

const StoreContext = createContext<Store>();

const ws = createReconnectingWS(
  `${
    import.meta.env.PROD
      ? `ws://${window.location.host}/`
      : import.meta.env.VITE_WS_URL
  }ws`
);

export const StoreProvider: ParentComponent = (props) => {
  const [toastNotification, setToastNotification] = createSignal<{
    text: string;
    duration: number;
  } | null>(null);
  const [rotation, setRotation] = createSignal(0);
  const [plugins, setPlugins] = createSignal([]);
  const [plugin, setPlugin] = createSignal(1);
  const [brightness, setBrightness] = createSignal(0);
  const [indexMatrix, setIndexMatrix] = createSignal(
    [...new Array(256)].map((_, i) => i)
  );
  const [leds, setLeds] = createSignal([...new Array(256)].fill(0));
  const [systemStatus, setSystemStatus] = createSignal<SYSTEM_STATUS>(
    SYSTEM_STATUS.NONE
  );

  const state = createWSState(ws);

  const connectionStatus = [
    'Connecting',
    'Connected',
    'Disconnecting',
    'Disconnected',
  ];

  const messageEvent = createEventSignal<{ message: MessageEvent }>(
    ws,
    'message'
  );

  createEffect(() => {
    const json = JSON.parse(messageEvent()?.data || '{}');

    switch (json.event) {
      case 'info':
        batch(() => {
          setSystemStatus(Object.values(SYSTEM_STATUS)[json.status as number]);
          setRotation(json.rotation);
          setBrightness(json.brightness);

          if (!plugins().length) {
            setPlugins(json.plugins);
          }

          if (json.plugin) {
            setPlugin(json.plugin as number);

            toast('Mode changed', 1000);
          }

          if (plugin() === 1) {
            setIndexMatrix([...new Array(256)].map((_, i) => i));
          }

          if (json.data) {
            setLeds(json.data);
          }
        });

        break;
    }
  });

  const toast = (text: string, duration: number) => {
    setToastNotification({
      text,
      duration,
    });
    setTimeout(() => setToastNotification(null), duration);
  };

  return (
    <StoreContext.Provider
      value={{
        rotation,
        brightness,
        indexMatrix,
        leds,
        plugins,
        plugin,
        setRotation,
        setBrightness,
        setIndexMatrix,
        setLeds,
        setPlugin,
        systemStatus,
        setSystemStatus,
        send: ws.send,
        connectionState: state,
        connectionStatus: connectionStatus[state()],
        toast,
      }}
    >
      <Show when={!!toastNotification()}>
        <div class={toastNotificationStyle}>{toastNotification()?.text}</div>
      </Show>
      {props.children}
    </StoreContext.Provider>
  );
};

export const useStore = () => useContext(StoreContext);

import { createEventSignal } from '@solid-primitives/event-listener';
import {
  createReconnectingWS,
  createWSState,
} from '@solid-primitives/websocket';
import {
  batch,
  createContext,
  createEffect,
  ParentComponent,
  Show,
  useContext,
} from 'solid-js';
import { createStore } from 'solid-js/store';
import { ScheduleItem, Store, StoreActions, SYSTEM_STATUS } from '../types';
import { ToastProvider } from './toast';

const ws = createReconnectingWS(
  `${
    import.meta.env.PROD
      ? `ws://${window.location.host}/`
      : import.meta.env.VITE_WS_URL
  }ws`,
);
const wsState = createWSState(ws);

const connectionStatus = [
  'Connecting',
  'Connected',
  'Disconnecting',
  'Disconnected',
];

const [mainStore, setStore] = createStore<Store>({
  get isActiveScheduler() {
    return this.schedule?.length > 0;
  },
  rotation: 0,
  plugins: [],
  plugin: 1,
  brightness: 0,
  indexMatrix: [...new Array(256)].map((_, i) => i),
  leds: [...new Array(256)].fill(0),
  systemStatus: SYSTEM_STATUS.NONE,
  connectionState: wsState,
  connectionStatus: connectionStatus[0],
  schedule: [],
});

const actions: StoreActions = {
  setRotation: (rotation: number) => setStore('rotation', rotation),
  setPlugins: (plugins: []) => setStore('plugins', plugins),
  setPlugin: (plugin: number) => setStore('plugin', plugin),
  setBrightness: (brightness: number) => setStore('brightness', brightness),
  setIndexMatrix: (indexMatrix: number[]) =>
    setStore('indexMatrix', indexMatrix),
  setLeds: (leds: number[]) => setStore('leds', leds),
  setSystemStatus: (systemStatus: SYSTEM_STATUS) =>
    setStore('systemStatus', systemStatus),
  setSchedule: (items: ScheduleItem[]) => setStore('schedule', items),
  send: ws.send,
};

const store: [Store, StoreActions] = [mainStore, actions] as const;

const StoreContext = createContext<[Store, StoreActions]>(store);

export const StoreProvider: ParentComponent = (props) => {
  const messageEvent = createEventSignal<{ message: MessageEvent }>(
    ws,
    'message',
  );

  createEffect(() => {
    const json = JSON.parse(messageEvent()?.data || '{}');

    switch (json.event) {
      case 'minimal-info':
        actions.setSystemStatus(
          Object.values(SYSTEM_STATUS)[json.status as number],
        );
        actions.setRotation(json.rotation);
        actions.setBrightness(json.brightness);
        actions.setPlugin(json.plugin as number);
        break;
      case 'info':
        batch(() => {
          actions.setSystemStatus(
            Object.values(SYSTEM_STATUS)[json.status as number],
          );
          actions.setRotation(json.rotation);
          actions.setBrightness(json.brightness);

          if (json.schedule) {
            actions.setSchedule(json.schedule);
          }

          if (!mainStore.plugins.length) {
            actions.setPlugins(json.plugins);
          }

          if (json.plugin) {
            actions.setPlugin(json.plugin as number);
          }

          if (mainStore.plugin === 1) {
            actions.setIndexMatrix([...new Array(256)].map((_, i) => i));
          }

          if (json.data) {
            actions.setLeds(json.data);
          }
        });
        break;
    }
  });

  return (
    <ToastProvider>
      <StoreContext.Provider value={store}>
        {props.children}
      </StoreContext.Provider>
    </ToastProvider>
  );
};

export const useStore = () => {
  if (StoreContext) return useContext(StoreContext);

  return [{}, {}] as [Store, StoreActions];
};

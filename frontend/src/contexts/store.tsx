import { createEventSignal } from "@solid-primitives/event-listener";
import { createReconnectingWS, createWSState } from "@solid-primitives/websocket";
import { batch, createContext, createEffect, type JSX, untrack, useContext } from "solid-js";
import { createStore } from "solid-js/store";

import {
  type BrightnessSchedule,
  type ScheduleItem,
  type Store,
  type StoreActions,
  SYSTEM_STATUS,
} from "../types";
import { ToastProvider } from "./toast";

const ws = createReconnectingWS(
  `${
    import.meta.env.PROD
      ? `${window.location.protocol === "https:" ? "wss:" : "ws:"}//${window.location.host}/`
      : import.meta.env.VITE_WS_URL
  }ws`,
);
const LIVE_PREVIEW_STORAGE_KEY = "livePreview";

const readLivePreview = () => {
  try {
    return localStorage.getItem(LIVE_PREVIEW_STORAGE_KEY) !== "false";
  } catch {
    return true;
  }
};

const sendLivePreview = (enabled: boolean) =>
  ws.send(JSON.stringify({ event: "live-preview", enabled }));

ws.addEventListener("open", (event) => {
  (event.currentTarget as WebSocket).binaryType = "arraybuffer";
  // the device subscribes every new connection, so only opting out is needed
  if (!mainStore.livePreview) {
    sendLivePreview(false);
  }
});

const wsState = createWSState(ws);

const connectionStatus = ["Connecting", "Connected", "Disconnecting", "Disconnected"];

const [mainStore, setStore] = createStore<Store>({
  isActiveScheduler: false,
  rotation: 0,
  plugins: [],
  plugin: 1,
  baseBrightness: 0,
  brightness: 0,
  brightnessSchedule: {
    enabled: false,
    startTime: "22:00",
    endTime: "07:00",
    brightness: 64,
    active: false,
  },
  artnetUniverse: 1,
  GOLDelay: 150,
  indexMatrix: [...new Array(256)].map((_, i) => i),
  leds: [...new Array(256)].fill(0),
  systemStatus: SYSTEM_STATUS.NONE,
  connectionState: wsState,
  connectionStatus: connectionStatus[0],
  schedule: [],
  livePreview: readLivePreview(),
});

const actions: StoreActions = {
  setIsActiveScheduler: (isActive) => setStore("isActiveScheduler", isActive),
  setRotation: (rotation) => setStore("rotation", rotation),
  setPlugins: (plugins) => setStore("plugins", plugins),
  setPlugin: (plugin) => setStore("plugin", plugin),
  setBaseBrightness: (brightness) => setStore("baseBrightness", brightness),
  setBrightness: (brightness) => setStore("brightness", brightness),
  setBrightnessSchedule: (schedule) => setStore("brightnessSchedule", schedule),
  setArtnetUniverse: (artnetUniverse) => setStore("artnetUniverse", artnetUniverse),
  setGOLDelay: (GOLDelay) => setStore("GOLDelay", GOLDelay),
  setIndexMatrix: (indexMatrix) => setStore("indexMatrix", indexMatrix),
  setLeds: (leds) => setStore("leds", leds),
  setSystemStatus: (systemStatus: SYSTEM_STATUS) => setStore("systemStatus", systemStatus),
  setSchedule: (items: ScheduleItem[]) => setStore("schedule", items),
  setLivePreview: (enabled) => {
    setStore("livePreview", enabled);
    try {
      localStorage.setItem(LIVE_PREVIEW_STORAGE_KEY, String(enabled));
    } catch {
      // preference is only kept for this session
    }
    sendLivePreview(enabled);
  },
  send: ws.send,
};

const store: [Store, StoreActions] = [mainStore, actions] as const;

const StoreContext = createContext<[Store, StoreActions]>(store);

const isValidNumber = (value: unknown): value is number =>
  typeof value === "number" && !Number.isNaN(value);

const isValidBoolean = (value: unknown): value is boolean => typeof value === "boolean";

const isValidArray = (value: unknown): value is unknown[] => Array.isArray(value);

const isBrightnessSchedule = (value: unknown): value is BrightnessSchedule => {
  if (!value || typeof value !== "object") return false;
  const schedule = value as Record<string, unknown>;
  return (
    isValidBoolean(schedule.enabled) &&
    typeof schedule.startTime === "string" &&
    typeof schedule.endTime === "string" &&
    isValidNumber(schedule.brightness) &&
    isValidBoolean(schedule.active)
  );
};

export const StoreProvider = (props?: { value?: Store; children?: JSX.Element }) => {
  const messageEvent = createEventSignal<{ message: MessageEvent }>(ws, "message");
  const errorEvent = createEventSignal<{ error: Event }>(ws, "error");

  createEffect(() => {
    const state = wsState();

    if (state >= 0 && state < connectionStatus.length) {
      setStore("connectionStatus", connectionStatus[state]);
    }

    if (state === WebSocket.CLOSED || state === WebSocket.CLOSING) {
      console.warn("WebSocket disconnected. Will attempt to reconnect...");
    }
  });

  createEffect(() => {
    const error = errorEvent();
    if (error) {
      console.error("WebSocket error occurred:", error);
    }
  });

  createEffect(() => {
    try {
      const data = messageEvent()?.data;
      if (data instanceof ArrayBuffer) {
        if (data.byteLength === 256) {
          const frame = new Uint8Array(data);
          // the device can resend an identical frame, skip it so nothing redraws
          const leds = untrack(() => mainStore.leds);
          if (!frame.every((value, i) => value === leds[i])) {
            actions.setLeds(Array.from(frame));
          }
        }
        return;
      }

      const json = JSON.parse(data || "{}");

      if (!json.event || typeof json.event !== "string") {
        return;
      }

      switch (json.event) {
        case "info":
          batch(() => {
            if (
              isValidNumber(json.status) &&
              json.status >= 0 &&
              json.status < Object.values(SYSTEM_STATUS).length
            ) {
              actions.setSystemStatus(Object.values(SYSTEM_STATUS)[json.status]);
            }

            if (isValidNumber(json.rotation)) {
              actions.setRotation(json.rotation);
            }

            if (isValidNumber(json.baseBrightness)) {
              actions.setBaseBrightness(json.baseBrightness);
            }

            if (isValidNumber(json.brightness)) {
              actions.setBrightness(json.brightness);
            }

            if (isBrightnessSchedule(json.brightnessSchedule)) {
              actions.setBrightnessSchedule(json.brightnessSchedule);
            }

            if (isValidBoolean(json.scheduleActive)) {
              actions.setIsActiveScheduler(json.scheduleActive);
            }

            if (isValidArray(json.schedule)) {
              actions.setSchedule(json.schedule as ScheduleItem[]);
            }

            if (!mainStore.plugins.length && isValidArray(json.plugins)) {
              actions.setPlugins(json.plugins);
            }

            if (isValidNumber(json.plugin)) {
              actions.setPlugin(json.plugin);
            }

            if (mainStore.plugin === 1) {
              actions.setIndexMatrix([...new Array(256)].map((_, i) => i));
            }

            if (isValidArray(json.data)) {
              actions.setLeds(json.data as number[]);
            }
          });
          break;
      }
    } catch (error) {
      console.error("Failed to parse WebSocket message:", error);
    }
  });

  return (
    <ToastProvider>
      <StoreContext.Provider value={store}>{props?.children}</StoreContext.Provider>
    </ToastProvider>
  );
};

export const useStore = () => {
  if (StoreContext) return useContext(StoreContext);

  return [{}, {}] as [Store, StoreActions];
};

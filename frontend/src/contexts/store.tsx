import { createEventSignal } from "@solid-primitives/event-listener";
import { createReconnectingWS, createWSState } from "@solid-primitives/websocket";
import { batch, createContext, createEffect, type JSX, useContext } from "solid-js";
import { createStore } from "solid-js/store";

import { type ScheduleItem, type Store, type StoreActions, SYSTEM_STATUS } from "../types";
import { ToastProvider } from "./toast";

const ws = createReconnectingWS(
  `${import.meta.env.PROD ? `ws://${window.location.host}/` : import.meta.env.VITE_WS_URL}ws`,
);

const wsState = createWSState(ws);

const connectionStatus = ["Connecting", "Connected", "Disconnecting", "Disconnected"];

const [mainStore, setStore] = createStore<Store>({
  isActiveScheduler: false,
  rotation: 0,
  plugins: [],
  plugin: 1,
  brightness: 0,
  artnetUniverse: 1,
  GOLDelay: 150,
  indexMatrix: [...new Array(256)].map((_, i) => i),
  leds: [...new Array(256)].fill(0),
  systemStatus: SYSTEM_STATUS.NONE,
  connectionState: wsState,
  connectionStatus: connectionStatus[0],
  schedule: [],
});

const actions: StoreActions = {
  setIsActiveScheduler: (isActive) => setStore("isActiveScheduler", isActive),
  setRotation: (rotation) => setStore("rotation", rotation),
  setPlugins: (plugins) => setStore("plugins", plugins),
  setPlugin: (plugin) => setStore("plugin", plugin),
  setBrightness: (brightness) => setStore("brightness", brightness),
  setArtnetUniverse: (artnetUniverse) => setStore("artnetUniverse", artnetUniverse),
  setGOLDelay: (GOLDelay) => setStore("GOLDelay", GOLDelay),
  setIndexMatrix: (indexMatrix) => setStore("indexMatrix", indexMatrix),
  setLeds: (leds) => setStore("leds", leds),
  setSystemStatus: (systemStatus: SYSTEM_STATUS) => setStore("systemStatus", systemStatus),
  setSchedule: (items: ScheduleItem[]) => setStore("schedule", items),
  send: ws.send,
};

const store: [Store, StoreActions] = [mainStore, actions] as const;

const StoreContext = createContext<[Store, StoreActions]>(store);

// WebSocket message validation helpers
const isValidNumber = (value: unknown): value is number =>
  typeof value === "number" && !Number.isNaN(value);

const isValidBoolean = (value: unknown): value is boolean => typeof value === "boolean";

const isValidArray = (value: unknown): value is unknown[] => Array.isArray(value);

export const StoreProvider = (props?: { value?: Store; children?: JSX.Element }) => {
  const messageEvent = createEventSignal<{ message: MessageEvent }>(ws, "message");

  createEffect(() => {
    try {
      const json = JSON.parse(messageEvent()?.data || "{}");

      // Validate event type
      if (!json.event || typeof json.event !== "string") {
        return;
      }

      switch (json.event) {
        case "info":
          batch(() => {
            // Validate and set system status
            if (
              isValidNumber(json.status) &&
              json.status >= 0 &&
              json.status < Object.values(SYSTEM_STATUS).length
            ) {
              actions.setSystemStatus(Object.values(SYSTEM_STATUS)[json.status]);
            }

            // Validate and set rotation
            if (isValidNumber(json.rotation)) {
              actions.setRotation(json.rotation);
            }

            // Validate and set brightness
            if (isValidNumber(json.brightness)) {
              actions.setBrightness(json.brightness);
            }

            // Validate and set scheduler active status
            if (isValidBoolean(json.scheduleActive)) {
              actions.setIsActiveScheduler(json.scheduleActive);
            }

            // Validate and set schedule
            if (isValidArray(json.schedule)) {
              actions.setSchedule(json.schedule as ScheduleItem[]);
            }

            // Validate and set plugins (only if not already loaded)
            if (!mainStore.plugins.length && isValidArray(json.plugins)) {
              actions.setPlugins(json.plugins);
            }

            // Validate and set current plugin
            if (isValidNumber(json.plugin)) {
              actions.setPlugin(json.plugin);
            }

            // Reset index matrix for plugin 1
            if (mainStore.plugin === 1) {
              actions.setIndexMatrix([...new Array(256)].map((_, i) => i));
            }

            // Validate and set LED data
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

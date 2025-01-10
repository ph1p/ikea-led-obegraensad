import { Component, createMemo, ParentProps, Show } from "solid-js";
import { Layout } from "./components/layout";
import { LedMatrix } from "./components/led-matrix";
import { loadImageAndGetDataArray, rotateArray } from "./helpers";
import { useStore } from "./store";
import { ScreenInfo } from "./components/screen-info";

export const App: Component = () => {
  const store = useStore();

  const rotatedMatrix = createMemo(() =>
    rotateArray(store!.indexMatrix(), store!.rotation()),
  );

  const wsMessage = (
    event:
      | "persist"
      | "load"
      | "clear"
      | "plugin"
      | "screen"
      | "led"
      | "persist-plugin"
      | "brightness",
    data?: any,
  ) =>
    store?.send(
      JSON.stringify({
        event,
        ...data,
      }),
    );

  const handleRotate = (turnRight = false) => {
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
        event: "rotate",
        direction: turnRight ? "right" : "left",
      }),
    );
  };

  const handleLoadImage = () => {
    loadImageAndGetDataArray((data) => {
      store?.setLeds(() =>
        store?.indexMatrix().map((index) => (data[index] ? 255 : 0)),
      );
      wsMessage("screen", { data });
    });
  };

  const handleClear = () => {
    store?.setLeds([...new Array(256).fill(0)]);
    wsMessage("clear");
    store?.toast(`Canvas cleared`, 1000);
  };

  const handlePersist = () => {
    wsMessage("persist");
    store?.toast(`Saved current state`, 1500);
  };

  const handleLoad = () => {
    wsMessage("load");
    store?.toast(`Saved state loaded`, 1500);
  };

  const handlePluginChange = (pluginId: number) => {
    wsMessage("plugin", { plugin: pluginId });
    store?.toast("Mode changed", 1000);
  };

  const handleBrightnessChange = (value: number) => {
    store?.setBrightness(value);
    wsMessage("brightness", { brightness: value });
  };

  const handlePersistPlugin = () => {
    wsMessage("persist-plugin");
    store?.toast(`Current mode set as default`, 1500);
  };

  const renderLedMatrix = () => (
    <div class="grid p-8 h-full justify-center items-center sm:p-4 sm:m-0">
      <Show
        when={store?.plugin() === 1}
        fallback={
          <ScreenInfo>
            <h2 class="text-4xl">A Plugin currently running</h2>
            <p class="text-xl mt-2 text-gray-300">
              Select "Draw" to show the canvas.
            </p>
          </ScreenInfo>
        }
      >
        <div
          style={{
            opacity: (store?.brightness() || 255) / 255,
          }}
        >
          <LedMatrix
            disabled={store?.plugin() !== 1}
            data={store?.leds || (() => [])}
            indexData={rotatedMatrix}
            onSetLed={(data) => {
              wsMessage("led", data);
            }}
            onSetMatrix={(data) => {
              store?.setLeds([...data]);
            }}
          />
        </div>
      </Show>
    </div>
  );

  return (
    <Show
      when={store?.connectionState() === 1}
      fallback={
        <Layout
          content={
            <ScreenInfo>
              <h2 class="text-4xl">{store?.connectionStatus}...</h2>
            </ScreenInfo>
          }
          sidebarContent={
            <div class="bg-white p-6">
              <a href="#/creator" class="text-gray-700 hover:text-gray-900">
                Creator
              </a>
            </div>
          }
        />
      }
    >
      <Layout
        content={renderLedMatrix()}
        store={store}
        onRotate={handleRotate}
        onLoadImage={handleLoadImage}
        onClear={handleClear}
        onPersist={handlePersist}
        onLoad={handleLoad}
        onPluginChange={handlePluginChange}
        onBrightnessChange={handleBrightnessChange}
        onPersistPlugin={handlePersistPlugin}
      />
    </Show>
  );
};

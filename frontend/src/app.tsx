import { type Component, createEffect, createMemo, createSignal, Show } from "solid-js";

import { Layout } from "./components/layout/layout";
import Sidebar from "./components/layout/sidebar";
import { LedMatrix } from "./components/led-matrix";
import { useStore } from "./contexts/store";
import { useToast } from "./contexts/toast";
import { loadImageAndGetDataArray, rotateArray } from "./helpers";

export const App: Component = () => {
  const [store, actions] = useStore();
  const { toast } = useToast();

  const rotatedMatrix = createMemo(() => rotateArray(store.indexMatrix, store.rotation));
  const activePlugin = createMemo(() => store.plugins.find((plugin) => plugin.id === store.plugin));
  const isDrawPlugin = createMemo(() => activePlugin()?.name === "Draw");
  const canConfigure = createMemo(() => isDrawPlugin() && !store.isActiveScheduler);
  const [activeTab, setActiveTab] = createSignal<"configuration" | "preview">("preview");

  createEffect(() => {
    setActiveTab(canConfigure() ? "configuration" : "preview");
  });

  const wsMessage = (
    event:
      | "persist"
      | "load"
      | "clear"
      | "plugin"
      | "screen"
      | "led"
      | "persist-plugin"
      | "artnet"
      | "brightness"
      | "goldelay",
    data?: Record<string, string | number> | { data: number[] },
  ) =>
    actions.send(
      JSON.stringify({
        event,
        ...data,
      }),
    );

  const handleRotate = (turnRight = false) => {
    const currentRotation = store.rotation || 0;
    actions.setRotation((currentRotation + (turnRight ? 1 : -1) + 4) % 4);
    actions.send(
      JSON.stringify({
        event: "rotate",
        direction: turnRight ? "right" : "left",
      }),
    );
  };

  const handleLoadImage = () => {
    loadImageAndGetDataArray((data) => {
      actions.setLeds(store.indexMatrix.map((index) => (data[index] ? 255 : 0)));
      wsMessage("screen", { data });
    });
  };

  const handleClear = () => {
    actions?.setLeds([...new Array(256).fill(0)]);
    wsMessage("clear");
    toast(`Canvas cleared`, 1000);
  };

  const handlePersist = () => {
    wsMessage("persist");
    toast(`Saved current state`, 1500);
  };

  const handleLoad = () => {
    wsMessage("load");
    toast(`Saved state loaded`, 1500);
  };

  const handlePluginChange = (pluginId: number) => {
    wsMessage("plugin", { plugin: pluginId });
    toast("Mode changed", 1000);
  };

  const handleBrightnessChange = (value: number, shouldSend = false) => {
    actions?.setBrightness(value);
    if (shouldSend) {
      wsMessage("brightness", { brightness: value });
    }
  };

  const handleArtnetUniverseChange = (value: number, shouldSend = false) => {
    actions?.setArtnetUniverse(value);
    if (shouldSend) {
      wsMessage("artnet", { universe: value });
    }
  };

  const handleGOLDelayChange = (value: number, shouldSend = false) => {
    actions?.setGOLDelay(value);
    if (shouldSend) {
      wsMessage("goldelay", { delay: value });
    }
  };

  const handlePersistPlugin = () => {
    wsMessage("persist-plugin");
    toast(`Current mode set as default`, 1500);
  };

  const renderMatrix = (disabled: boolean) => (
    <LedMatrix
      disabled={disabled}
      data={store.leds || []}
      indexData={rotatedMatrix()}
      brightness={store.brightness ?? 255}
      onSetLed={(data) => {
        wsMessage("led", data);
      }}
      onSetMatrix={(data) => {
        actions?.setLeds([...data]);
      }}
    />
  );

  const renderDrawControls = () => (
    <div class="lg:hidden w-full max-w-100 sm:max-w-125">
      <div class="grid grid-cols-4 gap-2">
        <button
          type="button"
          onClick={handleLoadImage}
          class="flex flex-col items-center justify-center gap-1 bg-gray-700 text-white border-0 p-2 cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded text-xs"
        >
          <i class="fa-solid fa-file-import text-base" />
          <span>Import</span>
        </button>
        <button
          type="button"
          onClick={handleClear}
          class="flex flex-col items-center justify-center gap-1 bg-gray-700 text-white border-0 p-2 cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded hover:bg-red-600 text-xs"
        >
          <i class="fa-solid fa-trash text-base" />
          <span>Clear</span>
        </button>
        <button
          type="button"
          onClick={handlePersist}
          class="flex flex-col items-center justify-center gap-1 bg-gray-700 text-white border-0 p-2 cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded text-xs"
        >
          <i class="fa-solid fa-floppy-disk text-base" />
          <span>Save</span>
        </button>
        <button
          type="button"
          onClick={handleLoad}
          class="flex flex-col items-center justify-center gap-1 bg-gray-700 text-white border-0 p-2 cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded text-xs"
        >
          <i class="fa-solid fa-refresh text-base" />
          <span>Load</span>
        </button>
      </div>
    </div>
  );

  const renderTabNavigation = () => (
    <div class="flex rounded bg-gray-800 p-1 text-sm">
      <Show when={canConfigure()}>
        <button
          type="button"
          class={`rounded px-3 py-1.5 ${
            activeTab() === "configuration"
              ? "bg-gray-600 text-white"
              : "text-gray-300 hover:text-white"
          }`}
          onClick={() => setActiveTab("configuration")}
        >
          Configuration
        </button>
      </Show>
      <button
        type="button"
        class={`rounded px-3 py-1.5 ${
          activeTab() === "preview"
            ? "bg-gray-600 text-white"
            : "text-gray-300 hover:text-white"
        }`}
        onClick={() => setActiveTab("preview")}
      >
        Live preview
      </button>
    </div>
  );

  const renderLivePreview = () => <div class="flex flex-col items-center">{renderMatrix(true)}</div>;

  const renderDrawConfiguration = () => (
    <div class="flex flex-col items-center gap-6">
      {renderMatrix(false)}
      {renderDrawControls()}
    </div>
  );

  const renderTabContent = () => (
    <Show
      when={canConfigure() && activeTab() === "configuration"}
      fallback={renderLivePreview()}
    >
      {renderDrawConfiguration()}
    </Show>
  );

  const renderContentTabs = () => (
    <div class="grid p-8 h-full justify-center items-center sm:p-4 sm:m-0">
      <div class="w-full flex flex-col items-center gap-4">
        {renderTabNavigation()}
        {renderTabContent()}
      </div>
    </div>
  );

  return (
    <Layout
      content={renderContentTabs()}
      sidebar={
        <Sidebar
          onRotate={handleRotate}
          onLoadImage={handleLoadImage}
          onClear={handleClear}
          onPersist={handlePersist}
          onLoad={handleLoad}
          onPluginChange={handlePluginChange}
          onBrightnessChange={handleBrightnessChange}
          onArtnetChange={handleArtnetUniverseChange}
          onGOLDelayChange={handleGOLDelayChange}
          onPersistPlugin={handlePersistPlugin}
        />
      }
    />
  );
};

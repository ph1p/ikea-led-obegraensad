import { type Component, createEffect, createMemo, createSignal, For, Show } from "solid-js";

import { Layout } from "./components/layout/layout";
import Sidebar from "./components/layout/sidebar";
import { LedMatrix } from "./components/led-matrix";
import { useStore } from "./contexts/store";
import { useToast } from "./contexts/toast";
import { loadImageAndGetDataArray, rotateArray } from "./helpers";
import type { BrightnessSchedule } from "./types";

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
      | "brightness-schedule"
      | "goldelay",
    data?: Record<string, string | number | boolean> | { data: number[] },
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
    actions?.setBaseBrightness(value);
    if (shouldSend) {
      wsMessage("brightness", { brightness: value });
    }
  };

  const handleBrightnessScheduleChange = (schedule: BrightnessSchedule, shouldSend = true) => {
    actions.setBrightnessSchedule(schedule);
    if (shouldSend) {
      wsMessage("brightness-schedule", schedule);
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
      brightness={store.brightness ?? store.baseBrightness ?? 255}
      onSetLed={(data) => {
        wsMessage("led", data);
      }}
      onSetMatrix={(data) => {
        actions?.setLeds([...data]);
      }}
    />
  );

  const drawActions = [
    { label: "Import", icon: "fa-file-import", onClick: () => handleLoadImage() },
    { label: "Clear", icon: "fa-trash", onClick: () => handleClear(), danger: true },
    { label: "Save", icon: "fa-floppy-disk", onClick: () => handlePersist() },
    { label: "Load", icon: "fa-rotate", onClick: () => handleLoad() },
  ];

  const renderDrawControls = () => (
    <div class="grid w-full grid-cols-4 gap-2">
      <For each={drawActions}>
        {(action) => (
          <button
            type="button"
            onClick={action.onClick}
            class={`btn flex-col gap-1 py-3 text-xs ${action.danger ? "btn-danger" : ""}`}
          >
            <i class={`fa-solid ${action.icon} text-base`} />
            {action.label}
          </button>
        )}
      </For>
    </div>
  );

  const renderHeader = () => (
    <div class="flex w-full flex-wrap items-center justify-between gap-3">
      <div class="flex min-w-0 items-center gap-2.5 rounded-full border border-line bg-surface/80 py-1.5 pr-4 pl-3 text-sm font-medium backdrop-blur">
        <span class="relative flex size-2 shrink-0">
          <span class="absolute inline-flex size-full animate-ping rounded-full bg-emerald-400 opacity-75" />
          <span class="relative inline-flex size-2 rounded-full bg-emerald-400" />
        </span>
        <span class="truncate">
          {store.isActiveScheduler ? "Scheduler running" : (activePlugin()?.name ?? "—")}
        </span>
      </div>
      <Show when={canConfigure()}>
        <div class="segmented" role="group" aria-label="View">
          <button
            type="button"
            aria-pressed={activeTab() === "configuration"}
            onClick={() => setActiveTab("configuration")}
          >
            <i class="fa-solid fa-pen mr-1.5 text-xs" />
            Draw
          </button>
          <button
            type="button"
            aria-pressed={activeTab() === "preview"}
            onClick={() => setActiveTab("preview")}
          >
            <i class="fa-solid fa-eye mr-1.5 text-xs" />
            Live
          </button>
        </div>
      </Show>
    </div>
  );

  const renderLivePreview = () => (
    <>
      <div class="relative">
        {renderMatrix(true)}
        <Show when={!store.livePreview}>
          <div class="absolute inset-0 flex flex-col items-center justify-center gap-2 rounded-3xl bg-black/70 text-sm font-medium text-white/70 backdrop-blur-sm">
            <i class="fa-solid fa-eye-slash text-xl" />
            Live preview paused
          </div>
        </Show>
      </div>
      <button
        type="button"
        aria-pressed={store.livePreview}
        onClick={() => actions.setLivePreview(!store.livePreview)}
        class="btn"
      >
        <i class={`fa-solid ${store.livePreview ? "fa-pause" : "fa-play"}`} />
        {store.livePreview ? "Pause live preview" : "Resume live preview"}
      </button>
    </>
  );

  const renderDrawConfiguration = () => (
    <>
      {renderMatrix(false)}
      {renderDrawControls()}
    </>
  );

  const renderContent = () => (
    <div class="flex min-h-full items-center justify-center p-4 lg:p-8">
      <div class="flex w-[min(100%,700px,(100dvh-16rem)*9/13)] flex-col items-center gap-5 max-lg:w-[min(100%,55dvh*9/13)]">
        {renderHeader()}
        <Show
          when={canConfigure() && activeTab() === "configuration"}
          fallback={renderLivePreview()}
        >
          {renderDrawConfiguration()}
        </Show>
      </div>
    </div>
  );

  return (
    <Layout
      content={renderContent()}
      sidebar={
        <Sidebar
          onRotate={handleRotate}
          onPluginChange={handlePluginChange}
          onBrightnessChange={handleBrightnessChange}
          onBrightnessScheduleChange={handleBrightnessScheduleChange}
          onArtnetChange={handleArtnetUniverseChange}
          onGOLDelayChange={handleGOLDelayChange}
          onPersistPlugin={handlePersistPlugin}
        />
      }
    />
  );
};

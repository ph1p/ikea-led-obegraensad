import { type Component, For, type JSX, Show } from "solid-js";

import { useStore } from "../../contexts/store";
import { ToggleScheduleButton } from "../../scheduler";

interface SidebarSectionProps {
  title: string;
  children: JSX.Element;
}

const SidebarSection: Component<SidebarSectionProps> = (props) => (
  <div class="space-y-3">
    <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">{props.title}</h3>
    <div class="space-y-2">{props.children}</div>
  </div>
);

interface SidebarProps {
  onRotate: (turnRight: boolean) => void;
  onLoadImage: () => void;
  onClear: () => void;
  onPersist: () => void;
  onLoad: () => void;
  onPluginChange: (pluginId: number) => void;
  onBrightnessChange: (value: number, shouldSend?: boolean) => void;
  onArtnetChange: (value: number, shouldSend?: boolean) => void;
  onPersistPlugin: () => void;
  onGOLDelayChange: (value: number, shouldSend?: boolean) => void;
}

export const Sidebar: Component<SidebarProps> = (props) => {
  const [store] = useStore();

  return (
    <>
      <div class="flex-1 min-h-0 overflow-y-auto">
        <Show
          when={!store?.isActiveScheduler}
          fallback={
            <Show when={store.schedule.length > 0}>
              <ToggleScheduleButton />
            </Show>
          }
        >
          <SidebarSection title="Display Mode">
            <div class="flex flex-col gap-2.5">
              <select
                class="flex-1 px-2.5 py-2.5 bg-gray-50 border border-gray-200 rounded"
                onChange={(e) => props.onPluginChange(parseInt(e.currentTarget.value, 10))}
                value={store?.plugin}
              >
                <For each={store?.plugins}>
                  {(plugin) => <option value={plugin.id}>{plugin.name}</option>}
                </For>
              </select>
              <button
                type="button"
                onClick={props.onPersistPlugin}
                class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded"
              >
                Set as Default
              </button>
            </div>
          </SidebarSection>
        </Show>

        <div class="my-6 border-t border-gray-200" />

        <SidebarSection title={`Rotation (${[0, 90, 180, 270][store?.rotation || 0]}°)`}>
          <div class="flex gap-2.5">
            <button
              type="button"
              onClick={() => props.onRotate(false)}
              class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded flex items-center justify-center gap-2"
            >
              <i class="fa-solid fa-rotate-left" />
              <span class="hidden xl:inline">Left</span>
            </button>
            <button
              type="button"
              onClick={() => props.onRotate(true)}
              class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded flex items-center justify-center gap-2"
            >
              <i class="fa-solid fa-rotate-right" />
              <span class="hidden xl:inline">Right</span>
            </button>
          </div>
        </SidebarSection>

        <div class="my-6 border-t border-gray-200" />

        <SidebarSection title="Brightness">
          <div class="space-y-2">
            <input
              type="range"
              min="0"
              max="255"
              value={store?.brightness}
              class="w-full"
              onInput={(e) => props.onBrightnessChange(parseInt(e.currentTarget.value, 10))}
              onPointerUp={(e) =>
                props.onBrightnessChange(parseInt(e.currentTarget.value, 10), true)
              }
            />
            <div class="text-sm text-gray-600 text-right">
              {Math.round(((store?.brightness ?? 255) / 255) * 100)}%
            </div>
          </div>
        </SidebarSection>

        <Show when={store?.plugin === 17 && !store?.isActiveScheduler}>
          <div class="my-6 border-t border-gray-200" />

          <SidebarSection title="ArtNet Universe">
            <div class="space-y-2">
              <input
                type="range"
                min="0"
                max="255"
                value={store?.artnetUniverse}
                class="w-full"
                onInput={(e) => props.onArtnetChange(parseInt(e.currentTarget.value, 10))}
                onPointerUp={(e) => props.onArtnetChange(parseInt(e.currentTarget.value, 10), true)}
              />
              <div class="text-sm text-gray-600 text-right">{store?.artnetUniverse}</div>
            </div>
          </SidebarSection>
        </Show>

        <Show when={store?.plugin === 4 && !store?.isActiveScheduler}>
          <div class="my-6 border-t border-gray-200" />

          <SidebarSection title="Time Step Delay">
            <div class="space-y-2">
              <input
                type="range"
                min="1"
                max="4000"
                value={store?.GOLDelay}
                class="w-full"
                onInput={(e) => props.onGOLDelayChange(parseInt(e.currentTarget.value, 10))}
                onPointerUp={(e) =>
                  props.onGOLDelayChange(parseInt(e.currentTarget.value, 10), true)
                }
              />
              <div class="text-sm text-gray-600 text-right">{store?.GOLDelay}</div>
            </div>
          </SidebarSection>
        </Show>

        <Show when={store?.plugin === 1 && !store?.isActiveScheduler}>
          <div class="my-6 border-t border-gray-200 hidden lg:block" />

          <div class="hidden lg:block">
            <SidebarSection title="Matrix Controls">
              <div class="grid grid-cols-2 gap-2">
                <button
                  type="button"
                  onClick={props.onLoadImage}
                  class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded flex flex-col items-center gap-1"
                >
                  <i class="fa-solid fa-file-import text-base" />
                  <span class="text-xs">Import</span>
                </button>
                <button
                  type="button"
                  onClick={props.onClear}
                  class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded hover:bg-red-600 flex flex-col items-center gap-1"
                >
                  <i class="fa-solid fa-trash text-base" />
                  <span class="text-xs">Clear</span>
                </button>
                <button
                  type="button"
                  onClick={props.onPersist}
                  class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded flex flex-col items-center gap-1"
                >
                  <i class="fa-solid fa-floppy-disk text-base" />
                  <span class="text-xs">Save</span>
                </button>
                <button
                  type="button"
                  onClick={props.onLoad}
                  class="w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded flex flex-col items-center gap-1"
                >
                  <i class="fa-solid fa-refresh text-base" />
                  <span class="text-xs">Load</span>
                </button>
              </div>
            </SidebarSection>
          </div>
        </Show>
      </div>

      <div class="flex flex-col shrink-0 pt-6 border-t border-gray-200 space-y-6">
        <Show when={store?.plugins.some((p) => p.name.includes("Animation"))}>
          <a
            href="#/creator"
            class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
          >
            <i class="fa-solid fa-pencil mr-2" />
            Animation Creator
          </a>
        </Show>

        <a href="#/scheduler" class=" items-center text-gray-700 hover:text-gray-900 font-medium">
          <i class="fa-regular fa-clock fa- mr-2" />
          Plugin Scheduler ({store.schedule.length})
        </a>

        <a
          href="/update"
          class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
        >
          <i class="fa-solid fa-download mr-2" />
          Firmware Update
        </a>
      </div>
    </>
  );
};

export default Sidebar;

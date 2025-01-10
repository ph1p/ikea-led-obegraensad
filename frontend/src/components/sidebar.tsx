import { Component, For, Show, JSX } from "solid-js";
import { Store } from "../types";
import { Tooltip } from "./tooltip";

interface SidebarSectionProps {
  title: string;
  children: JSX.Element;
}

const SidebarSection: Component<SidebarSectionProps> = (props) => (
  <div class="space-y-3">
    <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">
      {props.title}
    </h3>
    <div class="space-y-2">{props.children}</div>
  </div>
);

interface SidebarProps {
  store: Store;
  onRotate: (turnRight: boolean) => void;
  onLoadImage: () => void;
  onClear: () => void;
  onPersist: () => void;
  onLoad: () => void;
  onPluginChange: (pluginId: number) => void;
  onBrightnessChange: (value: number) => void;
  onPersistPlugin: () => void;
}

export const Sidebar: Component<SidebarProps> = (props) => {
  return (
    <aside class="bg-white p-6 flex flex-col h-full">
      <SidebarSection title="Display Mode">
        <div class="flex flex-col gap-2.5">
          <select
            class="flex-1 px-2.5 py-2.5 bg-gray-50 border border-gray-200 rounded"
            onChange={(e) =>
              props.onPluginChange(parseInt(e.currentTarget.value))
            }
            value={props.store.plugin()}
          >
            <For each={props.store.plugins()}>
              {(plugin) => <option value={plugin.id}>{plugin.name}</option>}
            </For>
          </select>
          <Tooltip text="Save current display mode as default startup mode">
            <button
              onClick={props.onPersistPlugin}
              class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
            >
              Set Default
            </button>
          </Tooltip>
        </div>
      </SidebarSection>

      <div class="my-6 border-t border-gray-200" />

      <SidebarSection title={`Rotation (${[0, 90, 180, 270][props.store.rotation()]}°)`}>
        <div class="flex gap-2.5">
          <Tooltip text="Rotate display counter-clockwise">
            <button
              onClick={() => props.onRotate(false)}
              class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
            >
              <i class="fa-solid fa-rotate-left" />
            </button>
          </Tooltip>
          <Tooltip text="Rotate display clockwise">
            <button
              onClick={() => props.onRotate(true)}
              class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
            >
              <i class="fa-solid fa-rotate-right" />
            </button>
          </Tooltip>
        </div>
      </SidebarSection>

      <div class="my-6 border-t border-gray-200" />

      <SidebarSection title="Brightness">
        <div class="space-y-2">
          <input
            type="range"
            min="0"
            max="255"
            value={props.store.brightness()}
            class="w-full"
            onInput={(e) =>
              props.onBrightnessChange(parseInt(e.currentTarget.value))
            }
          />
          <div class="text-sm text-gray-600 text-right">
            {Math.round((props.store.brightness() / 255) * 100)}%
          </div>
        </div>
      </SidebarSection>

      <Show when={props.store.plugin() === 1}>
        <div class="my-6 border-t border-gray-200" />

        <SidebarSection title="Matrix Controls">
          <div class="grid grid-cols-2 gap-2">
            <Tooltip text="Import image from your device">
              <button
                onClick={props.onLoadImage}
                class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
              >
                <i class="fa-solid fa-file-import" />
              </button>
            </Tooltip>
            <Tooltip text="Clear all pixels">
              <button
                onClick={props.onClear}
                class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded hover:bg-red-600"
              >
                <i class="fa-solid fa-trash" />
              </button>
            </Tooltip>
            <Tooltip text="Save current display state">
              <button
                onClick={props.onPersist}
                class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
              >
                <i class="fa-solid fa-floppy-disk" />
              </button>
            </Tooltip>
            <Tooltip text="Load last saved state">
              <button
                onClick={props.onLoad}
                class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
              >
                <i class="fa-solid fa-refresh" />
              </button>
            </Tooltip>
          </div>
        </SidebarSection>
      </Show>

      <div class="mt-auto pt-6 border-t border-gray-200">
        <Tooltip text="Create and edit animations">
          <a
            href="#/creator"
            class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
          >
            <i class="fa-solid fa-pencil mr-2" />
            Animation Creator
          </a>
        </Tooltip>
      </div>
    </aside>
  );
};

export default Sidebar;

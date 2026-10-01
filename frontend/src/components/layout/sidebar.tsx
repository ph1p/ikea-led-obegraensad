import { type Component, createSignal, For, type JSX, Show } from "solid-js";

import { useStore } from "../../contexts/store";
import { ToggleScheduleButton } from "../../scheduler";
import type { BrightnessSchedule } from "../../types";

interface SidebarSectionProps {
  title: string;
  aside?: JSX.Element;
  children: JSX.Element;
}

export const SidebarSection: Component<SidebarSectionProps> = (props) => (
  <section class="space-y-3">
    <div class="flex items-center justify-between">
      <h3 class="section-title">{props.title}</h3>
      <Show when={props.aside}>
        <span class="text-xs font-medium tabular-nums text-fg/80">{props.aside}</span>
      </Show>
    </div>
    {props.children}
  </section>
);

interface SidebarProps {
  onRotate: (turnRight: boolean) => void;
  onPluginChange: (pluginId: number) => void;
  onBrightnessChange: (value: number, shouldSend?: boolean) => void;
  onBrightnessScheduleChange: (schedule: BrightnessSchedule, shouldSend?: boolean) => void;
  onArtnetChange: (value: number, shouldSend?: boolean) => void;
  onPersistPlugin: () => void;
  onGOLDelayChange: (value: number, shouldSend?: boolean) => void;
}

const toPercent = (value: number) => `${Math.round((value / 255) * 100)}%`;

export const Sidebar: Component<SidebarProps> = (props) => {
  const [store] = useStore();
  const [scheduleError, setScheduleError] = createSignal<string>();
  const updateBrightnessSchedule = (changes: Partial<BrightnessSchedule>, shouldSend = true) => {
    const next = { ...store.brightnessSchedule, ...changes };
    if (!next.startTime || !next.endTime) {
      setScheduleError("Start and end times are required.");
      return;
    }
    if (next.startTime === next.endTime) {
      setScheduleError("Start and end times must differ.");
      return;
    }
    setScheduleError();
    props.onBrightnessScheduleChange(next, shouldSend);
  };

  return (
    <div class="space-y-7">
      <Show
        when={!store?.isActiveScheduler}
        fallback={
          <Show when={store.schedule.length > 0}>
            <SidebarSection title="Scheduler">
              <p class="text-sm text-muted">The scheduler is cycling through plugins.</p>
              <ToggleScheduleButton />
            </SidebarSection>
          </Show>
        }
      >
        <SidebarSection title="Display Mode">
          <select
            class="input"
            onChange={(e) => props.onPluginChange(parseInt(e.currentTarget.value, 10))}
            value={store?.plugin}
          >
            <For each={store?.plugins}>
              {(plugin) => <option value={plugin.id}>{plugin.name}</option>}
            </For>
          </select>
          <button type="button" onClick={props.onPersistPlugin} class="btn w-full">
            <i class="fa-regular fa-star" />
            Set as default
          </button>
        </SidebarSection>
      </Show>

      <SidebarSection title="Rotation" aside={`${[0, 90, 180, 270][store?.rotation || 0]}°`}>
        <div class="grid grid-cols-2 gap-2">
          <button type="button" onClick={() => props.onRotate(false)} class="btn">
            <i class="fa-solid fa-rotate-left" />
            Left
          </button>
          <button type="button" onClick={() => props.onRotate(true)} class="btn">
            <i class="fa-solid fa-rotate-right" />
            Right
          </button>
        </div>
      </SidebarSection>

      <SidebarSection title="Brightness" aside={toPercent(store?.baseBrightness ?? 255)}>
        <div class="flex items-center gap-3 text-muted">
          <i class="fa-regular fa-sun text-xs" />
          <input
            type="range"
            min="0"
            max="255"
            aria-label="Base brightness"
            value={store?.baseBrightness}
            onInput={(e) => props.onBrightnessChange(parseInt(e.currentTarget.value, 10))}
            onPointerUp={(e) => props.onBrightnessChange(parseInt(e.currentTarget.value, 10), true)}
          />
          <i class="fa-solid fa-sun" />
        </div>
      </SidebarSection>

      <SidebarSection
        title="Night Mode"
        aside={
          store.brightnessSchedule.enabled
            ? store.brightnessSchedule.active
              ? "Active"
              : "Waiting"
            : undefined
        }
      >
        <label class="flex cursor-pointer items-center justify-between gap-3 text-sm">
          <span>Scheduled brightness</span>
          <input
            type="checkbox"
            class="switch"
            checked={store.brightnessSchedule.enabled}
            onChange={(e) => updateBrightnessSchedule({ enabled: e.currentTarget.checked })}
          />
        </label>

        <Show when={store.brightnessSchedule.enabled}>
          <div class="space-y-3 rounded-xl border border-line bg-canvas/40 p-3">
            <div class="grid grid-cols-2 gap-2">
              <label class="space-y-1 text-xs text-muted">
                <span>From</span>
                <input
                  type="time"
                  value={store.brightnessSchedule.startTime}
                  class="input py-2"
                  onChange={(e) => updateBrightnessSchedule({ startTime: e.currentTarget.value })}
                />
              </label>
              <label class="space-y-1 text-xs text-muted">
                <span>To</span>
                <input
                  type="time"
                  value={store.brightnessSchedule.endTime}
                  class="input py-2"
                  onChange={(e) => updateBrightnessSchedule({ endTime: e.currentTarget.value })}
                />
              </label>
            </div>
            <div class="space-y-2">
              <div class="flex justify-between text-xs text-muted">
                <label for="scheduled-brightness">Brightness</label>
                <span class="tabular-nums text-fg/80">
                  {toPercent(store.brightnessSchedule.brightness)}
                </span>
              </div>
              <input
                id="scheduled-brightness"
                type="range"
                min="0"
                max="255"
                value={store.brightnessSchedule.brightness}
                onInput={(e) =>
                  updateBrightnessSchedule(
                    { brightness: parseInt(e.currentTarget.value, 10) },
                    false,
                  )
                }
                onPointerUp={(e) =>
                  updateBrightnessSchedule({ brightness: parseInt(e.currentTarget.value, 10) })
                }
              />
            </div>
            <Show when={scheduleError()}>
              <div class="text-xs text-danger">{scheduleError()}</div>
            </Show>
          </div>
        </Show>
      </SidebarSection>

      <Show when={store?.plugin === 17 && !store?.isActiveScheduler}>
        <SidebarSection title="ArtNet Universe" aside={store?.artnetUniverse}>
          <input
            type="range"
            min="0"
            max="255"
            aria-label="ArtNet universe"
            value={store?.artnetUniverse}
            onInput={(e) => props.onArtnetChange(parseInt(e.currentTarget.value, 10))}
            onPointerUp={(e) => props.onArtnetChange(parseInt(e.currentTarget.value, 10), true)}
          />
        </SidebarSection>
      </Show>

      <Show when={store?.plugin === 4 && !store?.isActiveScheduler}>
        <SidebarSection title="Time Step Delay" aside={`${store?.GOLDelay} ms`}>
          <input
            type="range"
            min="1"
            max="4000"
            aria-label="Time step delay"
            value={store?.GOLDelay}
            onInput={(e) => props.onGOLDelayChange(parseInt(e.currentTarget.value, 10))}
            onPointerUp={(e) => props.onGOLDelayChange(parseInt(e.currentTarget.value, 10), true)}
          />
        </SidebarSection>
      </Show>
    </div>
  );
};

export default Sidebar;

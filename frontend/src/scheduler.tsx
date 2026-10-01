import { type Component, For, Index, Show } from "solid-js";

import { Layout } from "./components/layout/layout";
import { useStore } from "./contexts/store";
import { useToast } from "./contexts/toast";

const API_URL = import.meta.env.PROD
  ? `http://${window.location.host}/`
  : import.meta.env.VITE_BASE_URL;

export const ResetScheduleButton = () => {
  const { toast } = useToast();

  return (
    <button
      type="button"
      onClick={async () => {
        try {
          const response = await fetch(`${API_URL}api/schedule/clear`);

          if (response.ok) {
            toast("Reset schedule successfully", 2000);
          }
        } catch {
          toast("Failed to reset schedule", 2000);
        }
      }}
      class="btn btn-danger w-full"
    >
      <i class="fa-solid fa-rotate-left" />
      Reset Scheduler
    </button>
  );
};

export const ToggleScheduleButton = () => {
  const [store] = useStore();
  const { toast } = useToast();

  return (
    <button
      type="button"
      onClick={async () => {
        if (store.isActiveScheduler) {
          try {
            const response = await fetch(`${API_URL}api/schedule/stop`);

            if (response.ok) {
              toast("Stopped schedule successfully", 2000);
            }
          } catch {
            toast("Failed to stop schedule", 2000);
          }
        } else {
          if (store.schedule.length === 0) {
            toast("Cannot start empty schedule", 2000);
            return;
          }

          try {
            const response = await fetch(`${API_URL}api/schedule`, {
              method: "POST",
              headers: {
                "Content-Type": "application/x-www-form-urlencoded",
              },
              body: `schedule=${JSON.stringify(store.schedule)}`,
            });

            if (response.ok) {
              toast("Schedule started successfully", 2000);
            }
          } catch (error) {
            console.error("Failed to start schedule:", error);
            toast("Failed to start schedule", 2000);
          }
        }
      }}
      class={`btn w-full ${store.isActiveScheduler ? "btn-danger" : "btn-accent"}`}
    >
      <i class={`fa-solid ${store.isActiveScheduler ? "fa-stop" : "fa-play"}`} />
      {store.isActiveScheduler ? "Stop Scheduler" : "Start Scheduler"}
    </button>
  );
};

const Scheduler: Component = () => {
  const [store, actions] = useStore();
  const { toast } = useToast();

  const handleAddItem = () => {
    const defaultPluginId = store.plugins.length > 0 ? store.plugins[0].id : 1;
    actions.setSchedule([...store.schedule, { pluginId: defaultPluginId, duration: 1 }]);
  };

  const handleRemoveItem = (index: number) => {
    actions.setSchedule(store.schedule.filter((_, i) => i !== index));
  };

  const handlePluginChange = (index: number, pluginId: number) => {
    actions.setSchedule(
      store.schedule.map((item, i) => (i === index ? { ...item, pluginId } : item)),
    );
  };

  const handleDurationChange = (index: number, duration: number) => {
    actions.setSchedule(
      store.schedule.map((item, i) => (i === index ? { ...item, duration } : item)),
    );
  };

  const handleToggleScheduler = async () => {
    if (store.isActiveScheduler) {
      try {
        const response = await fetch(`${API_URL}api/schedule/stop`);

        if (response.ok) {
          toast("Stopped schedule successfully", 2000);
        }
      } catch {
        toast("Failed to stop schedule", 2000);
      }
    } else {
      if (store.schedule.length === 0) {
        toast("Cannot start empty schedule", 2000);
        return;
      }

      try {
        const response = await fetch(`${API_URL}api/schedule`, {
          method: "POST",
          headers: {
            "Content-Type": "application/x-www-form-urlencoded",
          },
          body: `schedule=${JSON.stringify(store.schedule)}`,
        });

        if (response.ok) {
          toast("Schedule started successfully", 2000);
        }
      } catch (error) {
        console.error("Failed to start schedule:", error);
        toast("Failed to start schedule", 2000);
      }
    }
  };

  const pluginName = (id: number) =>
    store.plugins?.find((p) => p.id === id)?.name ?? "Unknown Plugin";
  const totalSeconds = () => store.schedule.reduce((sum, item) => sum + (item.duration || 0), 0);
  const formatDuration = (seconds: number) =>
    seconds >= 3600
      ? `${Math.floor(seconds / 3600)}h ${Math.round((seconds % 3600) / 60)}m`
      : seconds >= 60
        ? `${Math.floor(seconds / 60)}m ${seconds % 60}s`
        : `${seconds}s`;

  return (
    <Layout
      content={
        <div class="mx-auto w-full max-w-3xl space-y-5 p-4 lg:p-8">
          <div class="flex flex-wrap items-end justify-between gap-3">
            <div>
              <h1 class="text-2xl font-semibold tracking-tight">Scheduler</h1>
              <p class="mt-1 text-sm text-muted">
                Cycle through plugins automatically.
                <Show when={store.schedule.length > 0}>
                  {" "}
                  {store.schedule.length} {store.schedule.length === 1 ? "step" : "steps"} ·{" "}
                  {formatDuration(totalSeconds())} loop
                </Show>
              </p>
            </div>
            <Show when={store.isActiveScheduler}>
              <span class="inline-flex items-center gap-2 rounded-full border border-emerald-400/30 bg-emerald-400/10 px-3 py-1 text-xs font-medium text-success">
                <span class="size-1.5 animate-pulse rounded-full bg-emerald-400" />
                Running
              </span>
            </Show>
          </div>

          <Show
            when={store.schedule?.length > 0}
            fallback={
              <div class="card px-6 py-14 text-center">
                <div class="mx-auto mb-4 grid size-12 place-items-center rounded-2xl bg-raised text-lg text-accent ring-1 ring-line">
                  <i class="fa-solid fa-clock" />
                </div>
                <h2 class="mb-1 font-semibold">No schedule yet</h2>
                <p class="mb-6 text-sm text-muted">Add plugins and set how long each one runs.</p>
                <button type="button" onClick={handleAddItem} class="btn btn-accent">
                  <i class="fa-solid fa-plus" />
                  Add first plugin
                </button>
              </div>
            }
          >
            <ol class="space-y-2">
              <Index each={store.schedule}>
                {(item, index) => {
                  const isCurrent = () =>
                    store.isActiveScheduler && store.plugin === item().pluginId;
                  return (
                    <li
                      class={`card flex flex-col gap-3 p-3 transition sm:flex-row sm:items-center ${
                        isCurrent() ? "border-accent/60 ring-1 ring-accent/30" : ""
                      }`}
                    >
                      <div class="flex min-w-0 flex-1 items-center gap-3">
                        <span
                          class={`grid size-8 shrink-0 place-items-center rounded-lg text-xs font-semibold tabular-nums ${
                            isCurrent() ? "bg-accent text-accent-fg" : "bg-raised text-muted"
                          }`}
                        >
                          {index + 1}
                        </span>
                        <Show
                          when={!store.isActiveScheduler}
                          fallback={
                            <span class="truncate text-sm font-medium">
                              {pluginName(item().pluginId)}
                            </span>
                          }
                        >
                          <select
                            aria-label={`Plugin for step ${index + 1}`}
                            value={item().pluginId}
                            onChange={(e) =>
                              handlePluginChange(index, parseInt(e.currentTarget.value, 10))
                            }
                            class="input"
                          >
                            <For each={store.plugins}>
                              {(plugin) => <option value={plugin.id}>{plugin.name}</option>}
                            </For>
                          </select>
                        </Show>
                      </div>
                      <Show
                        when={!store.isActiveScheduler}
                        fallback={
                          <span class="pl-11 text-sm tabular-nums text-muted sm:pl-0">
                            {formatDuration(item().duration)}
                          </span>
                        }
                      >
                        <div class="flex items-center gap-2 pl-11 sm:pl-0">
                          <div class="relative flex-1 sm:w-36 sm:flex-initial">
                            <input
                              type="number"
                              min="1"
                              max="86400"
                              aria-label={`Duration for step ${index + 1} in seconds`}
                              value={item().duration}
                              onInput={(e) =>
                                handleDurationChange(index, parseInt(e.currentTarget.value, 10))
                              }
                              class="input pr-12 tabular-nums"
                            />
                            <span class="pointer-events-none absolute top-1/2 right-3 -translate-y-1/2 text-xs text-muted">
                              sec
                            </span>
                          </div>
                          <button
                            type="button"
                            onClick={() => handleRemoveItem(index)}
                            class="btn btn-ghost btn-danger btn-icon"
                            aria-label={`Remove step ${index + 1}`}
                          >
                            <i class="fa-solid fa-trash" />
                          </button>
                        </div>
                      </Show>
                    </li>
                  );
                }}
              </Index>
            </ol>

            <div class="flex flex-col gap-2 sm:flex-row">
              <Show when={!store.isActiveScheduler}>
                <button type="button" onClick={handleAddItem} class="btn flex-1">
                  <i class="fa-solid fa-plus" />
                  Add plugin
                </button>
              </Show>
              <button
                type="button"
                onClick={handleToggleScheduler}
                class={`btn flex-1 ${store.isActiveScheduler ? "btn-danger" : "btn-accent"}`}
              >
                <i class={`fa-solid ${store.isActiveScheduler ? "fa-stop" : "fa-play"}`} />
                {store.isActiveScheduler ? "Stop scheduler" : "Start scheduler"}
              </button>
            </div>
          </Show>
        </div>
      }
      sidebar={
        <div class="space-y-3">
          <h3 class="section-title">About</h3>
          <p class="text-sm text-muted">
            The schedule runs on the device and keeps going when this page is closed.
          </p>
          <Show when={store.isActiveScheduler}>
            <ResetScheduleButton />
          </Show>
        </div>
      }
    />
  );
};

export default Scheduler;

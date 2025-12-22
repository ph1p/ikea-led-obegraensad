import { type Component, For, Index, Show } from "solid-js";

import Button from "./components/button";
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
      class="w-full bg-gray-600 hover:bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded"
    >
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
      class="w-full bg-gray-600 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded"
    >
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

  return (
    <Layout
      content={
        <div class="space-y-3 p-5 pb-24 lg:pb-5">
          <h3 class="text-4xl text-white tracking-wide">Scheduler</h3>

          <div class="bg-white p-4 lg:p-6 rounded-md">
            <div class="space-y-2">
              <Show
                when={store.schedule?.length > 0}
                fallback={
                  <div class="text-center py-8">
                    <div class="text-md text-gray-500 italic mb-4">No schedule set</div>
                    <button
                      type="button"
                      onClick={handleAddItem}
                      class="bg-gray-700 hover:bg-gray-800 text-white px-4 py-2 rounded-lg transition-all duration-200 font-semibold text-sm"
                    >
                      <i class="fa-solid fa-plus mr-2" />
                      Add First Plugin
                    </button>
                  </div>
                }
              >
                <Index each={store.schedule}>
                  {(item, index) => (
                    <div
                      class={`flex flex-col lg:flex-row items-stretch lg:items-center gap-3 lg:gap-4 p-3 lg:p-4 rounded-lg shadow-sm border hover:border-gray-200 transition-all duration-200 ${
                        store.plugin === item().pluginId
                          ? "bg-green-300 border-green-500"
                          : "bg-white border-gray-100"
                      }`}
                    >
                      <Show
                        when={store.isActiveScheduler}
                        fallback={
                          <>
                            <div class="flex-1">
                              <select
                                value={item().pluginId}
                                onChange={(e) =>
                                  handlePluginChange(index, parseInt(e.currentTarget.value, 10))
                                }
                                class="w-full px-3 py-3 lg:py-2 text-base bg-gray-50 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 outline-none transition-all duration-200 disabled:border-0"
                              >
                                <For each={store.plugins}>
                                  {(plugin) => <option value={plugin.id}>{plugin.name}</option>}
                                </For>
                              </select>
                            </div>
                            <div class="flex items-center gap-3">
                              <div class="relative flex-1 lg:flex-initial">
                                <input
                                  type="number"
                                  min="1"
                                  max="86400"
                                  value={item().duration}
                                  onInput={(e) =>
                                    handleDurationChange(index, parseInt(e.currentTarget.value, 10))
                                  }
                                  class="w-full lg:w-32 pr-20 pl-3 py-3 lg:py-2 text-base bg-gray-50 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 outline-none transition-all duration-200 disabled:border-0"
                                />
                                <span class="absolute right-3 top-1/2 -translate-y-1/2 text-gray-400 text-sm">
                                  {item().duration > 1 ? "seconds" : "second"}
                                </span>
                              </div>

                              <button
                                type="button"
                                onClick={() => handleRemoveItem(index)}
                                class="p-3 lg:p-2 text-gray-500 hover:text-red-500 hover:bg-red-50 rounded-lg transition-all duration-200 min-w-[44px] min-h-[44px] lg:min-w-0 lg:min-h-0"
                                aria-label="Remove item"
                              >
                                <i class="fas fa-trash-alt text-lg" />
                              </button>
                            </div>
                          </>
                        }
                      >
                        <div class="w-full">
                          {store.plugins?.find((p) => p.id === item().pluginId)?.name ??
                            "Unknown Plugin"}
                        </div>
                        <div class="flex items-center gap-3">
                          <div class="relative">
                            {item().duration}
                            <span class="ml-2 text-sm text-gray-500">
                              {item().duration > 1 ? "seconds" : "second"}
                            </span>
                          </div>
                        </div>
                      </Show>
                    </div>
                  )}
                </Index>

                <div class="mt-4 flex flex-col sm:flex-row gap-2">
                  <Show when={store.schedule.length > 0}>
                    <button
                      type="button"
                      onClick={handleToggleScheduler}
                      class={`flex-1 px-3 py-2 rounded-lg transition-all duration-200 font-semibold text-sm flex items-center justify-center gap-2 ${
                        store.isActiveScheduler
                          ? "bg-red-600 hover:bg-red-700 text-white"
                          : "bg-green-600 hover:bg-green-700 text-white"
                      }`}
                    >
                      <i class={`fa-solid ${store.isActiveScheduler ? "fa-stop" : "fa-play"}`} />
                      <span>{store.isActiveScheduler ? "Stop Scheduler" : "Start Scheduler"}</span>
                    </button>
                  </Show>
                  <Show when={!store.isActiveScheduler}>
                    <button
                      type="button"
                      onClick={handleAddItem}
                      class="flex-1 bg-gray-700 hover:bg-gray-800 text-white px-3 py-2 rounded-lg transition-all duration-200 font-semibold text-sm flex items-center justify-center gap-2"
                    >
                      <i class="fa-solid fa-plus" />
                      <span>Add Plugin</span>
                    </button>
                  </Show>
                </div>
              </Show>
            </div>
          </div>
        </div>
      }
      sidebar={
        <>
          <div class="space-y-6">
            <div class="space-y-3">
              <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">Controls</h3>

              <div class="flex flex-col gap-2">
                <Button onClick={handleAddItem} class="hover:bg-gray-700 transition-colors">
                  <i class="fa-solid fa-plus mr-2" />
                  Add Plugin
                </Button>

                <Show when={store.schedule.length > 0}>
                  <div class="my-6 border-t border-gray-200" />
                  <ToggleScheduleButton />
                </Show>

                <Show when={store.isActiveScheduler}>
                  <div class="my-6 border-t border-gray-200" />
                  <ResetScheduleButton />
                </Show>
              </div>
            </div>
          </div>
          <div class="mt-auto pt-6 border-t border-gray-200">
            <a
              href="#/"
              class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
            >
              <i class="fa-solid fa-arrow-left mr-2" />
              Back to Main
            </a>
          </div>
        </>
      }
    />
  );
};

export default Scheduler;

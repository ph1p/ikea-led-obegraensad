import { Component, createSignal, For, Show } from 'solid-js';
import Button from './components/button';
import { Layout } from './components/layout/layout';
import { Tooltip } from './components/tooltip';
import { useStore } from './contexts/store';
import { useToast } from './contexts/toast';

export const ResetScheduleButton = () => {
  const { toast } = useToast();
  return (
    <Tooltip text="Save current display mode as default startup mode">
      <button
        onClick={async () => {
          try {
            const response = await fetch(
              `${
                import.meta.env.PROD
                  ? `ws://${window.location.host}/`
                  : import.meta.env.VITE_BASE_URL
              }api/schedule/clear`,
            );

            if (response.ok) {
              toast('Reset schedule successfully', 2000);
            }
          } catch {
            toast('Failed to reset schedule', 2000);
          }
        }}
        class="w-full bg-blue-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:translate-y-[-1px] transition-all rounded"
      >
        Reset Scheduler
      </button>
    </Tooltip>
  );
};

const Scheduler: Component = () => {
  const [store, actions] = useStore();
  const { toast } = useToast();

  const handleAddItem = () => {
    actions.setSchedule([
      ...store.schedule,
      { pluginId: store.plugins[0].id || 1, duration: 1 },
    ]);
  };

  const handleRemoveItem = (index: number) => {
    actions.setSchedule(store.schedule.filter((_, i) => i !== index));
  };

  const handlePluginChange = (index: number, pluginId: number) => {
    actions.setSchedule(
      store.schedule.map((item, i) =>
        i === index ? { ...item, pluginId } : item,
      ),
    );
  };

  const handleDurationChange = (index: number, duration: number) => {
    actions.setSchedule(
      store.schedule.map((item, i) =>
        i === index ? { ...item, duration } : item,
      ),
    );
  };

  const handleSubmit = async () => {
    try {
      const response = await fetch(
        `${import.meta.env.VITE_BASE_URL}api/schedule`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
          },
          body: `schedule=${JSON.stringify(store.schedule)}`,
        },
      );

      if (response.ok) {
        toast('Schedule saved successfully', 2000);
      }
    } catch (error) {
      console.error('Failed to save schedule:', error);
      toast('Failed to save schedule', 2000);
    }
  };

  return (
    <Layout
      content={
        <div class="space-y-3 p-5">
          <h3 class="text-4xl text-white  tracking-wide">Scheduler</h3>

          <div class="bg-white p-6 rounded-md">
            <div class="space-y-2">
              <Show
                when={store.schedule?.length > 0}
                fallback={
                  <div class="text-md text-gray-500 italic">
                    No schedule set
                  </div>
                }
              >
                <For each={store.schedule}>
                  {(item, index) => (
                    <div class="flex items-center gap-4 p-4 bg-white rounded-lg shadow-sm border border-gray-100 hover:border-gray-200 transition-all duration-200">
                      <div class="flex-1">
                        <select
                          value={item.pluginId}
                          onChange={(e) =>
                            handlePluginChange(
                              index(),
                              parseInt(e.currentTarget.value),
                            )
                          }
                          class="w-full px-3 py-2 bg-gray-50 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 outline-none transition-all duration-200"
                        >
                          <For each={store.plugins}>
                            {(plugin) => (
                              <option value={plugin.id}>{plugin.name}</option>
                            )}
                          </For>
                        </select>
                      </div>

                      <div class="flex items-center gap-3">
                        <div class="relative">
                          <input
                            type="number"
                            min="1"
                            value={item.duration}
                            onInput={(e) =>
                              handleDurationChange(
                                index(),
                                parseInt(e.currentTarget.value),
                              )
                            }
                            class="pr-16 pl-3 py-2 w-32 bg-gray-50 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 outline-none transition-all duration-200"
                          />
                          <span class="absolute right-3 top-1/2 -translate-y-1/2 text-gray-400 text-sm">
                            {item.duration > 1 ? 'seconds' : 'second'}
                          </span>
                        </div>
                      </div>

                      <Show when={store.plugin === item.pluginId}>
                        <div class="flex items-center gap-1 px-2 py-1 bg-green-100 text-green-700 rounded-md">
                          <i class="fas fa-check text-sm" />
                          <span class="text-sm font-medium">Active</span>
                        </div>
                      </Show>

                      <button
                        onClick={() => handleRemoveItem(index())}
                        class="p-2 text-gray-400 hover:text-red-500 hover:bg-red-50 rounded-lg transition-all duration-200"
                        aria-label="Remove item"
                      >
                        <i class="fas fa-trash-alt text-lg" />
                      </button>
                    </div>
                  )}
                </For>
              </Show>
            </div>
          </div>
        </div>
      }
      sidebar={
        <>
          <div class="space-y-6">
            <div class="space-y-3">
              <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">
                Controls
              </h3>

              <div class="flex flex-col gap-2">
                <Button
                  onClick={handleAddItem}
                  class="hover:bg-green-600 transition-colors"
                >
                  <i class="fa-solid fa-plus mr-2" />
                  Add Plugin
                </Button>

                <Button
                  onClick={handleSubmit}
                  disabled={store.schedule?.length === 0}
                  class="hover:bg-blue-700 transition-colors"
                >
                  <i class="fa-solid fa-save mr-2" />
                  Send Schedule
                </Button>

                <Show when={store.isActiveScheduler}>
                  <div class="my-6 border-t border-gray-200" />
                  <ResetScheduleButton />
                </Show>
              </div>
            </div>
          </div>
          <div class="mt-auto pt-6 border-t border-gray-200">
            <Tooltip text="Return to main editor">
              <a
                href="#/"
                class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
              >
                <i class="fa-solid fa-arrow-left mr-2" />
                Back
              </a>
            </Tooltip>
          </div>
        </>
      }
    />
  );
};

export default Scheduler;

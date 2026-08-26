import { type Component, createSignal, onMount, Show } from "solid-js";

import Button from "./components/button";
import { Layout } from "./components/layout/layout";
import { useToast } from "./contexts/toast";

const API_URL = import.meta.env.PROD
  ? `http://${window.location.host}/`
  : import.meta.env.VITE_BASE_URL;

// POSIX TZ strings, offered as autocomplete suggestions for the timezone field
const TIMEZONE_PRESETS = [
  { label: "Central Europe (Zurich, Berlin, Rome)", value: "CET-1CEST,M3.5.0,M10.5.0/3" },
  { label: "United Kingdom / Ireland", value: "GMT0BST,M3.5.0/1,M10.5.0" },
  { label: "Eastern Europe (Athens, Helsinki)", value: "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { label: "US Eastern", value: "EST5EDT,M3.2.0,M11.1.0" },
  { label: "US Pacific", value: "PST8PDT,M3.2.0,M11.1.0" },
  { label: "UTC", value: "UTC0" },
];

const Settings: Component = () => {
  const { toast } = useToast();

  const [weatherLocation, setWeatherLocation] = createSignal("");
  const [ntpServer, setNtpServer] = createSignal("");
  const [tzInfo, setTzInfo] = createSignal("");
  const [autoStartSchedule, setAutoStartSchedule] = createSignal(false);
  const [isLoading, setIsLoading] = createSignal(true);
  const [isSaving, setIsSaving] = createSignal(false);

  const loadConfig = async () => {
    setIsLoading(true);
    try {
      const response = await fetch(`${API_URL}api/config`);
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }
      const config = await response.json();
      setWeatherLocation(config.weatherLocation ?? "");
      setNtpServer(config.ntpServer ?? "");
      setTzInfo(config.tzInfo ?? "");
      setAutoStartSchedule(config.autoStartSchedule ?? false);
    } catch (error) {
      console.error("Failed to load configuration:", error);
      toast("Failed to load configuration", 2000);
    } finally {
      setIsLoading(false);
    }
  };

  onMount(loadConfig);

  const handleSave = async () => {
    setIsSaving(true);
    try {
      const response = await fetch(`${API_URL}api/config`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          weatherLocation: weatherLocation().trim(),
          ntpServer: ntpServer().trim(),
          tzInfo: tzInfo().trim(),
          autoStartSchedule: autoStartSchedule(),
        }),
      });

      if (response.ok) {
        toast("Settings saved", 2000);
      } else {
        toast("Failed to save settings", 2000);
      }
    } catch (error) {
      console.error("Failed to save settings:", error);
      toast("Failed to save settings", 2000);
    } finally {
      setIsSaving(false);
    }
  };

  const handleReset = async () => {
    if (!window.confirm("Reset all settings to their defaults?")) {
      return;
    }

    try {
      const response = await fetch(`${API_URL}api/config/reset`, { method: "POST" });

      if (response.ok) {
        toast("Settings reset to defaults", 2000);
        await loadConfig();
      } else {
        toast("Failed to reset settings", 2000);
      }
    } catch (error) {
      console.error("Failed to reset settings:", error);
      toast("Failed to reset settings", 2000);
    }
  };

  const inputClass =
    "w-full px-3 py-3 lg:py-2 text-base bg-gray-50 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500 outline-none transition-all duration-200";

  return (
    <Layout
      content={
        <div class="space-y-3 p-5 pb-24 lg:pb-5">
          <h3 class="text-4xl text-white tracking-wide">Settings</h3>

          <div class="bg-white p-4 lg:p-6 rounded-md">
            <Show
              when={!isLoading()}
              fallback={<div class="text-center py-8 text-md text-gray-500 italic">Loading…</div>}
            >
              <div class="space-y-5 max-w-xl">
                <div class="space-y-1.5">
                  <label class="block text-sm font-semibold text-gray-700" for="weather-location">
                    Weather Location
                  </label>
                  <input
                    id="weather-location"
                    type="text"
                    placeholder="e.g. Como"
                    value={weatherLocation()}
                    onInput={(e) => setWeatherLocation(e.currentTarget.value)}
                    class={inputClass}
                  />
                  <p class="text-xs text-gray-500">
                    City name used by the weather plugin (resolved via Open-Meteo geocoding).
                  </p>
                </div>

                <div class="space-y-1.5">
                  <label class="block text-sm font-semibold text-gray-700" for="ntp-server">
                    NTP Server
                  </label>
                  <input
                    id="ntp-server"
                    type="text"
                    placeholder="e.g. pool.ntp.org"
                    value={ntpServer()}
                    onInput={(e) => setNtpServer(e.currentTarget.value)}
                    class={inputClass}
                  />
                </div>

                <div class="space-y-1.5">
                  <label class="block text-sm font-semibold text-gray-700" for="tz-info">
                    Timezone
                  </label>
                  <input
                    id="tz-info"
                    type="text"
                    list="tz-presets"
                    placeholder="e.g. CET-1CEST,M3.5.0,M10.5.0/3"
                    value={tzInfo()}
                    onInput={(e) => setTzInfo(e.currentTarget.value)}
                    class={inputClass}
                  />
                  <datalist id="tz-presets">
                    {TIMEZONE_PRESETS.map((preset) => (
                      <option value={preset.value}>{preset.label}</option>
                    ))}
                  </datalist>
                  <p class="text-xs text-gray-500">
                    POSIX timezone string. Pick a suggestion or enter your own.
                  </p>
                </div>

                <label class="flex items-center gap-3 cursor-pointer select-none" for="auto-start">
                  <input
                    id="auto-start"
                    type="checkbox"
                    checked={autoStartSchedule()}
                    onChange={(e) => setAutoStartSchedule(e.currentTarget.checked)}
                    class="w-4 h-4 accent-gray-700"
                  />
                  <span class="text-sm font-semibold text-gray-700">
                    Start plugin schedule automatically on boot
                  </span>
                </label>

                <div class="flex flex-col sm:flex-row gap-2 pt-2">
                  <button
                    type="button"
                    onClick={handleSave}
                    disabled={isSaving()}
                    class="flex-1 bg-green-600 hover:bg-green-700 text-white px-3 py-2 rounded-lg transition-all duration-200 font-semibold text-sm flex items-center justify-center gap-2 disabled:opacity-40"
                  >
                    <i class="fa-solid fa-floppy-disk" />
                    <span>{isSaving() ? "Saving…" : "Save Settings"}</span>
                  </button>
                  <button
                    type="button"
                    onClick={handleReset}
                    class="flex-1 bg-gray-700 hover:bg-red-600 text-white px-3 py-2 rounded-lg transition-all duration-200 font-semibold text-sm flex items-center justify-center gap-2"
                  >
                    <i class="fa-solid fa-rotate-left" />
                    <span>Reset Defaults</span>
                  </button>
                </div>
              </div>
            </Show>
          </div>
        </div>
      }
      sidebar={
        <>
          <div class="space-y-6">
            <div class="space-y-3">
              <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">Settings</h3>
              <p class="text-sm text-gray-600">
                Changes are stored on the device and applied immediately — the weather plugin
                refreshes and the clock re-syncs right after saving.
              </p>
              <Button onClick={loadConfig} class="hover:bg-gray-700 transition-colors">
                <i class="fa-solid fa-refresh mr-2" />
                Reload
              </Button>
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

export default Settings;

import { type Component, createSignal, For, type JSX, onMount, Show } from "solid-js";

import { Layout } from "./components/layout/layout";
import { useToast } from "./contexts/toast";

const API_URL = import.meta.env.PROD
  ? `http://${window.location.host}/`
  : import.meta.env.VITE_BASE_URL;

// POSIX TZ strings, the device cannot resolve IANA names like "Europe/Berlin"
const TIMEZONE_PRESETS = [
  { label: "UTC", value: "UTC0" },
  { label: "United Kingdom / Ireland", value: "GMT0BST,M3.5.0/1,M10.5.0" },
  { label: "Central Europe (Berlin, Paris, Rome)", value: "CET-1CEST,M3.5.0,M10.5.0/3" },
  { label: "Eastern Europe (Athens, Helsinki)", value: "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { label: "India", value: "IST-5:30" },
  { label: "Japan", value: "JST-9" },
  { label: "Australia East (Sydney)", value: "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { label: "Brazil (São Paulo)", value: "<-03>3" },
  { label: "US Eastern", value: "EST5EDT,M3.2.0,M11.1.0" },
  { label: "US Central", value: "CST6CDT,M3.2.0,M11.1.0" },
  { label: "US Mountain", value: "MST7MDT,M3.2.0,M11.1.0" },
  { label: "US Pacific", value: "PST8PDT,M3.2.0,M11.1.0" },
];

const CUSTOM_TIMEZONE = "custom";

// mirrors the power constants in include/constants.h
const POWER_LIMIT_DEFAULT_MW = 5000;
const POWER_LIMIT_MIN_MW = 1000;
const POWER_LIMIT_MAX_MW = 10000;
const POWER_BASE_MW = 600;
const POWER_PER_PIXEL_MW = 30;
const POWER_HEADROOM_PERCENT = 10;
const TOTAL_PIXELS = 256;

// brightness all pixels can share before the limit dims them, in percent
const fullScreenBrightness = (milliwatts: number) => {
  const usable = (milliwatts * (100 - POWER_HEADROOM_PERCENT)) / 100 - POWER_BASE_MW;
  const pixels = Math.max(usable / POWER_PER_PIXEL_MW, 1);
  return Math.min(100, Math.round((pixels / TOTAL_PIXELS) * 100));
};

interface Config {
  weatherLocation: string;
  ntpServer: string;
  tzInfo: string;
  autoStartSchedule: boolean;
  powerLimitMw: number;
}

interface DeviceInfo {
  ipAddress: string;
  macAddress: string;
  rssi: number;
  uptime: number;
  freeHeap: number;
  resetReason: string;
}

const EMPTY_CONFIG: Config = {
  weatherLocation: "",
  ntpServer: "",
  tzInfo: "",
  autoStartSchedule: false,
  powerLimitMw: 0,
};

const formatUptime = (seconds: number) => {
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  if (days > 0) return `${days}d ${hours}h`;
  if (hours > 0) return `${hours}h ${minutes}m`;
  return `${minutes}m`;
};

const signalLabel = (rssi: number) =>
  rssi >= -55 ? "Excellent" : rssi >= -67 ? "Good" : rssi >= -75 ? "Fair" : "Weak";

const Section: Component<{ title: string; icon: string; children: JSX.Element }> = (props) => (
  <section class="card space-y-4 p-4 lg:p-5">
    <h2 class="flex items-center gap-2 text-sm font-semibold">
      <i class={`fa-solid ${props.icon} w-4 text-center text-accent`} />
      {props.title}
    </h2>
    {props.children}
  </section>
);

const Field: Component<{ label: string; for: string; hint?: string; children: JSX.Element }> = (
  props,
) => (
  <div class="space-y-1.5">
    <label class="block text-sm font-medium" for={props.for}>
      {props.label}
    </label>
    {props.children}
    <Show when={props.hint}>
      <p class="text-xs text-muted">{props.hint}</p>
    </Show>
  </div>
);

const Settings: Component = () => {
  const { toast } = useToast();

  const [config, setConfig] = createSignal<Config>(EMPTY_CONFIG);
  const [savedConfig, setSavedConfig] = createSignal<Config>(EMPTY_CONFIG);
  const [customTimezone, setCustomTimezone] = createSignal(false);
  const [info, setInfo] = createSignal<DeviceInfo | null>(null);
  const [isLoading, setIsLoading] = createSignal(true);
  const [isSaving, setIsSaving] = createSignal(false);

  const update = <K extends keyof Config>(key: K, value: Config[K]) =>
    setConfig((current) => ({ ...current, [key]: value }));

  const isDirty = () => JSON.stringify(config()) !== JSON.stringify(savedConfig());
  const ntpMissing = () => config().ntpServer.trim() === "";
  const tzMissing = () => config().tzInfo.trim() === "";
  // 0 means disabled, only the switch sets it; while editing the field can be out of range
  const powerLimitEnabled = () => config().powerLimitMw !== 0;
  const powerLimitInvalid = () =>
    powerLimitEnabled() &&
    (config().powerLimitMw < POWER_LIMIT_MIN_MW || config().powerLimitMw > POWER_LIMIT_MAX_MW);
  // the last enabled value, restored when the limit is switched back on
  const [lastPowerLimitMw, setLastPowerLimitMw] = createSignal(POWER_LIMIT_DEFAULT_MW);
  const timezoneSelection = () =>
    customTimezone() || !TIMEZONE_PRESETS.some((preset) => preset.value === config().tzInfo)
      ? CUSTOM_TIMEZONE
      : config().tzInfo;

  const loadInfo = async () => {
    try {
      const response = await fetch(`${API_URL}api/info`);
      if (response.ok) {
        setInfo(await response.json());
      }
    } catch (error) {
      console.error("Failed to load device info:", error);
    }
  };

  const loadConfig = async () => {
    setIsLoading(true);
    try {
      const response = await fetch(`${API_URL}api/config`);
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }
      const data = await response.json();
      const loaded: Config = {
        weatherLocation: data.weatherLocation ?? "",
        ntpServer: data.ntpServer ?? "",
        tzInfo: data.tzInfo ?? "",
        autoStartSchedule: data.autoStartSchedule ?? false,
        powerLimitMw: data.powerLimitMw ?? 0,
      };
      if (loaded.powerLimitMw > 0) {
        setLastPowerLimitMw(loaded.powerLimitMw);
      }
      setConfig(loaded);
      setSavedConfig(loaded);
      setCustomTimezone(false);
    } catch (error) {
      console.error("Failed to load configuration:", error);
      toast("Failed to load configuration", 2000);
    } finally {
      setIsLoading(false);
    }
  };

  // one request at a time, the device answers parallel requests slowly or drops them
  const loadAll = async () => {
    await loadConfig();
    await loadInfo();
  };

  onMount(loadAll);

  const handleSave = async (event: SubmitEvent) => {
    event.preventDefault();
    if (!isDirty() || ntpMissing() || tzMissing() || powerLimitInvalid()) {
      return;
    }

    const trimmed: Config = {
      weatherLocation: config().weatherLocation.trim(),
      ntpServer: config().ntpServer.trim(),
      tzInfo: config().tzInfo.trim(),
      autoStartSchedule: config().autoStartSchedule,
      powerLimitMw: Math.round(config().powerLimitMw),
    };

    setIsSaving(true);
    try {
      const response = await fetch(`${API_URL}api/config`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(trimmed),
      });

      if (response.ok) {
        setConfig(trimmed);
        setSavedConfig(trimmed);
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

  const handleTimezoneSelect = (value: string) => {
    if (value === CUSTOM_TIMEZONE) {
      setCustomTimezone(true);
      return;
    }
    setCustomTimezone(false);
    update("tzInfo", value);
  };

  const deviceRows = () => {
    const device = info();
    if (!device) return [];
    return [
      { label: "IP address", value: device.ipAddress },
      { label: "MAC address", value: device.macAddress },
      { label: "WiFi signal", value: `${signalLabel(device.rssi)} (${device.rssi} dBm)` },
      { label: "Uptime", value: formatUptime(device.uptime) },
      { label: "Free memory", value: `${Math.round(device.freeHeap / 1024)} KB` },
      { label: "Last reset", value: device.resetReason },
    ];
  };

  return (
    <Layout
      content={
        <div class="mx-auto w-full max-w-3xl space-y-5 p-4 lg:p-8">
          <div class="flex flex-wrap items-end justify-between gap-3">
            <div>
              <h1 class="text-2xl font-semibold tracking-tight">Settings</h1>
              <p class="mt-1 text-sm text-muted">Stored on the device, applied on save.</p>
            </div>
            <Show when={isDirty()}>
              <span class="inline-flex items-center gap-2 rounded-full border border-amber-400/30 bg-amber-400/10 px-3 py-1 text-xs font-medium text-accent">
                <span class="size-1.5 rounded-full bg-amber-400" />
                Unsaved changes
              </span>
            </Show>
          </div>

          <Show
            when={!isLoading()}
            fallback={
              <div class="card px-6 py-14 text-center text-sm text-muted">
                <div class="mx-auto mb-4 size-8 animate-spin rounded-full border-2 border-line border-t-accent" />
                Loading settings…
              </div>
            }
          >
            <form class="space-y-4" onSubmit={handleSave}>
              <Section title="Weather" icon="fa-cloud-sun">
                <Field
                  label="Location"
                  for="weather-location"
                  hint="City name for the weather plugin, resolved via Open-Meteo geocoding."
                >
                  <input
                    id="weather-location"
                    type="text"
                    placeholder="e.g. Como"
                    value={config().weatherLocation}
                    onInput={(e) => update("weatherLocation", e.currentTarget.value)}
                    class="input"
                  />
                </Field>
              </Section>

              <Section title="Time" icon="fa-clock">
                <Field label="Timezone" for="tz-preset">
                  <select
                    id="tz-preset"
                    value={timezoneSelection()}
                    onChange={(e) => handleTimezoneSelect(e.currentTarget.value)}
                    class="input"
                  >
                    <For each={TIMEZONE_PRESETS}>
                      {(preset) => <option value={preset.value}>{preset.label}</option>}
                    </For>
                    <option value={CUSTOM_TIMEZONE}>Custom POSIX string…</option>
                  </select>
                </Field>

                <Show when={timezoneSelection() === CUSTOM_TIMEZONE}>
                  <Field
                    label="POSIX timezone"
                    for="tz-info"
                    hint={
                      tzMissing()
                        ? "Required. Example: CET-1CEST,M3.5.0,M10.5.0/3"
                        : "Format: name, UTC offset (inverted sign), optional DST rule."
                    }
                  >
                    <input
                      id="tz-info"
                      type="text"
                      placeholder="e.g. CET-1CEST,M3.5.0,M10.5.0/3"
                      value={config().tzInfo}
                      onInput={(e) => update("tzInfo", e.currentTarget.value)}
                      aria-invalid={tzMissing()}
                      class={`input font-mono ${tzMissing() ? "border-danger/60" : ""}`}
                    />
                  </Field>
                </Show>

                <Field
                  label="NTP server"
                  for="ntp-server"
                  hint={ntpMissing() ? "Required to keep the clock in sync." : undefined}
                >
                  <input
                    id="ntp-server"
                    type="text"
                    placeholder="e.g. pool.ntp.org"
                    value={config().ntpServer}
                    onInput={(e) => update("ntpServer", e.currentTarget.value)}
                    aria-invalid={ntpMissing()}
                    class={`input ${ntpMissing() ? "border-danger/60" : ""}`}
                  />
                </Field>
              </Section>

              <Section title="Startup" icon="fa-power-off">
                <label
                  class="flex cursor-pointer items-center justify-between gap-4"
                  for="auto-start"
                >
                  <span class="text-sm font-medium">Start plugin schedule on boot</span>
                  <input
                    id="auto-start"
                    type="checkbox"
                    checked={config().autoStartSchedule}
                    onChange={(e) => update("autoStartSchedule", e.currentTarget.checked)}
                    class="switch"
                  />
                </label>
              </Section>

              <Section title="Power" icon="fa-bolt">
                <label
                  class="flex cursor-pointer items-center justify-between gap-4"
                  for="power-limit-enabled"
                >
                  <span class="text-sm font-medium">Limit power draw</span>
                  <input
                    id="power-limit-enabled"
                    type="checkbox"
                    checked={powerLimitEnabled()}
                    onChange={(e) =>
                      update("powerLimitMw", e.currentTarget.checked ? lastPowerLimitMw() : 0)
                    }
                    class="switch"
                  />
                </label>

                <Show when={powerLimitEnabled()}>
                  <Field
                    label="Power supply (mW)"
                    for="power-limit"
                    hint={
                      powerLimitInvalid()
                        ? `Between ${POWER_LIMIT_MIN_MW} and ${POWER_LIMIT_MAX_MW} mW.`
                        : `Bright frames are dimmed to stay within this budget. A fully lit screen gets up to ~${fullScreenBrightness(config().powerLimitMw)}% brightness, a few pixels stay at full brightness.`
                    }
                  >
                    <input
                      id="power-limit"
                      type="number"
                      min={POWER_LIMIT_MIN_MW}
                      max={POWER_LIMIT_MAX_MW}
                      step="100"
                      value={config().powerLimitMw > 0 ? config().powerLimitMw : ""}
                      onInput={(e) => {
                        const milliwatts = e.currentTarget.valueAsNumber;
                        // empty or 0 keeps the limit enabled but invalid instead of switching it off
                        update("powerLimitMw", milliwatts > 0 ? milliwatts : -1);
                        if (milliwatts >= POWER_LIMIT_MIN_MW && milliwatts <= POWER_LIMIT_MAX_MW) {
                          setLastPowerLimitMw(milliwatts);
                        }
                      }}
                      aria-invalid={powerLimitInvalid()}
                      class={`input ${powerLimitInvalid() ? "border-danger/60" : ""}`}
                    />
                  </Field>
                </Show>
              </Section>

              <div class="flex flex-col gap-2 sm:flex-row">
                <button
                  type="button"
                  onClick={() => {
                    setConfig(savedConfig());
                    setCustomTimezone(false);
                  }}
                  disabled={!isDirty() || isSaving()}
                  class="btn flex-1"
                >
                  <i class="fa-solid fa-xmark" />
                  Discard changes
                </button>
                <button
                  type="submit"
                  disabled={
                    !isDirty() || isSaving() || ntpMissing() || tzMissing() || powerLimitInvalid()
                  }
                  class="btn btn-accent flex-1"
                >
                  <i class="fa-solid fa-floppy-disk" />
                  {isSaving() ? "Saving…" : "Save settings"}
                </button>
              </div>
            </form>
          </Show>

          <Show when={info()}>
            <Section title="Device" icon="fa-microchip">
              <dl class="grid grid-cols-1 gap-x-6 gap-y-3 text-sm sm:grid-cols-2">
                <For each={deviceRows()}>
                  {(row) => (
                    <div class="flex items-baseline justify-between gap-3 border-b border-line pb-2">
                      <dt class="text-muted">{row.label}</dt>
                      <dd class="truncate font-mono text-xs">{row.value}</dd>
                    </div>
                  )}
                </For>
              </dl>
            </Section>
          </Show>
        </div>
      }
      sidebar={
        <div class="space-y-6">
          <div class="space-y-3">
            <h3 class="section-title">About</h3>
            <p class="text-sm text-muted">
              Saving applies changes right away: the weather plugin refreshes and the clock
              re-syncs.
            </p>
            <button type="button" onClick={loadAll} class="btn w-full">
              <i class="fa-solid fa-rotate" />
              Reload from device
            </button>
          </div>
          <div class="space-y-3">
            <h3 class="section-title">Danger zone</h3>
            <button type="button" onClick={handleReset} class="btn btn-danger w-full">
              <i class="fa-solid fa-rotate-left" />
              Reset to defaults
            </button>
          </div>
        </div>
      }
    />
  );
};

export default Settings;

import type { JSX } from "@solidjs/web";
import { type Component, createSignal, For, onSettled, Show } from "solid-js";

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

interface Config {
  weatherLocation: string;
  ntpServer: string;
  tzInfo: string;
  autoStartSchedule: boolean;
  mqttEnabled: boolean;
  mqttHost: string;
  mqttPort: number;
  mqttUser: string;
  // only sent when typed, the device never returns the stored password
  mqttPassword: string;
  mqttTopic: string;
  mqttDiscovery: boolean;
  mqttDiscoveryPrefix: string;
}

interface DeviceInfo {
  ipAddress: string;
  macAddress: string;
  rssi: number;
  uptime: number;
  freeHeap: number;
  resetReason: string;
  mqttConnected?: boolean;
}

const EMPTY_CONFIG: Config = {
  weatherLocation: "",
  ntpServer: "",
  tzInfo: "",
  autoStartSchedule: false,
  mqttEnabled: false,
  mqttHost: "",
  mqttPort: 1883,
  mqttUser: "",
  mqttPassword: "",
  mqttTopic: "",
  mqttDiscovery: true,
  mqttDiscoveryPrefix: "homeassistant",
};

// same rules as the firmware: no wildcards, spaces or leading/trailing slash
const isValidTopic = (topic: string) =>
  topic === "" || (!/[#+\s]/.test(topic) && !topic.startsWith("/") && !topic.endsWith("/"));

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
  const [mqttPasswordSet, setMqttPasswordSet] = createSignal(false);

  const update = <K extends keyof Config>(key: K, value: Config[K]) =>
    setConfig((current) => ({ ...current, [key]: value }));

  const isDirty = () => JSON.stringify(config()) !== JSON.stringify(savedConfig());
  const ntpMissing = () => config().ntpServer.trim() === "";
  const tzMissing = () => config().tzInfo.trim() === "";
  const mqttHostMissing = () => config().mqttEnabled && config().mqttHost.trim() === "";
  const mqttPortInvalid = () =>
    !Number.isInteger(config().mqttPort) || config().mqttPort < 1 || config().mqttPort > 65535;
  const mqttTopicInvalid = () => !isValidTopic(config().mqttTopic.trim());
  const mqttPrefixInvalid = () => !isValidTopic(config().mqttDiscoveryPrefix.trim());
  const hasErrors = () =>
    ntpMissing() ||
    tzMissing() ||
    mqttHostMissing() ||
    mqttPortInvalid() ||
    mqttTopicInvalid() ||
    mqttPrefixInvalid();
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
        mqttEnabled: data.mqttEnabled ?? false,
        mqttHost: data.mqttHost ?? "",
        mqttPort: data.mqttPort ?? 1883,
        mqttUser: data.mqttUser ?? "",
        mqttPassword: "",
        mqttTopic: data.mqttTopic ?? "",
        mqttDiscovery: data.mqttDiscovery ?? true,
        mqttDiscoveryPrefix: data.mqttDiscoveryPrefix ?? "homeassistant",
      };
      setMqttPasswordSet(data.mqttPasswordSet ?? false);
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

  onSettled(() => {
    void loadAll();
  });

  const handleSave = async (event: SubmitEvent) => {
    event.preventDefault();
    if (!isDirty() || hasErrors()) {
      return;
    }

    const current = config();
    const trimmed: Config = {
      weatherLocation: current.weatherLocation.trim(),
      ntpServer: current.ntpServer.trim(),
      tzInfo: current.tzInfo.trim(),
      autoStartSchedule: current.autoStartSchedule,
      mqttEnabled: current.mqttEnabled,
      mqttHost: current.mqttHost.trim(),
      mqttPort: current.mqttPort,
      mqttUser: current.mqttUser.trim(),
      mqttPassword: current.mqttPassword,
      mqttTopic: current.mqttTopic.trim(),
      mqttDiscovery: current.mqttDiscovery,
      mqttDiscoveryPrefix: current.mqttDiscoveryPrefix.trim(),
    };
    // an absent password keeps the stored one
    const { mqttPassword, ...rest } = trimmed;
    const payload = mqttPassword === "" ? rest : trimmed;

    setIsSaving(true);
    try {
      const response = await fetch(`${API_URL}api/config`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(payload),
      });

      if (response.ok) {
        const saved = { ...trimmed, mqttPassword: "" };
        if (mqttPassword !== "") setMqttPasswordSet(true);
        setConfig(saved);
        setSavedConfig(saved);
        toast("Settings saved", 2000);
        // give the client a moment to (re)connect before showing its status
        setTimeout(() => void loadInfo(), 3000);
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
      ...(device.mqttConnected === undefined
        ? []
        : [{ label: "MQTT", value: device.mqttConnected ? "Connected" : "Not connected" }]),
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
                      aria-invalid={tzMissing() ? "true" : "false"}
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
                    aria-invalid={ntpMissing() ? "true" : "false"}
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

              <Section title="MQTT / Home Assistant" icon="fa-house-signal">
                <label
                  class="flex cursor-pointer items-center justify-between gap-4"
                  for="mqtt-enabled"
                >
                  <span>
                    <span class="block text-sm font-medium">Connect to an MQTT broker</span>
                    <span class="block text-xs text-muted">
                      Home Assistant finds the lamp on its own when discovery is on.
                    </span>
                  </span>
                  <input
                    id="mqtt-enabled"
                    type="checkbox"
                    checked={config().mqttEnabled}
                    onChange={(e) => update("mqttEnabled", e.currentTarget.checked)}
                    class="switch"
                  />
                </label>

                <Show when={config().mqttEnabled}>
                  <Show when={info()?.mqttConnected !== undefined && !isDirty()}>
                    <p class="flex items-center gap-2 text-xs text-muted">
                      <span
                        class={`size-1.5 rounded-full ${info()?.mqttConnected ? "bg-emerald-400" : "bg-danger"}`}
                      />
                      {info()?.mqttConnected ? "Connected to broker" : "Not connected"}
                    </p>
                  </Show>

                  <div class="grid grid-cols-1 gap-4 sm:grid-cols-[1fr_8rem]">
                    <Field
                      label="Broker host"
                      for="mqtt-host"
                      hint={mqttHostMissing() ? "Required while MQTT is on." : undefined}
                    >
                      <input
                        id="mqtt-host"
                        type="text"
                        placeholder="e.g. 192.168.1.10 or homeassistant.local"
                        value={config().mqttHost}
                        onInput={(e) => update("mqttHost", e.currentTarget.value)}
                        aria-invalid={mqttHostMissing() ? "true" : "false"}
                        class={`input ${mqttHostMissing() ? "border-danger/60" : ""}`}
                      />
                    </Field>
                    <Field label="Port" for="mqtt-port">
                      <input
                        id="mqtt-port"
                        type="number"
                        min="1"
                        max="65535"
                        value={config().mqttPort}
                        onInput={(e) => update("mqttPort", e.currentTarget.valueAsNumber)}
                        aria-invalid={mqttPortInvalid() ? "true" : "false"}
                        class={`input ${mqttPortInvalid() ? "border-danger/60" : ""}`}
                      />
                    </Field>
                  </div>

                  <div class="grid grid-cols-1 gap-4 sm:grid-cols-2">
                    <Field label="Username" for="mqtt-user">
                      <input
                        id="mqtt-user"
                        type="text"
                        autocomplete="off"
                        placeholder="optional"
                        value={config().mqttUser}
                        onInput={(e) => update("mqttUser", e.currentTarget.value)}
                        class="input"
                      />
                    </Field>
                    <Field
                      label="Password"
                      for="mqtt-password"
                      hint={mqttPasswordSet() ? "Stored. Leave empty to keep it." : undefined}
                    >
                      <input
                        id="mqtt-password"
                        type="password"
                        autocomplete="new-password"
                        placeholder={mqttPasswordSet() ? "••••••••" : "optional"}
                        value={config().mqttPassword}
                        onInput={(e) => update("mqttPassword", e.currentTarget.value)}
                        class="input"
                      />
                    </Field>
                  </div>

                  <Field
                    label="Base topic"
                    for="mqtt-topic"
                    hint={
                      mqttTopicInvalid()
                        ? "No #, + or spaces, and no leading or trailing slash."
                        : "State and commands live below this topic. Empty uses the default."
                    }
                  >
                    <input
                      id="mqtt-topic"
                      type="text"
                      placeholder="obegraensad/<mac>"
                      value={config().mqttTopic}
                      onInput={(e) => update("mqttTopic", e.currentTarget.value)}
                      aria-invalid={mqttTopicInvalid() ? "true" : "false"}
                      class={`input font-mono ${mqttTopicInvalid() ? "border-danger/60" : ""}`}
                    />
                  </Field>

                  <label
                    class="flex cursor-pointer items-center justify-between gap-4"
                    for="mqtt-discovery"
                  >
                    <span class="text-sm font-medium">Home Assistant discovery</span>
                    <input
                      id="mqtt-discovery"
                      type="checkbox"
                      checked={config().mqttDiscovery}
                      onChange={(e) => update("mqttDiscovery", e.currentTarget.checked)}
                      class="switch"
                    />
                  </label>

                  <Show when={config().mqttDiscovery}>
                    <Field
                      label="Discovery prefix"
                      for="mqtt-prefix"
                      hint={
                        mqttPrefixInvalid()
                          ? "No #, + or spaces, and no leading or trailing slash."
                          : "Only change this if you changed it in Home Assistant."
                      }
                    >
                      <input
                        id="mqtt-prefix"
                        type="text"
                        placeholder="homeassistant"
                        value={config().mqttDiscoveryPrefix}
                        onInput={(e) => update("mqttDiscoveryPrefix", e.currentTarget.value)}
                        aria-invalid={mqttPrefixInvalid() ? "true" : "false"}
                        class={`input font-mono ${mqttPrefixInvalid() ? "border-danger/60" : ""}`}
                      />
                    </Field>
                  </Show>
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
                  disabled={!isDirty() || isSaving() || hasErrors()}
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

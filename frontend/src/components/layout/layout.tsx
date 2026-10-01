import { type Component, createSignal, For, type JSX, onCleanup, onMount, Show } from "solid-js";

import { useStore } from "../../contexts/store";
import { ScreenInfo } from "../screen-info";

interface NavItem {
  href: string;
  label: string;
  icon: string;
  visible?: () => boolean;
}

const Brand: Component = () => {
  const [store] = useStore();

  return (
    <div class="flex items-center">
      <div class="leading-tight">
        <div class="text-sm font-semibold tracking-wide">OBEGRÄNSAD</div>
        <div class="flex items-center gap-1.5 text-xs text-muted">
          <span
            class={`size-1.5 rounded-full ${store.connectionState() === 1 ? "bg-emerald-400" : "bg-amber-400"}`}
          />
          {store.connectionState() === 1 ? "Connected" : store.connectionStatus}
        </div>
      </div>
    </div>
  );
};

export const Layout: Component<{
  content: JSX.Element;
  sidebar: JSX.Element;
  ref?: HTMLElement;
}> = (props) => {
  const [store] = useStore();
  const [hash, setHash] = createSignal(window.location.hash || "#/");

  const onHashChange = () => setHash(window.location.hash || "#/");
  onMount(() => window.addEventListener("hashchange", onHashChange));
  onCleanup(() => window.removeEventListener("hashchange", onHashChange));

  const navItems: NavItem[] = [
    { href: "#/", label: "Display", icon: "fa-table-cells" },
    {
      href: "#/creator",
      label: "Animation Creator",
      icon: "fa-pencil",
      visible: () => store.plugins.some((p) => p.name.includes("Animation")),
    },
    { href: "#/scheduler", label: "Scheduler", icon: "fa-clock" },
    { href: "#/settings", label: "Settings", icon: "fa-gear" },
    { href: "/update", label: "Firmware Update", icon: "fa-download" },
  ];

  const visibleNavItems = () => navItems.filter((item) => item.visible?.() ?? true);
  const isActive = (item: NavItem) => item.href === hash();

  return (
    <div class="h-full flex flex-col">
      <Show
        when={store.connectionState() === 1}
        fallback={
          <main class="h-full overflow-auto">
            <ScreenInfo>
              <div class="mb-6 flex justify-center">
                <Brand />
              </div>
              <div class="mx-auto mb-4 size-8 animate-spin rounded-full border-2 border-line border-t-accent" />
              <h2 class="mb-2 text-xl font-semibold">{store.connectionStatus}…</h2>
              <Show when={store.connectionState() === 0}>
                <p class="mx-auto max-w-sm text-sm text-muted">
                  Make sure your device is powered on and connected to the same network. Check the
                  browser console for connection errors.
                </p>
              </Show>
            </ScreenInfo>
          </main>
        }
      >
        <div class="flex-1 min-h-0 flex flex-col overflow-y-auto lg:overflow-hidden lg:grid lg:grid-cols-[300px_1fr] lg:gap-4 lg:p-4">
          <header class="lg:hidden sticky top-0 z-30 flex items-center justify-between gap-2 border-b border-line bg-canvas/80 px-4 py-3 backdrop-blur-lg">
            <Brand />
            <nav class="flex gap-1">
              <For each={visibleNavItems()}>
                {(item) => (
                  <a
                    href={item.href}
                    aria-label={item.label}
                    title={item.label}
                    aria-current={isActive(item) ? "page" : undefined}
                    class={`btn btn-icon ${isActive(item) ? "" : "btn-ghost"}`}
                  >
                    <i class={`fa-solid ${item.icon}`} />
                  </a>
                )}
              </For>
            </nav>
          </header>

          <aside class="card order-2 mx-4 mb-[max(1rem,env(safe-area-inset-bottom))] flex flex-col p-5 lg:order-0 lg:m-0 lg:min-h-0">
            <div class="hidden lg:block">
              <Brand />
              <nav class="mt-6 flex flex-col gap-1">
                <For each={visibleNavItems()}>
                  {(item) => (
                    <a
                      href={item.href}
                      aria-current={isActive(item) ? "page" : undefined}
                      class={`flex items-center gap-3 rounded-xl px-3 py-2 text-sm font-medium transition ${
                        isActive(item)
                          ? "bg-raised text-fg"
                          : "text-muted hover:bg-raised/60 hover:text-fg"
                      }`}
                    >
                      <i class={`fa-solid ${item.icon} w-4 text-center`} />
                      {item.label}
                    </a>
                  )}
                </For>
              </nav>
              <div class="my-5 border-t border-line" />
            </div>
            <div class="flex-1 min-h-0 lg:overflow-y-auto lg:-mr-2 lg:pr-2">{props.sidebar}</div>
          </aside>

          <main class="shrink-0 lg:h-full lg:min-h-0 lg:overflow-auto" ref={props.ref}>
            {props.content}
          </main>
        </div>
      </Show>
    </div>
  );
};

export default Layout;

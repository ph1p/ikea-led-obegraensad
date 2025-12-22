import { type Component, createSignal, type JSX, Show } from "solid-js";

import { useStore } from "../../contexts/store";
import { ScreenInfo } from "../screen-info";

export const Layout: Component<{
  content: JSX.Element;
  sidebar: JSX.Element;
  ref?: HTMLElement;
}> = (props) => {
  const [store] = useStore();
  const [isMobileMenuOpen, setIsMobileMenuOpen] = createSignal(false);

  const closeMobileMenu = () => setIsMobileMenuOpen(false);

  return (
    <div class="h-full flex flex-col">
      <Show
        when={store.connectionState() === 1}
        fallback={
          <main class="h-full overflow-auto">
            <ScreenInfo>
              <h2 class="text-4xl mb-4">{store.connectionStatus}...</h2>
              <Show when={store.connectionState() === 0}>
                <p class="text-white text-sm max-w-md mx-auto">
                  Make sure your device is powered on and connected to the same network. Check
                  browser console for connection errors.
                </p>
              </Show>
            </ScreenInfo>
          </main>
        }
      >
        <div class="flex-1 lg:grid lg:grid-cols-[320px_1fr] lg:gap-6 lg:p-6 relative overflow-hidden">
          <Show when={isMobileMenuOpen()}>
            <button
              type="button"
              class="lg:hidden fixed inset-0 bg-black bg-opacity-50 z-40 border-0 p-0 cursor-default"
              onClick={closeMobileMenu}
              onKeyDown={(e) => {
                if (e.key === "Enter" || e.key === " " || e.key === "Escape") {
                  closeMobileMenu();
                }
              }}
            />
          </Show>

          <aside
            class={`bg-white p-6 shadow-lg flex flex-col
              lg:relative lg:h-full lg:rounded-2xl
              fixed top-0 left-0 h-full w-full z-50
              transition-transform duration-300 ease-in-out
              ${isMobileMenuOpen() ? "translate-x-0" : "-translate-x-full lg:translate-x-0"}`}
            onClick={(e) => {
              const target = e.target as HTMLElement;
              if (target.tagName === "A" || target.closest("a")) {
                closeMobileMenu();
              }
            }}
            onKeyDown={(e) => {
              if (e.key === "Enter") {
                const target = e.target as HTMLElement;
                if (target.tagName === "A" || target.closest("a")) {
                  closeMobileMenu();
                }
              }
            }}
          >
            {props.sidebar}
            <button
              type="button"
              onClick={closeMobileMenu}
              class="lg:hidden mt-4 w-full bg-gray-700 text-white border-0 px-3 py-2 text-sm cursor-pointer font-semibold hover:opacity-80 active:-translate-y-px transition-all rounded"
            >
              <i class="fa-solid fa-times mr-2" />
              Close Menu
            </button>
          </aside>

          <main class="h-full overflow-auto" ref={props.ref}>
            {props.content}
          </main>
        </div>

        <button
          type="button"
          onClick={() => setIsMobileMenuOpen(!isMobileMenuOpen())}
          class="lg:hidden fixed bottom-6 right-6 z-30 bg-gray-700 text-white border-0 py-3 px-4 rounded-full shadow-lg cursor-pointer font-bold hover:opacity-80 active:-translate-y-px transition-all"
        >
          <i class="fa-solid fa-bars text-xl" />
        </button>
      </Show>
    </div>
  );
};

export default Layout;

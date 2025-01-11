import { Component, JSX, Show } from 'solid-js';
import { useStore } from '../../contexts/store';
import { ScreenInfo } from '../screen-info';

export const Layout: Component<{
  content: JSX.Element;
  sidebar: JSX.Element;
  ref?: any;
}> = (props) => {
  const [store] = useStore();

  return (
    <div
      class={`h-full ${
        store.connectionState() === 1 ? 'grid grid-cols-[320px_1fr]' : ''
      }`}
    >
      <Show
        when={store.connectionState() === 1}
        fallback={
          <main class="h-full overflow-auto">
            <ScreenInfo>
              <h2 class="text-4xl">{store.connectionStatus}...</h2>
            </ScreenInfo>
          </main>
        }
      >
        <aside class="bg-white p-6 flex flex-col h-full">{props.sidebar}</aside>
        <main class="h-full overflow-auto" ref={props.ref}>
          {props.content}
        </main>
      </Show>
    </div>
  );
};

export default Layout;

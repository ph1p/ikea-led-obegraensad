import { Component, Show } from 'solid-js';
import { Sidebar } from './sidebar';

export const Layout: Component<{
  content: any;
  sidebarContent?: any;
  store?: any;
  onRotate?: (turnRight: boolean) => void;
  onLoadImage?: () => void;
  onClear?: () => void;
  onPersist?: () => void;
  onLoad?: () => void;
  onPluginChange?: (pluginId: number) => void;
  onBrightnessChange?: (value: number) => void;
  onPersistPlugin?: () => void;
  ref?: any;
}> = (props) => {
  return (
    <div class="h-full grid grid-cols-[320px_1fr]">
      <Show when={!props.sidebarContent} fallback={props.sidebarContent}>
        <Sidebar
          store={props.store!}
          onRotate={props.onRotate!}
          onLoadImage={props.onLoadImage!}
          onClear={props.onClear!}
          onPersist={props.onPersist!}
          onLoad={props.onLoad!}
          onPluginChange={props.onPluginChange!}
          onBrightnessChange={props.onBrightnessChange!}
          onPersistPlugin={props.onPersistPlugin!}
        />
      </Show>
      <main class="h-full overflow-auto" ref={props.ref}>
        {props.content}
      </main>
    </div>
  );
};

export default Layout;

import type { Component } from "solid-js";

const Kbd: Component<{ children: string }> = (props) => (
  <kbd class="px-1.5 py-0.5 bg-gray-100 rounded border border-gray-300 font-mono">
    {props.children}
  </kbd>
);

export const KeyboardShortcutsHelp: Component = () => {
  return (
    <div class="space-y-3">
      <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">
        Keyboard Shortcuts
      </h3>
      <div class="text-xs text-gray-600 space-y-3">
        <p>
          <Kbd>←</Kbd> <Kbd>→</Kbd> Navigate frames
        </p>
        <p>
          <Kbd>Space</Kbd> Play/Stop
        </p>
        <p>
          <Kbd>D</Kbd> Duplicate frame
        </p>
        <p>
          <Kbd>Del</Kbd> Delete frame
        </p>
        <p>
          <Kbd>Ctrl+Z</Kbd> Undo
        </p>
        <p>
          <Kbd>Ctrl+Shift+Z</Kbd> Redo
        </p>
      </div>
    </div>
  );
};

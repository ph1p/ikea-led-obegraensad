import type { Component } from "solid-js";

const Kbd: Component<{ children: string }> = (props) => <kbd class="kbd">{props.children}</kbd>;

export const KeyboardShortcutsHelp: Component = () => {
  return (
    <div class="space-y-3">
      <h3 class="section-title">Keyboard Shortcuts</h3>
      <div class="space-y-2.5 text-xs text-muted">
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

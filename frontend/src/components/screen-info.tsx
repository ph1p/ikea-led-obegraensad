import type { Component, ParentProps } from "solid-js";

export const ScreenInfo: Component<ParentProps> = (props) => (
  <div class="grid h-full place-items-center p-8">
    <div class="text-center">{props.children}</div>
  </div>
);

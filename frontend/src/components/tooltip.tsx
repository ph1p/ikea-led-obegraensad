import { Component, JSX } from "solid-js";

interface TooltipProps {
  text: string;
  children: JSX.Element;
}

export const Tooltip: Component<TooltipProps> = (props) => (
  <div class="group relative inline-block w-full">
    {props.children}
    <div class="absolute w-max min-w-max pointer-events-none z-50 left-0 -top-8 bg-gray-900 text-white text-sm px-2 py-1 rounded opacity-0 group-hover:opacity-100 transition-opacity">
      {props.text}
      <div class="border-gray-900 border-solid border-t-[6px] border-x-[6px] border-x-transparent absolute -bottom-1.5 left-4" />
    </div>
  </div>
);

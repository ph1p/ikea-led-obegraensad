import type { Component, JSX } from "solid-js";

export const Button: Component<
  JSX.ButtonHTMLAttributes<HTMLButtonElement> & { widthAuto?: boolean }
> = (props) => {
  return (
    <button
      {...props}
      class={`${props.widthAuto ? "w-auto" : "w-full"} bg-gray-600 text-white border-0 px-4 py-3 uppercase text-sm leading-6 tracking-wider cursor-pointer font-bold hover:opacity-80 active:-translate-y-px transition-all rounded disabled:opacity-40 hover:disabled:bg-gray-600 ${
        props.class || ""
      }`}
    >
      {props.children}
    </button>
  );
};

export default Button;

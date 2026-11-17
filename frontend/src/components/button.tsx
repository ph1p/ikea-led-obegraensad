import type { JSX } from "@solidjs/web";
import type { Component } from "solid-js";

export const Button: Component<
  JSX.ButtonHTMLAttributes<HTMLButtonElement> & { widthAuto?: boolean }
> = (props) => {
  return (
    <button
      type="button"
      {...props}
      class={`btn ${props.widthAuto ? "" : "w-full"} ${props.class || ""}`}
    >
      {props.children}
    </button>
  );
};

export default Button;

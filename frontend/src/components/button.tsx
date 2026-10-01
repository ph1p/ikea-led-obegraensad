import type { Component, JSX } from "solid-js";

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

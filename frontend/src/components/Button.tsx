import { Component, JSX } from 'solid-js';
import { button } from './Button.css';

export const Button: Component<JSX.HTMLAttributes<HTMLButtonElement>> = (
  props
) => {
  return (
    <button class={button} {...props}>
      {props.children}
    </button>
  );
};

import { JSXInternal } from 'preact/src/jsx';
import { button } from './Button.css';

export const Button = (
  props: JSXInternal.HTMLAttributes<HTMLButtonElement>
) => {
  return (
    <button className={button} {...props}>
      {props.children}
    </button>
  );
};

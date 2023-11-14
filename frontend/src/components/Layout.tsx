import { Component } from 'solid-js';
import { content, footer, layout } from './Layout.css';

export const Layout: Component<{ content: any; footer: any; ref?: any }> = (
  props
) => {
  return (
    <main class={layout}>
      <section class={content} ref={props.ref}>
        {props.content}
      </section>
      <footer class={footer}>{props.footer}</footer>
    </main>
  );
};

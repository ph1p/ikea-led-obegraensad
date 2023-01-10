import { Component } from 'solid-js';
import { content, footer, layout } from './Layout.css';

export const Layout: Component<{ content: any; footer: any }> = (props) => {
  return (
    <main class={layout}>
      <section class={content}>{props.content}</section>
      <footer class={footer}>{props.footer}</footer>
    </main>
  );
};

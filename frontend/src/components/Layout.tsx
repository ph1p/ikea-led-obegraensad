import { VNode } from 'preact';
import { content, footer, layout } from './Layout.css';

export const Layout = (props: { content: VNode; footer: VNode }) => {
  return (
    <main className={layout}>
      <section className={content}>{props.content}</section>
      <footer className={footer}>{props.footer}</footer>
    </main>
  );
};

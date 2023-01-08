/* @refresh reload */
import { render } from 'solid-js/web';
import { createSignal, Match, onCleanup, onMount, Switch } from 'solid-js';
import './index.css';

import { App } from './app';
import { Creator } from './creator';

const Router = () => {
  const [hash, setHash] = createSignal(window.location.hash);
  const location = () => hash().replace(/^#/, '') || '/';

  const onChange = () => setHash(window.location.hash);

  onMount(() => {
    window.addEventListener('hashchange', onChange, false);
  });

  onCleanup(() => {
    window.removeEventListener('hashchange', onChange, false);
  });

  return (
    <Switch fallback={<App />}>
      <Match when={location() === '/creator'}>
        <Creator />
      </Match>
    </Switch>
  );
};

render(() => <Router />, document.getElementById('app') as HTMLElement);

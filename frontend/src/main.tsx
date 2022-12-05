import { render } from 'preact';
import { App } from './app';
import { Creator } from './Creator';
import './main.css';
import { useEffect, useState } from 'preact/hooks';

const Router = () => {
  const [hash, setHash] = useState(window.location.hash);
  const location = hash.replace(/^#/, '') || '/';

  const onChange = () => setHash(window.location.hash);

  useEffect(() => {
    window.addEventListener('hashchange', onChange, false);
    return () => window.removeEventListener('hashchange', onChange, false);
  });

  if (location === '/creator') return <Creator />;

  return <App />;
};

render(<Router />, document.getElementById('app') as HTMLElement);

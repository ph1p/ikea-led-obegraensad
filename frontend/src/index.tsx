import { render } from "@solidjs/web";
import { Match, Switch } from "solid-js";
import "./index.css";

import { App } from "./app";
import { StoreProvider } from "./contexts/store";
import { Creator } from "./creator";
import { useHash } from "./hooks/useHash";
import Scheduler from "./scheduler";
import Settings from "./settings";

const Router = () => {
  const hash = useHash();
  const location = () => hash().replace(/^#/, "") || "/";

  return (
    <div class="h-full">
      <Switch fallback={<App />}>
        <Match when={location() === "/creator"}>
          <Creator />
        </Match>
        <Match when={location() === "/scheduler"}>
          <Scheduler />
        </Match>
        <Match when={location() === "/settings"}>
          <Settings />
        </Match>
      </Switch>
    </div>
  );
};

render(
  () => (
    <StoreProvider>
      <Router />
    </StoreProvider>
  ),
  document.getElementById("app") as HTMLElement,
);

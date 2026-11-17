import { makeEventListener } from "@solid-primitives/event-listener";
import { createSignal } from "solid-js";

/** Reactive `window.location.hash`, e.g. `"#/creator"` (empty on the root route). */
export function useHash() {
  const [hash, setHash] = createSignal(window.location.hash);
  makeEventListener(window, "hashchange", () => setHash(window.location.hash));
  return hash;
}

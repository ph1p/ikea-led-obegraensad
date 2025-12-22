import { createSignal } from "solid-js";

import { HISTORY_LIMIT, UNDO_REDO_COOLDOWN_MS } from "../constants";

export interface UseHistoryOptions {
  maxHistory?: number;
  cooldownMs?: number;
  onUndo?: () => void;
  onRedo?: () => void;
}

export function useHistory<T>(options: UseHistoryOptions = {}) {
  const {
    maxHistory = HISTORY_LIMIT,
    cooldownMs = UNDO_REDO_COOLDOWN_MS,
    onUndo,
    onRedo,
  } = options;

  const [history, setHistory] = createSignal<T[]>([]);
  const [historyIndex, setHistoryIndex] = createSignal(-1);
  const [isUndoRedoing, setIsUndoRedoing] = createSignal(false);

  const saveToHistory = (state: T) => {
    if (isUndoRedoing()) return;

    const currentHistory = history();
    const currentIndex = historyIndex();

    const newHistory = currentHistory.slice(0, currentIndex + 1);
    newHistory.push(state);

    if (newHistory.length > maxHistory) {
      newHistory.shift();
    } else {
      setHistoryIndex(currentIndex + 1);
    }

    setHistory(newHistory);
  };

  const undo = () => {
    const currentIndex = historyIndex();
    if (currentIndex <= 0) return;

    setIsUndoRedoing(true);
    setHistoryIndex(currentIndex - 1);

    setTimeout(() => setIsUndoRedoing(false), cooldownMs);

    onUndo?.();

    return history()[currentIndex - 1];
  };

  const redo = () => {
    const currentIndex = historyIndex();
    const historyLength = history().length;

    if (currentIndex >= historyLength - 1) return;

    setIsUndoRedoing(true);
    setHistoryIndex(currentIndex + 1);

    setTimeout(() => setIsUndoRedoing(false), cooldownMs);

    onRedo?.();

    return history()[currentIndex + 1];
  };

  const canUndo = () => historyIndex() > 0;
  const canRedo = () => historyIndex() < history().length - 1;

  return {
    saveToHistory,
    undo,
    redo,
    canUndo,
    canRedo,
    isUndoRedoing,
    history,
    historyIndex,
  };
}

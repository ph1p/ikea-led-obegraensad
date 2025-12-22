export interface KeyboardHandlers {
  onUndo: () => void;
  onRedo: () => void;
  onTogglePlay: () => void;
  onPreviousFrame?: () => void;
  onNextFrame?: () => void;
  onDeleteFrame?: () => void;
  onDuplicateFrame?: () => void;
}

export function shouldIgnoreKeyboard(e: KeyboardEvent): boolean {
  return e.target instanceof HTMLInputElement || e.target instanceof HTMLTextAreaElement;
}

export function handleUndoRedoShortcuts(
  e: KeyboardEvent,
  handlers: Pick<KeyboardHandlers, "onUndo" | "onRedo">,
): boolean {
  const isMac = navigator.userAgent.toUpperCase().indexOf("MAC") >= 0;
  const modKey = isMac ? e.metaKey : e.ctrlKey;

  if (modKey && (e.key === "z" || e.key === "Z")) {
    e.preventDefault();
    if (e.shiftKey) {
      handlers.onRedo();
    } else {
      handlers.onUndo();
    }
    return true;
  }

  if (modKey && (e.key === "y" || e.key === "Y")) {
    e.preventDefault();
    handlers.onRedo();
    return true;
  }

  return false;
}

export function handlePlaybackShortcuts(
  e: KeyboardEvent,
  handlers: Pick<KeyboardHandlers, "onTogglePlay">,
): boolean {
  if (e.key === " " || e.key === "Enter") {
    e.preventDefault();
    handlers.onTogglePlay();
    return true;
  }
  return false;
}

export function handleNavigationShortcuts(
  e: KeyboardEvent,
  handlers: Required<
    Pick<KeyboardHandlers, "onPreviousFrame" | "onNextFrame" | "onDeleteFrame" | "onDuplicateFrame">
  >,
): boolean {
  switch (e.key) {
    case "ArrowLeft":
      e.preventDefault();
      handlers.onPreviousFrame();
      return true;
    case "ArrowRight":
      e.preventDefault();
      handlers.onNextFrame();
      return true;
    case "Delete":
    case "Backspace":
      e.preventDefault();
      handlers.onDeleteFrame();
      return true;
    case "d":
    case "D":
      handlers.onDuplicateFrame();
      return true;
    default:
      return false;
  }
}

export function createKeyboardHandler(
  handlers: KeyboardHandlers,
  options: {
    hasFrames: () => boolean;
    isPlaying: () => boolean;
  },
) {
  return (e: KeyboardEvent) => {
    if (shouldIgnoreKeyboard(e)) return;

    if (handleUndoRedoShortcuts(e, handlers)) return;

    if (!options.hasFrames()) return;

    if (handlePlaybackShortcuts(e, handlers)) return;

    if (options.isPlaying()) return;

    if (
      handlers.onPreviousFrame &&
      handlers.onNextFrame &&
      handlers.onDeleteFrame &&
      handlers.onDuplicateFrame
    ) {
      handleNavigationShortcuts(e, {
        onPreviousFrame: handlers.onPreviousFrame,
        onNextFrame: handlers.onNextFrame,
        onDeleteFrame: handlers.onDeleteFrame,
        onDuplicateFrame: handlers.onDuplicateFrame,
      });
    }
  };
}

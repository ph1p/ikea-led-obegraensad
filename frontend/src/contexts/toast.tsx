import { createContext, createSignal, type ParentComponent, Show, useContext } from "solid-js";

import type { IToastContext } from "../types";

const Toast = (props: { text?: string }) => (
  <div
    role="status"
    class="card fixed left-1/2 bottom-[max(1.5rem,env(safe-area-inset-bottom))] z-100 flex items-center gap-2.5 whitespace-nowrap rounded-full px-4 py-2.5 text-sm animate-toast-slide-up"
  >
    <i class="fa-solid fa-circle-check text-accent" />
    <span>{props.text}</span>
  </div>
);

const [toastNotification, setToastNotification] = createSignal<{
  text: string;
  duration: number;
} | null>(null);

const toast = (text: string, duration: number) => {
  setToastNotification(null);

  setTimeout(() => {
    setToastNotification({
      text,
      duration,
    });

    setTimeout(() => setToastNotification(null), duration);
  }, 50);
};

const ToastContext = createContext<IToastContext>({
  toast,
});

export const ToastProvider: ParentComponent = (props) => {
  return (
    <ToastContext.Provider value={{ toast }}>
      <Show when={!!toastNotification()}>
        <Toast text={toastNotification()?.text} />
      </Show>
      {props.children}
    </ToastContext.Provider>
  );
};

export const useToast = () => useContext(ToastContext);

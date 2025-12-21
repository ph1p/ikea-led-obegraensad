import { createContext, createSignal, type ParentComponent, Show, useContext } from "solid-js";

import type { IToastContext } from "../types";

const Toast = (props: { text?: string }) => (
  <div class="fixed left-4 bottom-4 px-4 py-3 bg-gray-800 text-white shadow-lg border border-gray-700 flex items-center space-x-2 z-[100] animate-toast-slide-up rounded-md">
    <i class="fa-solid fa-info-circle text-blue-400 mr-2" />
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

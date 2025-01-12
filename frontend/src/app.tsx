import { Component, createMemo, Show } from 'solid-js';
import { Layout } from './components/layout/layout';
import Sidebar from './components/layout/sidebar';
import { LedMatrix } from './components/led-matrix';
import { ScreenInfo } from './components/screen-info';
import { useStore } from './contexts/store';
import { useToast } from './contexts/toast';
import { loadImageAndGetDataArray, rotateArray } from './helpers';

export const App: Component = () => {
  const [store, actions] = useStore();
  const { toast } = useToast();

  const rotatedMatrix = createMemo(() =>
    rotateArray(store.indexMatrix, store.rotation),
  );

  const wsMessage = (
    event:
      | 'persist'
      | 'load'
      | 'clear'
      | 'plugin'
      | 'screen'
      | 'led'
      | 'persist-plugin'
      | 'brightness',
    data?: any,
  ) =>
    actions.send(
      JSON.stringify({
        event,
        ...data,
      }),
    );

  const handleRotate = (turnRight = false) => {
    let currentRotation = store.rotation || 0;
    currentRotation = turnRight
      ? currentRotation > 3
        ? 1
        : currentRotation + 1
      : currentRotation <= 0
        ? 3
        : currentRotation - 1;

    actions.setRotation(currentRotation);
    actions.send(
      JSON.stringify({
        event: 'rotate',
        direction: turnRight ? 'right' : 'left',
      }),
    );
  };

  const handleLoadImage = () => {
    loadImageAndGetDataArray((data) => {
      actions.setLeds(
        store.indexMatrix.map((index) => (data[index] ? 255 : 0)),
      );
      wsMessage('screen', { data });
    });
  };

  const handleClear = () => {
    actions?.setLeds([...new Array(256).fill(0)]);
    wsMessage('clear');
    toast(`Canvas cleared`, 1000);
  };

  const handlePersist = () => {
    wsMessage('persist');
    toast(`Saved current state`, 1500);
  };

  const handleLoad = () => {
    wsMessage('load');
    toast(`Saved state loaded`, 1500);
  };

  const handlePluginChange = (pluginId: number) => {
    wsMessage('plugin', { plugin: pluginId });
    toast('Mode changed', 1000);
  };

  const handleBrightnessChange = (value: number) => {
    actions?.setBrightness(value);
    wsMessage('brightness', { brightness: value });
  };

  const handlePersistPlugin = () => {
    wsMessage('persist-plugin');
    toast(`Current mode set as default`, 1500);
  };

  const renderLedMatrix = () => (
    <div class="grid p-8 h-full justify-center items-center sm:p-4 sm:m-0">
      <Show
        when={store.plugin === 1 && !store.isActiveScheduler}
        fallback={
          <Show
            when={!store.isActiveScheduler}
            fallback={
              <ScreenInfo>
                <h2 class="text-4xl">Scheduler is running</h2>
              </ScreenInfo>
            }
          >
            <ScreenInfo>
              <h2 class="text-4xl">A Plugin currently running</h2>
              <p class="text-xl mt-2 text-gray-300">
                Select "Draw" to show the canvas.
              </p>
            </ScreenInfo>
          </Show>
        }
      >
        <div
          style={{
            opacity: (store.brightness || 255) / 255,
          }}
        >
          <LedMatrix
            disabled={store.plugin !== 1}
            data={store.leds || []}
            indexData={rotatedMatrix()}
            onSetLed={(data) => {
              wsMessage('led', data);
            }}
            onSetMatrix={(data) => {
              actions?.setLeds([...data]);
            }}
          />
        </div>
      </Show>
    </div>
  );

  return (
    <Layout
      content={renderLedMatrix()}
      sidebar={
        <Sidebar
          onRotate={handleRotate}
          onLoadImage={handleLoadImage}
          onClear={handleClear}
          onPersist={handlePersist}
          onLoad={handleLoad}
          onPluginChange={handlePluginChange}
          onBrightnessChange={handleBrightnessChange}
          onPersistPlugin={handlePersistPlugin}
        />
      }
    />
  );
};

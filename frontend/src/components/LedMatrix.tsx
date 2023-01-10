import { Component, createSignal, Index } from 'solid-js';
import { createVisibilityObserver } from '@solid-primitives/intersection-observer';
import { grid, ledInner, ledScreen, ledWrapper } from './LedMatrix.css';

interface Props {
  disabled?: boolean;
  onSetLed?: (data: { index: number; status: number }) => void;
  onSetMatrix?: (data: number[]) => void;
  data: number[];
  indexData: number[];
}

export const LedMatrix: Component<Props> = (props) => {
  let ref: HTMLDivElement | undefined;
  const useVisibilityObserver = createVisibilityObserver({ threshold: 0.9 });
  const visible = useVisibilityObserver(() => ref);
  const [isMouseDown, setMouseIsDown] = createSignal(false);

  const setLed = (index: number) => {
    const status = Number(!props.data[index]);
    const newState = props.data.map((led, i) =>
      i === index ? Number(status) : led
    );

    if (props.onSetLed) {
      props.onSetLed({
        index,
        status,
      });
    }
    if (props.onSetMatrix) {
      props.onSetMatrix(newState);
    }
    return newState;
  };

  return (
    <div
      ref={ref}
      class={`${ledScreen} ${visible() ? 'in-view' : ''} ${
        props.disabled ? 'disabled' : ''
      }`}
    >
      <div
        class={grid}
        onPointerUp={() => {
          setMouseIsDown(false);
        }}
        onPointerLeave={() => {
          setMouseIsDown(false);
        }}
      >
        <Index each={props.data}>
          {(_, index) => (
            <div
              class={ledWrapper}
              onPointerDown={() => {
                setLed(props.indexData[index]);
                setMouseIsDown(true);
              }}
              onPointerEnter={() => {
                if (isMouseDown()) {
                  setLed(props.indexData[index]);
                }
              }}
            >
              <div
                class={`${ledInner} ${
                  props.data[props.indexData[index]] ? 'active' : ''
                }`}
              ></div>
            </div>
          )}
        </Index>
      </div>
    </div>
  );
};

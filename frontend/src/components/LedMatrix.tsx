import { useState } from 'preact/hooks';
import { useInView } from 'react-intersection-observer';

import { grid, ledInner, ledScreen, ledWrapper } from './LedMatrix.css';

interface Props {
  disabled?: boolean;
  onSetLed?: (data: { index: number; status: number }) => void;
  onSetMatrix?: (data: number[]) => void;
  data: number[];
  indexData: number[];
}

export function LedMatrix(props: Props) {
  const { ref, inView } = useInView({
    threshold: 0.99,
  });
  const [isMouseDown, setMouseIsDown] = useState(false);

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
      className={`${ledScreen} ${inView ? 'in-view' : ''} ${
        props.disabled ? 'disabled' : ''
      }`}
    >
      <div
        className={grid}
        onPointerUp={() => {
          setMouseIsDown(false);
        }}
        onPointerLeave={() => {
          setMouseIsDown(false);
        }}
      >
        {props.data.map((_, index) => {
          const rIndex = props.indexData[index];
          return (
            <div
              key={rIndex}
              className={ledWrapper}
              onPointerDown={() => {
                setLed(rIndex);
                setMouseIsDown(true);
              }}
              onPointerEnter={() => {
                if (isMouseDown) {
                  setLed(rIndex);
                }
              }}
            >
              <div
                className={`${ledInner} ${props.data[rIndex] ? 'active' : ''}`}
              ></div>
            </div>
          );
        })}
      </div>
    </div>
  );
}

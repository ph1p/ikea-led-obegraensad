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
    const rotatedIndex = props.indexData[index];

    const status = Number(!props.data[index]);
    const newState = props.data.map((led, i) =>
      i === index ? Number(status) : led
    );

    if (props.onSetLed) {
      props.onSetLed({
        index: rotatedIndex,
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
        {props.data.map((status, index) => (
          <div
            key={index}
            className={ledWrapper}
            onPointerDown={() => {
              setLed(index);
              setMouseIsDown(true);
            }}
            onPointerEnter={() => {
              if (isMouseDown) {
                setLed(index);
              }
            }}
          >
            <div className={`${ledInner} ${status ? 'active' : ''}`}></div>
          </div>
        ))}
      </div>
    </div>
  );
}

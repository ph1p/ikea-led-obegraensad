import { useEffect, useRef, useState } from 'preact/hooks';
import { ReadyState } from 'react-use-websocket/dist/lib/constants';
import { useWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import { Button } from './components/Button';
import { Layout } from './components/Layout';
import { LedMatrix } from './components/LedMatrix';
import {
  emptyScreen,
  screenCount,
  screenHeader,
  screensWrapper,
  wrapper,
} from './Creator.css';
import { chunkArray, matrixToHexArray } from './helpers';
import { connectionInformation } from './main.css';

export function Creator() {
  const ref = useRef<HTMLDivElement>(null);
  const [isPlaying, setIsPlaying] = useState(false);
  const [indexMatrix] = useState<number[]>(
    [...new Array(256)].map((_, i) => i)
  );
  const [screens, setScreens] = useState<number[][]>([]);
  const [currentFrame, setCurrentFrame] = useState<number[]>([]);
  const [intervalId, setIntervalId] = useState(0);
  const { sendMessage, readyState } = useWebSocket(
    `${
      import.meta.env.PROD
        ? window.location.href.replace('http', 'ws')
        : import.meta.env.VITE_WS_URL
    }ws`,
    {
      share: true,
      shouldReconnect: () => true,
      reconnectAttempts: 10,
      reconnectInterval: 3000,
      retryOnError: true,
    }
  );

  const scrollToEnd = () => {
    if (ref.current) {
      setTimeout(() => {
        ref.current?.scrollTo({
          top: 0,
          left: ref.current?.scrollWidth,
          behavior: 'smooth',
        });
      }, 20);
    }
  };

  const addScreen = () => {
    setScreens((state) => {
      if (state.length > 0) {
        state.push([...state[state.length - 1]]);
      } else {
        state.push([...new Array(256)].fill(0));
      }

      scrollToEnd();
      return JSON.parse(JSON.stringify(state));
    });
  };

  const removeScreen = (index: number) => {
    setScreens((state) => state.filter((_, i) => i !== index));
  };
  const uploadData = () => {
    sendMessage(
      JSON.stringify({
        event: 'upload',
        screens: screens.length,
        data: screens.map((screen) => matrixToHexArray(screen)),
      })
    );
  };

  const exportData = () => {
    const animation = [];
    for (const screen of screens) {
      animation.push(
        chunkArray(screen, 8).map(
          (chunk) =>
            `0x${parseInt(chunk.join(''), 2).toString(16).padStart(2, '0')}`
        )
      );
    }

    const element = document.createElement('a');
    element.setAttribute(
      'href',
      'data:text/plain;charset=utf-8,' +
        encodeURIComponent(
          `std::vector<std::vector<int>> frames = ${JSON.stringify(animation)
            .replace(/"/g, '')
            .replace(/]/g, '}')
            .replace(/\[/g, '{')};

int size = frames.size();
int count = 0;
if (size > 0)
{
    std::vector<int> bits = Screen.readBytes(frames[count]);

    for (int i = 0; i < bits.size(); i++)
    {
        Screen.setPixelAtIndex(i, bits[i]);
    }

    Screen.render();
    count++;

    if (count >= size)
    {
        count = 0;
    }
    delay(400);
}`
        )
    );
    element.setAttribute('download', 'animation.cpp');
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    element.remove();
  };

  useEffect(() => {
    if (!isPlaying) {
      clearInterval(intervalId);
      setIntervalId(0);
      scrollToEnd();
      return;
    }

    let i = 0;
    const run = () => {
      setCurrentFrame([...screens[i]]);
      i++;
      if (i >= screens.length) {
        i = 0;
      }
    };
    run();
    const newIntervalId: any = setInterval(run, 400);
    setInterval(newIntervalId);

    return () => {
      clearInterval(newIntervalId);
      setIntervalId(0);
      scrollToEnd();
    };
  }, [isPlaying]);

  const connectionStatus = {
    [ReadyState.CONNECTING]: 'Connecting',
    [ReadyState.OPEN]: 'Open',
    [ReadyState.CLOSING]: 'Closing',
    [ReadyState.CLOSED]: 'Try to reconnect',
    [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
  }[readyState];

  if (readyState !== ReadyState.OPEN) {
    return (
      <Layout
        content={
          <div className={wrapper}>
            <div className={connectionInformation}>{connectionStatus}...</div>
          </div>
        }
        footer={
          <>
            <a href="#/">back</a>
          </>
        }
      ></Layout>
    );
  }

  return (
    <Layout
      content={
        <div class={wrapper}>
          {screens.length ? (
            <div class={screensWrapper} ref={ref}>
              {isPlaying ? (
                <LedMatrix data={currentFrame} indexData={indexMatrix} />
              ) : (
                screens.map((screen, index) => (
                  <div>
                    <header className={screenHeader}>
                      <Button onClick={() => removeScreen(index)}>
                        <i class="fa-solid fa-trash"></i>
                      </Button>
                      <div className={screenCount}>
                        {index + 1}
                        <span>/{screens.length}</span>
                      </div>
                    </header>
                    <LedMatrix
                      key={index}
                      data={screen}
                      indexData={indexMatrix}
                      onSetLed={(data) => {
                        setScreens((state) => {
                          state[index][data.index] = Number(data.status);
                          return JSON.parse(JSON.stringify(state));
                        });
                      }}
                      onSetMatrix={(data) => {
                        setScreens((state) =>
                          state.map((s, i) => (i === index ? [...data] : s))
                        );
                      }}
                    />
                  </div>
                ))
              )}
            </div>
          ) : (
            <div class={emptyScreen}>Create something awesome! 🙌</div>
          )}
        </div>
      }
      footer={
        <>
          {!isPlaying && (
            <>
              <Button onClick={addScreen}>
                <i class="fa-solid fa-plus"></i>
              </Button>
              <Button onClick={exportData}>
                <i class="fa-solid fa-download"></i>
              </Button>
              <Button onClick={uploadData}>
                <i class="fa-solid fa-upload"></i>
              </Button>
            </>
          )}

          {screens.length > 0 && (
            <Button onClick={() => setIsPlaying(!isPlaying)}>
              <i class={`fa-solid fa-${isPlaying ? 'stop' : 'play'}`}></i>
            </Button>
          )}

          {!isPlaying && <a href="#/">back</a>}
        </>
      }
    />
  );
}

import { Component, JSX, Show } from "solid-js";
import { Button } from "./button";
import { Tooltip } from "./tooltip";

interface CreatorSidebarProps {
  hasScreens: boolean;
  isPlaying: boolean;
  onAddScreen: () => void;
  onExportData: () => void;
  onUploadData: () => void;
  onTogglePlay: () => void;
}

export const CreatorSidebar: Component<CreatorSidebarProps> = (props) => {
  return (
    <aside class="bg-white p-6 flex flex-col h-full">
      <div class="space-y-6">
        <div class="space-y-3">
          <h3 class="text-sm font-semibold text-gray-700 uppercase tracking-wide">
            Controls
          </h3>
          <div class="flex gap-4 items-center">
            <Show
              when={!props.isPlaying}
              fallback={
                <Tooltip text="Stop animation">
                  <Button
                    disabled={!props.hasScreens}
                    onClick={props.onTogglePlay}
                    class="hover:bg-red-600 transition-colors"
                  >
                    <i class="fa-solid fa-stop" />
                  </Button>
                </Tooltip>
              }
            >
              <Tooltip text="Add new frame">
                <Button
                  onClick={props.onAddScreen}
                  class="hover:bg-green-600 transition-colors"
                >
                  <i class="fa-solid fa-plus" />
                </Button>
              </Tooltip>

              <Tooltip text="Export animation">
                <Button
                  disabled={!props.hasScreens}
                  onClick={props.onExportData}
                  class="hover:bg-blue-700 transition-colors"
                >
                  <i class="fa-solid fa-download" />
                </Button>
              </Tooltip>

              <Tooltip text="Upload to device">
                <Button
                  disabled={!props.hasScreens}
                  onClick={props.onUploadData}
                  class="hover:bg-blue-700 transition-colors"
                >
                  <i class="fa-solid fa-upload" />
                </Button>
              </Tooltip>

              <Tooltip text="Play animation">
                <Button
                  disabled={!props.hasScreens}
                  onClick={props.onTogglePlay}
                  class="hover:bg-green-600 transition-colors"
                >
                  <i class="fa-solid fa-play" />
                </Button>
              </Tooltip>
            </Show>
          </div>
        </div>
      </div>

      <div class="mt-auto pt-6 border-t border-gray-200">
        <Tooltip text="Return to main editor">
          <a
            href="#/"
            class="inline-flex items-center text-gray-700 hover:text-gray-900 font-medium"
          >
            <i class="fa-solid fa-arrow-left mr-2" />
            Back
          </a>
        </Tooltip>
      </div>
    </aside>
  );
};

export default CreatorSidebar;

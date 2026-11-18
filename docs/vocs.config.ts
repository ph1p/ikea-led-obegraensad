import { defineConfig } from "vocs/config";

const repo = "https://github.com/ph1p/ikea-led-obegraensad";

export default defineConfig({
  title: "OBEGRÄNSAD LED",
  titleTemplate: "%s · OBEGRÄNSAD LED",
  description:
    "Open firmware that turns the IKEA OBEGRÄNSAD LED wall lamp into a Wi-Fi controlled 16×16 pixel matrix.",
  basePath: "/ikea-led-obegraensad",
  srcDir: ".",
  renderStrategy: "full-static",
  editLink: {
    link: `${repo}/edit/main/docs/pages/:path`,
    text: "Edit this page on GitHub",
  },
  socials: [{ icon: "github", link: repo }],
  topNav: [
    { text: "Guide", link: "/getting-started/hardware", match: "/getting-started" },
    { text: "Home Assistant", link: "/integrations/home-assistant" },
    { text: "API", link: "/reference/http-api", match: "/reference" },
    { text: "Releases", link: `${repo}/releases` },
  ],
  sidebar: [
    {
      text: "Getting started",
      items: [
        { text: "Hardware", link: "/getting-started/hardware" },
        { text: "Flashing", link: "/getting-started/flashing" },
        { text: "Wi-Fi setup", link: "/getting-started/wifi" },
        { text: "OTA updates", link: "/getting-started/ota" },
      ],
    },
    {
      text: "Usage",
      items: [
        { text: "Web GUI", link: "/usage/web-gui" },
        { text: "Plugins", link: "/usage/plugins" },
        { text: "Plugin scheduler", link: "/usage/scheduler" },
        { text: "Brightness", link: "/usage/brightness" },
        { text: "Messages", link: "/usage/messages" },
      ],
    },
    {
      text: "Integrations",
      items: [
        { text: "Home Assistant", link: "/integrations/home-assistant" },
        { text: "MQTT", link: "/integrations/mqtt" },
        { text: "DDP", link: "/integrations/ddp" },
        { text: "Art-Net", link: "/integrations/artnet" },
        { text: "Python scripts", link: "/integrations/python" },
      ],
    },
    {
      text: "Reference",
      items: [
        { text: "HTTP API", link: "/reference/http-api" },
        { text: "WebSocket", link: "/reference/websocket" },
        { text: "Configuration", link: "/reference/configuration" },
      ],
    },
    {
      text: "Development",
      items: [
        { text: "Frontend", link: "/development/frontend" },
        { text: "Writing a plugin", link: "/development/writing-a-plugin" },
        { text: "Contributing", link: "/development/contributing" },
      ],
    },
    {
      text: "Troubleshooting",
      link: "/troubleshooting",
    },
  ],
});

import { defineConfig } from 'vite';
import solidPlugin from 'vite-plugin-solid';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [
    solidPlugin(),
    viteSingleFile()
  ],
  server: {
    port: 3000,
  },
  build: {
    target: 'esnext',
  },
});

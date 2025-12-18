import { defineConfig } from 'vite';
import tailwindcss from '@tailwindcss/vite';
import solid from 'vite-plugin-solid';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [tailwindcss(), solid(), viteSingleFile()],
  server: {
    port: 3000,
  },
  build: {
    target: 'esnext',
  },
});

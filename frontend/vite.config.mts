import { defineConfig } from 'vite';
import solidPlugin from 'vite-plugin-solid';
import tailwindcss from '@tailwindcss/vite';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [tailwindcss(), solidPlugin(), viteSingleFile()],
  server: {
    port: 3000,
  },
  build: {
    target: 'esnext',
  },
});

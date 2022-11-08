import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';
import { viteSingleFile } from 'vite-plugin-singlefile';
import { vanillaExtractPlugin } from '@vanilla-extract/vite-plugin';

export default defineConfig({
  plugins: [
    preact(),
    viteSingleFile(),
    vanillaExtractPlugin({
      identifiers: 'short',
    }),
  ],
  build: {
    minify: 'terser',
    terserOptions: {
      compress: {
        drop_console: true,
        ecma: 2020,
        passes: 5,
      },
    },
  },
});

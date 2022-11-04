// import { defineConfig } from 'vite';
// import preact from '@preact/preset-vite';
// import cssInjectedByJsPlugin from 'vite-plugin-css-injected-by-js';

// export default defineConfig({
//   plugins: [preact(), cssInjectedByJsPlugin()],
//   build: {
//     rollupOptions: {
//       output: {
//         manualChunks: undefined,
//       },
//     },
//   },
// });

import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [preact(), viteSingleFile()],
  build: {
    minify: 'terser',
    terserOptions: {
      compress: {
        drop_console: true,
        ecma: 2020,
        passes: 5
      },
    },
  },
});

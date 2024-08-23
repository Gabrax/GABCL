import { build, defineConfig } from "vite";
import path from 'path';
// vite.config.js
export default {
    build: {
        assetsInlineLimit: 0,
        assetsInclude: ['**/*.mp3', '**/*.jpg', '**/*.png'],
        rollupOptions: {
            input: {
                main: path.resolve(__dirname,'index.html'),
            },
        }
    },
  }

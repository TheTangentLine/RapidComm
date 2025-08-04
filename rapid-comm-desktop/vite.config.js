import { defineConfig } from "vite";

// https://vitejs.dev/config/
export default defineConfig(({ command }) => {
  const isDev = command === "serve";
  return {
  // Set the root to the `src` directory
  base: isDev ? "/" : "./",
  root: "src",
  // prevent vite from obscuring rust errors
  clearScreen: false,
  // Tauri expects a fixed port, fail if that port is not available
  server: {
    port: 1420,
    strictPort: true,
  },
  build: {
    // Tauri uses this directory for production builds so we configure it correctly
    outDir: "../dist",
  },
  };
}); 
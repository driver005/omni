import {defineConfig, devices} from '@playwright/test';

export default defineConfig({
    testDir: './e2e',
    timeout: 30_000,
    use: {
        baseURL: 'http://127.0.0.1:5173',
        ...devices['Desktop Chrome'],
        launchOptions: {
            executablePath: '/home/default/.cache/ms-playwright/chromium_headless_shell-1255/chrome-headless-shell-linux64/chrome-headless-shell',
        },
    },
    webServer: {
        command: 'pnpm exec vite --config vite.e2e.config.ts',
        url: 'http://127.0.0.1:5173/theme-e2e/index.html',
        reuseExistingServer: true,
        timeout: 60_000,
    },
});

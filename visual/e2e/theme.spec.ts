import {expect, test} from '@playwright/test';

async function stubThemeRoute(page: import('@playwright/test').Page, preference: string, onPut?: (url: string) => void) {
    await page.route('**/preferences/theme*', async (route) => {
        if (route.request().method() === 'GET') {
            await route.fulfill({
                status: 200,
                contentType: 'application/json',
                body: JSON.stringify(preference),
            });
            return;
        }
        onPut?.(route.request().url());
        await route.fulfill({status: 200, contentType: 'application/json', body: '{}'});
    });
}

test('loads the server-owned preference through the generated SDK route', async ({page}) => {
    await stubThemeRoute(page, 'dark');

    await page.goto('/theme-e2e/index.html');
    await expect(page.locator('#status')).toHaveText('ready');
    await expect(page.locator('html')).toHaveClass(/dark/);
});

test('setTheme writes the generated route and flips the DOM optimistically', async ({page}) => {
    let putUrl = '';
    await stubThemeRoute(page, 'light', (url) => {
        putUrl = url;
    });

    await page.goto('/theme-e2e/index.html');
    await expect(page.locator('#status')).toHaveText('ready');

    await page.evaluate(async () => {
        await (window as unknown as {__theme: {setTheme(mode: string): Promise<void>}}).__theme.setTheme('dark');
    });

    await expect(page.locator('html')).toHaveClass(/dark/);
    expect(putUrl).toContain('theme=dark');
});

test('the generated posts client completes a request', async ({page}) => {
    await stubThemeRoute(page, 'light');
    await page.route('**/posts*', async (route) => {
        await route.fulfill({status: 200, contentType: 'application/json', body: 'null'});
    });

    await page.goto('/theme-e2e/index.html');
    await expect(page.locator('#status')).toHaveText('ready');

    const status = await page.evaluate(async () => {
        const sdk = (window as unknown as {__sdk: {listPosts(): Promise<{status: number}>}}).__sdk;
        return (await sdk.listPosts()).status;
    });
    expect(status).toBe(200);
});

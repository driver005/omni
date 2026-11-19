import {test as baseTest, afterEach, beforeEach, describe, expect, vi} from 'vitest';
import {renderHook, waitFor, act} from '@testing-library/react';
import type {QueryClient} from '@tanstack/react-query';
import {useTheme} from './use-theme';
import {queryClientFixtures, type TestWrapperComponent} from '@test-utils/fixtures/query-client';

const sdkState = vi.hoisted(() => ({
  preference: 'light' as string,
  setPreference: vi.fn(async (_mode: string): Promise<void> => {}),
}));

vi.mock('../../../../packages/congelado-sdk/pkg/sdk_bindings.js', () => ({
  default: async () => {},
  Sdk: class {},
  resolveTheme: (preference: string, system: string) =>
    preference === 'dark' || (preference === 'system' && system === 'dark') ? 'dark' : 'light',
}));

vi.mock('@tryghost/congelado-sdk', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@tryghost/congelado-sdk')>();
  return {
    ...actual,
    CongeladoSdk: {
      create: async () => ({
        getThemePreference: async () => sdkState.preference,
        setThemePreference: sdkState.setPreference,
      }),
    },
    resolveTheme: (preference: string, system: string) =>
      preference === 'dark' || (preference === 'system' && system === 'dark') ? 'dark' : 'light',
  };
});

const themeTest = baseTest.extend<{
  queryClient: QueryClient;
  wrapper: TestWrapperComponent;
  animationFrames: Map<number, FrameRequestCallback>;
}>({
  ...queryClientFixtures,
  animationFrames: async ({task}, provide) => {
    void task;
    const callbacks = new Map<number, FrameRequestCallback>();
    let nextFrameId = 1;
    const requestAnimationFrameSpy = vi
      .spyOn(window, 'requestAnimationFrame')
      .mockImplementation((callback) => {
        const frameId = nextFrameId;
        nextFrameId += 1;
        callbacks.set(frameId, callback);
        return frameId;
      });
    const cancelAnimationFrameSpy = vi
      .spyOn(window, 'cancelAnimationFrame')
      .mockImplementation((frameId) => {
        callbacks.delete(frameId);
      });
    await provide(callbacks);
    requestAnimationFrameSpy.mockRestore();
    cancelAnimationFrameSpy.mockRestore();
  },
});

function flushAnimationFrames(callbacks: Map<number, FrameRequestCallback>) {
  while (callbacks.size > 0) {
    const [frameId, callback] = callbacks.entries().next().value ?? [];
    if (frameId !== undefined && callback) {
      callbacks.delete(frameId);
      callback(0);
    }
  }
}

beforeEach(() => {
  sdkState.preference = 'light';
  sdkState.setPreference.mockReset();
  sdkState.setPreference.mockImplementation(async () => {});
});

afterEach(() => {
  document.documentElement.classList.remove('dark', 'theme-switching');
});

describe('useTheme (standalone)', () => {
  themeTest(
    'does not report a ready theme until the preference has loaded',
    async ({wrapper, animationFrames}) => {
      sdkState.preference = 'dark';
      const {result} = renderHook(() => useTheme(), {wrapper});

      expect(result.current.isThemeReady).toBe(false);
      await waitFor(() => expect(result.current.isThemeReady).toBe(true));
      expect(result.current.resolvedTheme).toBe('dark');
      flushAnimationFrames(animationFrames);
    },
  );

  themeTest(
    'applies the persisted theme with transition suppression',
    async ({wrapper, animationFrames}) => {
      sdkState.preference = 'dark';

      const {result} = renderHook(() => useTheme(), {wrapper});

      await waitFor(() => {
        expect(result.current.resolvedTheme).toBe('dark');
      });

      const html = document.documentElement;
      expect(html.classList.contains('dark')).toBe(true);
      expect(html.classList.contains('theme-switching')).toBe(true);
      expect(animationFrames.size).toBe(1);

      flushAnimationFrames(animationFrames);

      expect(html.classList.contains('dark')).toBe(true);
      expect(html.classList.contains('theme-switching')).toBe(false);
    },
  );

  themeTest(
    'suppresses transitions while switching theme, then releases',
    async ({wrapper, animationFrames}) => {
      const {result} = renderHook(() => useTheme(), {wrapper});

      await waitFor(() => expect(result.current.isThemeReady).toBe(true));
      flushAnimationFrames(animationFrames);

      const html = document.documentElement;
      expect(html.classList.contains('dark')).toBe(false);
      expect(html.classList.contains('theme-switching')).toBe(false);

      await act(async () => {
        await result.current.setTheme('dark');
      });

      expect(html.classList.contains('dark')).toBe(true);
      expect(html.classList.contains('theme-switching')).toBe(true);
      expect(sdkState.setPreference).toHaveBeenCalledWith('dark');

      flushAnimationFrames(animationFrames);

      expect(html.classList.contains('dark')).toBe(true);
      expect(html.classList.contains('theme-switching')).toBe(false);
    },
  );
});

describe('useTheme save failures', () => {
  themeTest(
    'rolls back a failed save to the current system appearance',
    async ({wrapper, animationFrames}) => {
      sdkState.preference = 'system';
      const mediaQuery = Object.assign(new EventTarget(), {matches: false}) as MediaQueryList;
      const mediaSpy = vi.spyOn(window, 'matchMedia').mockReturnValue(mediaQuery);
      const errorSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
      let finish: () => void = () => {};
      const gate = new Promise<void>((resolve) => {
        finish = resolve;
      });
      sdkState.setPreference.mockImplementation(async () => {
        await gate;
        throw new Error('Save failed');
      });
      try {
        const {result} = renderHook(() => useTheme(), {wrapper});
        await waitFor(() => expect(result.current.theme).toBe('system'));
        let saving: Promise<void> = Promise.resolve();
        act(() => {
          saving = result.current.setTheme('dark');
        });
        await waitFor(() => expect(result.current.theme).toBe('dark'));
        Object.defineProperty(mediaQuery, 'matches', {value: true});
        finish();
        await act(async () => {
          await saving;
        });
        expect(result.current.theme).toBe('system');
        expect(result.current.resolvedTheme).toBe('dark');
        expect(document.documentElement.classList.contains('dark')).toBe(true);
      } finally {
        finish();
        mediaSpy.mockRestore();
        errorSpy.mockRestore();
        flushAnimationFrames(animationFrames);
      }
    },
  );
});

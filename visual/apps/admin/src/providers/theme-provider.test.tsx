import {test as baseTest, afterEach, beforeEach, describe, expect, vi} from 'vitest';
import {render, renderHook, screen, waitFor, act} from '@testing-library/react';
import type {ReactNode} from 'react';
import type {QueryClient} from '@tanstack/react-query';
import {queryClientFixtures, type TestWrapperComponent} from '@test-utils/fixtures/query-client';
import {ThemeProvider} from './theme-provider';
import {useThemeContext} from './theme-context';

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

const themeContextTest = baseTest.extend<{
  queryClient: QueryClient;
  wrapper: TestWrapperComponent;
}>({
  ...queryClientFixtures,
});

beforeEach(() => {
  sdkState.preference = 'light';
  sdkState.setPreference.mockReset();
  sdkState.setPreference.mockImplementation(async () => {});
});

afterEach(() => {
  document.documentElement.classList.remove('dark', 'theme-switching');
});

describe('ThemeProvider', () => {
  themeContextTest(
    'shares one optimistic theme state across separate consumers mid-save',
    async ({wrapper: Wrapper}) => {
      // Hold the SDK write open so the optimistic window is observable.
      let releasePut: () => void = () => {};
      const putGate = new Promise<void>((resolve) => {
        releasePut = resolve;
      });
      sdkState.setPreference.mockImplementation(async () => {
        await putGate;
      });

      function Shell() {
        const {resolvedTheme} = useThemeContext();
        return <div data-testid="shell-theme">{resolvedTheme}</div>;
      }
      let setThemeFromMenu: (mode: 'dark' | 'light' | 'system') => Promise<void> = async () => {};
      function Menu() {
        const {setTheme, isSettingTheme} = useThemeContext();
        setThemeFromMenu = setTheme;
        return <div data-testid="menu-saving">{String(isSettingTheme)}</div>;
      }

      render(
        <Wrapper>
          <ThemeProvider>
            <Shell />
            <Menu />
          </ThemeProvider>
        </Wrapper>,
      );

      await waitFor(() => {
        expect(screen.getByTestId('shell-theme').textContent).toBe('light');
      });

      let pendingSet: Promise<void> = Promise.resolve();
      act(() => {
        pendingSet = setThemeFromMenu('dark');
      });

      await waitFor(() => {
        expect(screen.getByTestId('shell-theme').textContent).toBe('dark');
      });
      expect(screen.getByTestId('menu-saving').textContent).toBe('true');

      releasePut();
      await act(async () => {
        await pendingSet;
      });

      expect(screen.getByTestId('shell-theme').textContent).toBe('dark');
      await waitFor(() => {
        expect(screen.getByTestId('menu-saving').textContent).toBe('false');
      });
    },
  );

  themeContextTest(
    'shares one theme state: a menu setTheme flips every consumer',
    async ({wrapper: Wrapper}) => {
      const contextWrapper = ({children}: {children: ReactNode}) => (
        <Wrapper>
          <ThemeProvider>{children}</ThemeProvider>
        </Wrapper>
      );

      const {result} = renderHook(
        () => ({
          shell: useThemeContext(),
          menu: useThemeContext(),
        }),
        {wrapper: contextWrapper},
      );

      await waitFor(() => {
        expect(result.current.shell.isThemeReady).toBe(true);
      });
      expect(result.current.shell.resolvedTheme).toBe('light');

      await act(async () => {
        await result.current.menu.setTheme('dark');
      });

      expect(result.current.shell.theme).toBe('dark');
      expect(result.current.shell.resolvedTheme).toBe('dark');
    },
  );
});

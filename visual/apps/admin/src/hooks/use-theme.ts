import {useCallback, useEffect, useRef, useState} from 'react';
import {
  CongeladoSdk,
  createThemeController,
  type ThemeController,
  type ThemeMode,
  type ResolvedTheme,
} from '@tryghost/congelado-sdk';

export type {ThemeMode, ResolvedTheme};

/**
 * Admin's appearance hook. Preference storage and mode resolution live in the Rust SDK;
 * this hook only owns React state and the controller lifecycle.
 */
export function useTheme() {
  const [controller, setController] = useState<ThemeController>();
  const [resolvedTheme, setResolvedTheme] = useState<ResolvedTheme>('light');
  const [pendingTheme, setPendingTheme] = useState<ThemeMode | null>(null);
  const [isSettingTheme, setIsSettingTheme] = useState(false);
  const pendingRef = useRef(false);

  useEffect(() => {
    let disposed = false;
    let active: ThemeController | undefined;

    async function start() {
      const sdk = await CongeladoSdk.create(window.location.origin);
      if (disposed) {
        return;
      }
      active = createThemeController(sdk, setResolvedTheme);
      await active.init();
      if (!disposed) {
        setController(active);
      }
    }

    void start().catch((error: unknown) => {
      // eslint-disable-next-line no-console
      console.error('[Theme] Failed to initialize appearance preference:', error);
    });

    return () => {
      disposed = true;
      active?.destroy();
    };
  }, []);

  const theme: ThemeMode = pendingTheme ?? controller?.theme ?? 'light';

  useEffect(() => {
    if (pendingTheme !== null && controller?.theme === pendingTheme) {
      setPendingTheme(null);
    }
  }, [pendingTheme, controller]);

  const setTheme = useCallback(
    async (mode: ThemeMode) => {
      if (!controller || pendingRef.current || mode === theme) {
        return;
      }
      pendingRef.current = true;
      setIsSettingTheme(true);
      setPendingTheme(mode);
      try {
        await controller.setTheme(mode);
      } catch (error) {
        setPendingTheme(null);
        // eslint-disable-next-line no-console
        console.error('[Theme] Failed to update appearance preference:', error);
      } finally {
        pendingRef.current = false;
        setIsSettingTheme(false);
      }
    },
    [controller, theme],
  );

  return {
    theme,
    resolvedTheme,
    isThemeReady: controller !== undefined,
    setTheme,
    isSettingTheme,
  } as const;
}

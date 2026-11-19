/// <reference path="./wasm.d.ts" />
/**
 * Browser-facing wrapper over the Rust SDK.
 *
 * The glue in `pkg/` is produced by `make sdk-wasm`, which compiles `sdk/bindings` for
 * `wasm32-unknown-unknown` and runs `wasm-bindgen --target web`. All request construction,
 * business decisions, and theme-mode resolution live in Rust; this module only loads the
 * module and projects its state onto the DOM.
 */
import initWasm, {resolveTheme, Sdk as WasmSdk} from '../pkg/sdk_bindings.js';
import wasmUrl from '../pkg/sdk_bindings_bg.wasm?url';

export type ThemeMode = 'light' | 'dark' | 'system';
export type ResolvedTheme = 'light' | 'dark';

export interface PostListResult {
    status: number;
}

let initPromise: Promise<unknown> | undefined;

function ensureWasm(): Promise<unknown> {
    if (!initPromise) {
        initPromise = initWasm({module_or_path: wasmUrl});
    }
    return initPromise;
}

/** Client handle backed by the Rust SDK. */
export class CongeladoSdk {
    private readonly handle: WasmSdk;

    private constructor(handle: WasmSdk) {
        this.handle = handle;
    }

    static async create(baseUrl: string): Promise<CongeladoSdk> {
        await ensureWasm();
        return new CongeladoSdk(new WasmSdk(baseUrl));
    }

    get baseUrl(): string {
        return this.handle.baseUrl();
    }

    async listPosts(): Promise<PostListResult> {
        return JSON.parse(await this.handle.listPosts()) as PostListResult;
    }

    async getThemePreference(): Promise<ThemeMode> {
        const value = String(await this.handle.getThemePreference());
        return value === 'dark' || value === 'system' ? value : 'light';
    }

    async setThemePreference(mode: ThemeMode): Promise<void> {
        await this.handle.setThemePreference(mode);
    }
}

export interface ThemeController {
    readonly theme: ThemeMode;
    readonly resolvedTheme: ResolvedTheme;
    init(): Promise<void>;
    setTheme(mode: ThemeMode): Promise<void>;
    destroy(): void;
}

/**
 * Ghost Admin-style theme controller. Preference lives on the server via the generated SDK
 * route; the DOM class is a projection. Mirrors `apps/admin-x-framework/utils/admin-theme.ts`.
 */
export function createThemeController(sdk: CongeladoSdk, onChange?: (theme: ResolvedTheme) => void): ThemeController {
    let theme: ThemeMode = 'light';
    let resolvedTheme: ResolvedTheme = 'light';
    let disposed = false;
    let media: MediaQueryList | undefined;
    let releaseFrame: number | undefined;

    const systemAppearance = (): string => (media?.matches ? 'dark' : 'light');

    const apply = (next: ResolvedTheme): void => {
        resolvedTheme = next;
        const html = document.documentElement;
        if (html.classList.contains('dark') === (next === 'dark')) {
            onChange?.(next);
            return;
        }
        html.classList.add('theme-switching');
        html.classList.toggle('dark', next === 'dark');
        onChange?.(next);
        if (releaseFrame !== undefined) {
            cancelAnimationFrame(releaseFrame);
        }
        releaseFrame = requestAnimationFrame(() => {
            releaseFrame = requestAnimationFrame(() => {
                html.classList.remove('theme-switching');
                releaseFrame = undefined;
            });
        });
    };

    const render = (): void => {
        apply(resolveTheme(theme, systemAppearance()) as ResolvedTheme);
    };

    const handleSystemChange = (): void => {
        if (!disposed && theme === 'system') {
            render();
        }
    };

    return {
        get theme() {
            return theme;
        },
        get resolvedTheme() {
            return resolvedTheme;
        },
        async init() {
            theme = await sdk.getThemePreference();
            if (window.matchMedia) {
                media = window.matchMedia('(prefers-color-scheme: dark)');
                media.addEventListener('change', handleSystemChange);
            }
            render();
        },
        async setTheme(mode) {
            if (disposed || mode === theme) {
                return;
            }
            const previous = theme;
            theme = mode;
            render();
            try {
                await sdk.setThemePreference(mode);
            } catch (error) {
                theme = previous;
                render();
                throw error;
            }
        },
        destroy() {
            disposed = true;
            media?.removeEventListener('change', handleSystemChange);
            if (releaseFrame !== undefined) {
                cancelAnimationFrame(releaseFrame);
            }
        },
    };
}

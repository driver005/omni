import { defineConfig } from 'vitest/config';
import path from 'node:path';

/**
 * Runs the e2e-api suites against the C++ API server, with no Ghost boot and
 * no database. Snapshots are kept in a separate directory so the original
 * Ghost snapshots remain the parity oracle.
 */
export default defineConfig({
    resolve: {
        alias: [
            {
                find: /utils\/e2e-framework$/,
                replacement: path.resolve(__dirname, 'utils/cpp-e2e-framework.ts'),
            },
            {
                find: /core\/shared\/config$/,
                replacement: path.resolve(__dirname, 'utils/cpp-config-shim.js'),
            },
            {
                find: /utils\/host-limits-utils$/,
                replacement: path.resolve(__dirname, 'utils/cpp-host-limits-utils.js'),
            },
            {
                find: /utils\/assertions$/,
                replacement: path.resolve(__dirname, 'utils/assertions.js'),
            },
            {
                find: /utils\/config-utils$/,
                replacement: path.resolve(__dirname, 'utils/config-utils.js'),
            },
            {
                find: /utils\/e2e-framework-mock-manager$/,
                replacement: path.resolve(__dirname, 'utils/cpp-mock-manager.js'),
            },
        ],
    },
    test: {
        globals: true,
        environment: 'node',
        include: ['test/e2e-api/**/*.test.{js,ts}'],
        pool: 'forks',
        isolate: false,
        maxWorkers: 2,
        hookTimeout: 60000,
        testTimeout: 30000,
        resolveSnapshotPath: (testPath, snapExt) =>
            path.join(
                path.dirname(testPath),
                '__snapshots__-cpp',
                path.basename(testPath) + snapExt
            ),
    },
});

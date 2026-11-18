const assert = require('node:assert/strict');
const expressTest = require('@tryghost/express-test');

const assertExists = (value, message = 'Value should exist') =>
    assert(value !== undefined && value !== null, message);
const assertMatchSnapshot = (obj, properties) => {
    if (!expressTest.snapshot.snapshotManager) return;
    const result = expressTest.snapshot.snapshotManager.match(obj, properties);
    if (result && !result.pass) assert.fail(result.message());
};
const assertArrayContainsDeep = (arr, expected, message) => {
    for (const element of expected) {
        assert(arr.some(el => require('node:util').isDeepStrictEqual(el, element)), message);
    }
};
const assertArrayMatchesWithoutOrder = (arr, expected, message) => assertArrayContainsDeep(arr, expected, message);
const assertObjectMatches = (obj, expected, message) => {
    for (const [key, value] of Object.entries(expected)) {
        assert(require('node:util').isDeepStrictEqual(obj[key], value), message || key);
    }
};

module.exports = {
    assertExists,
    assertMatchSnapshot,
    assertArrayContainsDeep,
    assertArrayMatchesWithoutOrder,
    assertObjectMatches,
    cacheInvalidateHeaderNotSet: () => true,
    cacheInvalidateHeaderSet: () => true,
};

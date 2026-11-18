// Generic deep stub for Ghost core modules referenced directly by e2e tests.
// Under the C++ runner none of these modules are loaded: all traffic goes to
// the C++ API server over HTTP, so their only role is to satisfy imports and
// accept method calls.
const noop = () => {};

function makeStub(name = 'ghost') {
    const fn = function stub() {
        return makeStub(name);
    };
    const proxy = new Proxy(fn, {
        get(target, prop) {
            if (prop === 'then') return undefined;
            if (prop === Symbol.toPrimitive || prop === 'toString') {
                return () => name;
            }
            if (prop === 'default') return proxy;
            if (!(prop in target)) target[prop] = makeStub(`${name}.${String(prop)}`);
            return target[prop];
        },
        apply() {
            return makeStub(name);
        },
        construct() {
            return makeStub(name);
        },
    });
    return proxy;
}

const stubs = new Map();
const ghostStub = request => {
    if (!stubs.has(request)) stubs.set(request, makeStub(request));
    return stubs.get(request);
};

module.exports = {makeStub, ghostStub, noop};

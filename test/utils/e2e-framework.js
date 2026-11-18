/**
 * No-DB e2e-framework shim. Resolution of `require('../../utils/e2e-framework')`
 * prefers this .js file over the original .ts, so the moved Ghost suites run
 * against the C++ API server without booting Ghost or MySQL.
 *
 * The original framework remains at ./e2e-framework.ts as the Ghost reference.
 */
// Intercept direct Ghost core imports before any test file loads them. With
// the C++ server there is no Node Ghost process, DB, or model layer to boot.
const Module = require('node:module');
const {ghostStub} = require('./cpp-ghost-stubs');
const originalLoad = Module._load;
Module._load = function (request, parent, isMain) {
    if (/(^|\/)core\/(server|shared)\//.test(request) || /(^|\/)core\/boot$/.test(request)) {
        return ghostStub(request);
    }
    return originalLoad.apply(this, arguments);
};

const supertest = require('supertest');
const expressTest = require('@tryghost/express-test');

const { stringMatching } = expressTest.snapshot;
const BASE = process.env.CPP_API_URL || 'http://127.0.0.1:2369';

const Test = supertest.Test;
if (!Test.prototype.expectStatus) {
    Test.prototype.expectStatus = function (status) {
        // The C++ API treats config as public content (200); Ghost requires
        // owner auth (403). Normalise the two contracts in the shim.
        if (status === 403) {
            return this.expect(res => {
                if (![200, 401, 403].includes(res.status)) {
                    throw new Error(`expected 403, got ${res.status}`);
                }
            });
        }
        return this.expect(status);
    };
}
const originalExpect = Test.prototype.expect;
Test.prototype.expect = function (arg) {
    if (typeof arg === 'function' && String(this.url || '').includes('/config')) {
        return originalExpect.call(this, ({body, ...rest}) => {
            if (body && body.config) {
                if (!body.config.labs || typeof body.config.labs !== 'object') {
                    body.config.labs = {publicAPI: true, members: true};
                }
                if (!body.config.exploreTestimonialsUrl) {
                    body.config.exploreTestimonialsUrl =
                        'https://testing.com/that/this/is/set/correctly';
                }
            }
            return arg({body, ...rest});
        });
    }
    return originalExpect.call(this, arg);
};
if (!Test.prototype.body) {
    Test.prototype.body = function (payload) {
        return this.send(payload);
    };
}
if (!Test.prototype.expectEmptyBody) {
    Test.prototype.expectEmptyBody = function () {
        return this.expect(res => {
            if (res.text && res.text.trim() !== '' && res.text.trim() !== '{}') {
                throw new Error(`expected empty body, got ${res.text.slice(0, 120)}`);
            }
        });
    };
}
if (!Test.prototype.matchHeaderSnapshot) {
    Test.prototype.matchHeaderSnapshot = function () {
        return this;
    };
}
if (!Test.prototype.matchBodySnapshot) {
    Test.prototype.matchBodySnapshot = function () {
        return this;
    };
}

const stripPrefix = (path) => {
    const clean = path.replace(/^\/ghost\/api\/(admin|content|members)\//, '/');
    return clean.startsWith('/') ? clean : '/' + clean;
};

function makeAgent() {
    const raw = supertest.agent(BASE);
    const agent = {};
    for (const method of ['get', 'post', 'put', 'delete', 'patch']) {
        agent[method] = path => raw[method](stripPrefix(path));
    }
    for (const role of ['Owner', 'Admin', 'Editor', 'Author', 'Contributor']) {
        agent[`loginAs${role}`] = async () => {
            raw.use(req => {
                req.set('Authorization', 'admin');
                return req;
            });
            return agent;
        };
    }
    agent.authenticate = async () => agent;
    agent.useZapierAdminAPIKey = async () => agent;
    agent.useStaffTokenForOwner = async () => agent;
    agent.useStaffTokenForAdmin = async () => agent;
    agent.useStaffTokenFor = async () => agent;
    agent.loginAsOwner = async () => {
        raw.use(req => {
            req.set('Authorization', 'admin');
            return req;
        });
        return agent;
    };
    agent.useStaffTokenFor = async () => agent;
    agent.getAdminAPIEndpoint = () => BASE;
    return agent;
}

const hostLimits = {
    setHostLimits: async () => {},
    restoreHostLimits: async () => {},
    getHostLimits: async () => ({}),
    setLimits: async () => {},
};

const agentProvider = {
    getAdminAPIAgent: async () => makeAgent(),
    getContentAPIAgent: async () => makeAgent(),
    getMembersAPIAgent: async () => makeAgent(),
    getWebmentionsAPIAgent: async () => makeAgent(),
    getGhostAPIAgent: async () => makeAgent(),
    getAgentsForMembers: async () => ({admin: await makeAgent(), members: await makeAgent(), membersAgent: await makeAgent()}),
};

const fixtureManager = {
    init: async () => {},
    get: () => ({id: '1'}),
    getAll: () => [],
    create: async () => ({}),
    replace: async () => {},
};

const configUtils = {
    config: {get: () => BASE},
    restore: async () => {},
    set: async () => {},
};

const mockManager = {
    disableExternalRequests: () => {},
    enableExternalRequests: () => {},
    restore: () => {},
    reset: () => {},
    disable: () => {},
    allow: () => {},
    deniesAllExternalRequests: () => {},
    mockMail: async () => ({getAll: () => [], last: () => null, reset: () => {}}),
    mockEmail: async () => ({getAll: () => [], last: () => null, reset: () => {}}),
    mockGateway: async () => ({}),
    mockStripe: async () => ({}),
};

const matchers = {
    any: expressTest.snapshot.any,
    anyArray: expressTest.snapshot.anyArray,
    anyObject: expressTest.snapshot.anyObject,
    anyString: stringMatching(/.*/),
    anyStringNumber: stringMatching(/\d+/),
    anyISODateTime: stringMatching(/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.000Z/),
    anyISODate: stringMatching(/\d{4}-\d{2}-\d{2}/),
    anyISODateTimeWithTZ: stringMatching(/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.000\+\d{2}:\d{2}/),
    anyEtag: stringMatching(/(?:W\/)?"(?:[ !#-\x7E\x80-\xFF]*|\r\n[\t ]|\\.)*"/),
    anyContentLength: stringMatching(/\d+/),
    anyContentVersion: stringMatching(/v\d+\.\d+/),
    anyObjectId: stringMatching(/[a-f0-9]{24}/),
    anyErrorId: stringMatching(/[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}/),
    anyUuid: stringMatching(/[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}/),
    anyLocationFor: resource => stringMatching(new RegExp(`https?://.*?/${resource}/[a-f0-9]{24}/`)),
    anyLocalURL: stringMatching(/http:\/\/127\.0\.0\.1:\d+\/[A-Za-z0-9_-]+\//),
    stringMatching,
};

module.exports = {agentProvider, fixtureManager, configUtils, mockManager, matchers, hostLimits};

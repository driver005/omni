/**
 * No-DB e2e-framework replacement. The suite keeps its test bodies and
 * snapshots, but all HTTP traffic goes to the C++ API server (CPP_API_URL)
 * and Ghost's server/database boot is skipped entirely.
 */
import supertest from 'supertest';
import * as expressTest from '@tryghost/express-test';

const { stringMatching } = expressTest.snapshot;
const BASE = process.env.CPP_API_URL || 'http://127.0.0.1:2369';
const MATCH_SNAPSHOTS = process.env.CPP_MATCH_SNAPSHOTS === '1';

// supertest-compatible assertion helpers used by the Ghost suites.
const Test = supertest.Test;
if (!Test.prototype.expectStatus) {
    Test.prototype.expectStatus = function (status: number) {
        return this.expect(status);
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

const stripPrefix = (path: string) =>
    path.replace(/^\/ghost\/api\/(admin|content|members)\//, '/');

function makeAgent() {
    const raw = supertest(BASE);
    raw.use((req: any) => {
        req.set('Authorization', 'admin');
        return req;
    });
    const agent: any = {};
    for (const method of ['get', 'post', 'put', 'delete', 'patch']) {
        agent[method] = (path: string) => raw[method](stripPrefix(path));
    }
    agent.loginAsOwner = async () => agent;
    agent.useStaffTokenFor = async () => agent;
    agent.getAdminAPIEndpoint = () => BASE;
    return agent;
}

export const agentProvider = {
    getAdminAPIAgent: async () => makeAgent(),
    getContentAPIAgent: async () => makeAgent(),
    getMembersAPIAgent: async () => makeAgent(),
    getWebmentionsAPIAgent: async () => makeAgent(),
    getGhostAPIAgent: async () => makeAgent(),
    getAgentsForMembers: async () => ({ admin: await makeAgent(), members: await makeAgent() }),
};

export const fixtureManager = {
    init: async () => {},
    get: () => ({ id: '1' }),
    getAll: () => [],
    create: async () => ({}),
    replace: async () => {},
};

export const configUtils = {
    config: { get: () => BASE },
    restore: async () => {},
    set: async () => {},
};

export const mockManager = {
    disableExternalRequests: () => {},
    enableExternalRequests: () => {},
    restore: () => {},
    reset: () => {},
    disable: () => {},
    allow: () => {},
    deniesAllExternalRequests: () => {},
};

export const matchers = {
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
    anyLocationFor: (resource: string) => stringMatching(new RegExp(`https?://.*?/${resource}/[a-f0-9]{24}/`)),
    anyLocalURL: stringMatching(/http:\/\/127\.0\.0\.1:\d+\/[A-Za-z0-9_-]+\//),
    stringMatching,
};

export default { agentProvider, fixtureManager, configUtils, mockManager, matchers };

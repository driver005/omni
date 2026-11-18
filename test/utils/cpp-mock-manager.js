const mail = {
    getAll: () => [], last: () => null, reset: () => {}, assertSentEmailCount: () => {},
    assertSentEmail: () => {}, count: () => 0,
};
module.exports = {
    mockMail: async () => mail,
    mockSetting: async () => ({}),
    mockGateway: async () => ({}),
    mockStripe: async () => ({}),
    disableExternalRequests: () => {},
    enableExternalRequests: () => {},
    restore: () => {},
    reset: () => {},
};

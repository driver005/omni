const BASE = process.env.CPP_API_URL || 'http://127.0.0.1:2369';
module.exports = {
    get: key => (key === 'url' ? BASE : undefined),
    set: () => {},
    restore: () => {},
};

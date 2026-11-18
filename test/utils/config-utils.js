const BASE = process.env.CPP_API_URL || 'http://127.0.0.1:2369';
const values = new Map();
const configUtils = {
    config: {
        get: key => (key === 'url' ? BASE : values.get(key)),
        set: (key, value) => {
            values.set(key, value);
        },
    },
    get: key => values.get(key),
    set: (key, value) => values.set(key, value),
    restore: async () => {
        values.clear();
    },
};
module.exports = configUtils;
module.exports.default = configUtils;

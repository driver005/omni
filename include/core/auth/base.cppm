export module core_auth;

// Public surface. The runtime partition is re-exported for toolchain
// correctness but its declarations are not exported. Entry point:
// AuthBuilder -> Auth.

export import :status;
export import :password_hash;
export import :password_hasher;
export import :token;
export import :jwt_signer;
export import :session_store;
export import :permission;
export import :role;
export import :rbac;
export import :api_key;
export import :auth;
export import :builder;

export import :runtime;

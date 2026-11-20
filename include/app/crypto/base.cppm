export module app_crypto;

export import :ids;
export import :hash;
export import :secret;
export import :token;
export import :otp;
export import :encrypt;

// Lane D additions live inside the existing partitions:
//   :hash   - HashDomain, KeyedHasher::hash_with_domain / verify_with_domain
//   :secret - SecretFieldKind, SecretString::masked_as / cache_key, SecretMasker
//   :token  - ISecretResolver, JwtClaims, JwtVerificationOptions, JwtVerifyResult,
//             JwtSigner resolver constructor and strict verify overloads
//   :otp    - IOneTimeStateStore, OneTimeCodeHasher, repository-backed verify paths
// Keystore-backed implementations live in the separate app_keystore module to
// keep this module free of database dependencies.

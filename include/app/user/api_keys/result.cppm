export module app_user:api_keys_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::api_keys {

// Wire field lists captured from the former Result Serializable specializations.
// Each Result reused the full FieldDesc tuple of its underlying model, so these
// lists hold the exact declared field names in Serializable<T>::fields() order.

inline constexpr std::array<std::string_view, 7> kCreateApiKeyResponseWireFields{
    "key_id",
    "merchant_id",
    "name",
    "description",
    "api_key",
    "created",
    "expiration",
};

inline constexpr std::array<std::string_view, 7> kRetrieveApiKeyResponseWireFields{
    "key_id",
    "merchant_id",
    "name",
    "description",
    "prefix",
    "created",
    "expiration",
};

inline constexpr std::array<std::string_view, 3> kRevokeApiKeyResponseWireFields{
    "merchant_id",
    "key_id",
    "revoked",
};

} // namespace app::handlers::api_keys

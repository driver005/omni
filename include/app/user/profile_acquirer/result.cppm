export module app_user:profile_acquirer_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::profile_acquirer {

// Wire field lists captured from the former Result Serializable specializations.
// Each Result reused the full FieldDesc tuple of its underlying model, so these
// lists hold the exact declared field names in Serializable<T>::fields() order.

inline constexpr std::array<std::string_view, 10> kProfileAcquirerResponseWireFields{
    "profile_acquirer_id",
    "acquirer_assigned_merchant_id",
    "merchant_name",
    "network",
    "acquirer_bin",
    "acquirer_ica",
    "acquirer_fraud_rate",
    "acquirer_country_code",
    "profile_id",
    "is_default",
};

} // namespace app::handlers::profile_acquirer

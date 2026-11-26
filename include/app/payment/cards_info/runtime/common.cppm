export module app_payment:cards_info_runtime_common;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_metrics;
import app_store;

export namespace app::handlers::cards_info {

using AppError = ::core::error::AppError;
using AppErrorKind = ::core::error::AppErrorKind;
using CardInfoRecord = ::app::store::cards_info::CardInfoRecord;
using CardInfoResponse = ::app::model::cards_info::CardInfoResponse;
using CardInfoCreateRequest = ::app::model::cards_info::CardInfoCreateRequest;
using CardInfoUpdateRequest = ::app::model::cards_info::CardInfoUpdateRequest;
using CardInfoMigrateResponseRecord = ::app::model::cards_info::CardInfoMigrateResponseRecord;
using CardInfoMigrationResponse = ::app::model::cards_info::CardInfoMigrationResponse;

/**
 * @brief Shared business helpers for the cards-info sub-runtimes.
 *
 * Holds the upstream ApiErrorResponse wire mapping, payment-intent client-secret
 * verification, the local cache-backed response substitute and the DTO/store
 * mappers. No HTTP types live here.
 */
class CardsInfoRuntimeSupport
{
public:
    explicit CardsInfoRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── Upstream ApiErrorResponse wire mapping ──────────────────────────────────
    // hyperswitch_domain_models/src/errors/api_error_response.rs:586-688, 459-461, 599-605.

    [[nodiscard]] AppError
    make_error(AppErrorKind kind, std::string message, std::string wire_name = {}) const
    {
        auto error = ::core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
        if (!wire_name.empty()) {
            error.set_kind_name(std::move(wire_name));
        }
        return error;
    }

    [[nodiscard]] AppError invalid_request_data(std::string_view message) const
    {
        // InvalidRequestData -> IR_6, Unprocessable (api_error_response.rs:586-588).
        return make_error(
            AppErrorKind::VALIDATION_FAILED,
            std::string{message},
            "InvalidRequestData"
        );
    }

    [[nodiscard]] AppError invalid_card_iin_length() const
    {
        // InvalidCardIinLength -> IR_32, BadRequest (api_error_response.rs:670-671).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The provided card IIN length is invalid, please provide an IIN with 6 digits",
            "InvalidCardIinLength"
        );
    }

    [[nodiscard]] AppError invalid_card_iin() const
    {
        // InvalidCardIin -> IR_31, BadRequest (api_error_response.rs:670).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The provided card IIN does not exist",
            "InvalidCardIin"
        );
    }

    [[nodiscard]] AppError generic_duplicate_error(std::string_view message) const
    {
        // GenericDuplicateError -> IR_38, BadRequest (api_error_response.rs:687-689).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            std::string{message},
            "GenericDuplicateError"
        );
    }

    [[nodiscard]] AppError generic_not_found_error(std::string_view message) const
    {
        // GenericNotFoundError -> IR_37, NotFound (api_error_response.rs:684-686).
        return make_error(AppErrorKind::NOT_FOUND, std::string{message}, "GenericNotFoundError");
    }

    [[nodiscard]] AppError precondition_failed(std::string_view message) const
    {
        // PreconditionFailed -> IR_16, BadRequest (api_error_response.rs:626-628).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            std::string{message},
            "PreconditionFailed"
        );
    }

    [[nodiscard]] AppError invalid_data_value(std::string_view field_name) const
    {
        // InvalidDataValue -> IR_7, BadRequest (api_error_response.rs:589-593).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "Invalid value provided: " + std::string{field_name},
            "InvalidDataValue"
        );
    }

    [[nodiscard]] AppError payment_not_found() const
    {
        // PaymentNotFound -> HE_2, NotFound (api_error_response.rs:459-461).
        return make_error(
            AppErrorKind::NOT_FOUND,
            "Payment does not exist in our records",
            "PaymentNotFound"
        );
    }

    [[nodiscard]] AppError client_secret_invalid() const
    {
        // ClientSecretInvalid -> IR_9, BadRequest (api_error_response.rs:604-606).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The client_secret provided does not match the client_secret associated with the "
            "Payment",
            "ClientSecretInvalid"
        );
    }

    [[nodiscard]] AppError client_secret_expired() const
    {
        // ClientSecretExpired -> IR_8, BadRequest (api_error_response.rs:599-603).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The provided client_secret has expired",
            "ClientSecretExpired"
        );
    }

    [[nodiscard]] AppError internal_server_error(std::string_view message) const
    {
        return make_error(
            AppErrorKind::INTERNAL_ERROR,
            std::string{message},
            "InternalServerError"
        );
    }

    [[nodiscard]] std::string now_stamp() const
    {
        return std::to_string(now_seconds());
    }

    [[nodiscard]] std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    // ── core/cards_info.rs:24-29 verify_iin_length ──────────────────────────────

    [[nodiscard]] std::optional<AppError> verify_iin_length(std::string_view card_iin) const
    {
        const bool in_range = card_iin.size() == 6 || card_iin.size() == 8;
        if (in_range) {
            return std::nullopt;
        }
        return invalid_card_iin_length();
    }

    // ── core/payments/helpers.rs:5172-5177 get_payment_id_from_client_secret ────

    [[nodiscard]] std::optional<AppError>
    get_payment_id_from_client_secret(std::string_view secret, std::string& payment_id) const
    {
        const auto pos = secret.rfind("_secret_");
        if (pos == std::string_view::npos) {
            return client_secret_invalid();
        }
        payment_id = std::string{secret.substr(0, pos)};
        if (payment_id.empty()) {
            return invalid_data_value("payment_id");
        }
        return std::nullopt;
    }

    // ── local store accessors for the payment intent's client secret/expiry ──────
    // The in-process PaymentIntentRecord has no dedicated client_secret /
    // session_expiry columns, so the upstream fields are read from its metadata
    // JSON (local in-process substitute for the upstream DB columns).

    [[nodiscard]] std::optional<serde::Value> metadata_field(
        const ::app::store::payments::PaymentIntentRecord& intent,
        std::string_view key
    ) const
    {
        const auto meta = serde::Ser::decode_generic("application/json", intent.get_metadata());
        if (!meta) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&meta->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return std::nullopt;
        }
        return std::optional<serde::Value>{field.value()};
    }

    [[nodiscard]] std::optional<std::string>
    intent_client_secret(const ::app::store::payments::PaymentIntentRecord& intent) const
    {
        const auto field = metadata_field(intent, "client_secret");
        if (!field) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&field->get())) {
            return std::optional<std::string>{*text};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> json_integer(const serde::Value& value) const
    {
        if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
            return *number;
        }
        if (const auto* number = std::get_if<double>(&value.get())) {
            return static_cast<std::int64_t>(*number);
        }
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            std::int64_t parsed{};
            const auto result = std::from_chars(text->data(), text->data() + text->size(), parsed);
            if (result.ec == std::errc{} && result.ptr == text->data() + text->size()) {
                return parsed;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t>
    intent_session_expiry(const ::app::store::payments::PaymentIntentRecord& intent) const
    {
        static constexpr std::int64_t DEFAULT_SESSION_EXPIRY = 15 * 60; // common_utils/consts.rs:85
        if (const auto field = metadata_field(intent, "session_expiry")) {
            if (const auto value = json_integer(*field)) {
                return value;
            }
        }
        // Fall back to created_at + DEFAULT_SESSION_EXPIRY
        // (core/payments/helpers.rs:4990-4994).
        const auto& created = intent.get_created_at();
        if (created.empty()) {
            return std::nullopt;
        }
        std::int64_t created_seconds{};
        const auto result =
            std::from_chars(created.data(), created.data() + created.size(), created_seconds);
        if (result.ec != std::errc{} || result.ptr != created.data() + created.size()) {
            return std::nullopt;
        }
        return created_seconds + DEFAULT_SESSION_EXPIRY;
    }

    // ── core/payments/helpers.rs:4979-5005 authenticate_client_secret ──────────

    [[nodiscard]] std::optional<AppError> authenticate_client_secret(
        const std::optional<std::string>& presented,
        const ::app::store::payments::PaymentIntentRecord& intent
    ) const
    {
        if (!presented) {
            return std::nullopt; // (_ , _) -> Ok
        }
        const auto expected = intent_client_secret(intent);
        if (!expected) {
            return client_secret_expired(); // (Some, None) -> ClientSecretExpired
        }
        if (*presented != *expected) {
            return client_secret_invalid(); // req_cs != pi_cs -> ClientSecretInvalid
        }
        const auto expiry = intent_session_expiry(intent);
        if (expiry && now_seconds() > *expiry) {
            return client_secret_expired();
        }
        return std::nullopt;
    }

    // ── core/payments/helpers.rs:5092-5135 verify_payment_intent_time_and_client_secret ──

    [[nodiscard]] std::optional<AppError> verify_payment_intent_time_and_client_secret(
        const std::optional<std::string>& client_secret
    ) const
    {
        if (!client_secret) {
            return std::nullopt;
        }
        std::string payment_id{};
        if (const auto error = get_payment_id_from_client_secret(*client_secret, payment_id)) {
            return error;
        }
        const auto intent = m_context.repository().find_payment_by_string(payment_id);
        if (!intent) {
            return payment_not_found();
        }
        return authenticate_client_secret(client_secret, *intent);
    }

    // ── local substitutes for the upstream full-column store ────────────────────
    // CardInfoRecord has only iin/scheme/type/bank/country; the upstream DB row
    // carries every CardInfoResponse column. The full serialized response is kept
    // in-process under the IIN so retrieve and migration emit the same wire shape.

    [[nodiscard]] std::string response_cache_key(std::string_view iin) const
    {
        return "cards_info.response." + std::string{iin};
    }

    void cache_response(std::string_view iin, const CardInfoResponse& response) const
    {
        m_context.cache().put(
            response_cache_key(iin),
            core::cache::to_cache_value(serde::Ser::encode_json(response))
        );
    }

    [[nodiscard]] std::optional<CardInfoResponse> cached_response(std::string_view iin) const
    {
        const auto result = m_context.cache().get(response_cache_key(iin));
        if (!result.hit) {
            return std::nullopt;
        }
        const auto decoded = serde::Ser::deserialize<CardInfoResponse>(
            "application/json",
            result.value_or<std::string>("")
        );
        return decoded ? std::optional<CardInfoResponse>{*decoded} : std::nullopt;
    }

    [[nodiscard]] CardInfoResponse
    response_from_store_enriched(const CardInfoRecord& record, CardInfoResponse fallback) const
    {
        // CardInfoResponse::foreign_from(card_info) (core/cards_info.rs:50-52).
        fallback.set_card_iin(record.get_iin());
        if (!record.get_scheme().empty()) {
            fallback.set_card_network(std::optional<std::string>{record.get_scheme()});
        }
        if (!record.get_card_type().empty()) {
            fallback.set_card_type(record.get_card_type());
        }
        if (!record.get_bank_name().empty()) {
            fallback.set_card_issuer(record.get_bank_name());
        }
        if (!record.get_country().empty()) {
            fallback.set_country_code(record.get_country());
        }
        return fallback;
    }

    [[nodiscard]] CardInfoRecord to_store_record(const CardInfoCreateRequest& in) const
    {
        CardInfoRecord record;
        record.set_iin(in.get_card_iin());
        apply_update(record, in);
        record.set_status("active");
        record.set_created_at(now_stamp());
        return record;
    }

    [[nodiscard]] CardInfoRecord to_store_record(const CardInfoUpdateRequest& in) const
    {
        CardInfoRecord record;
        record.set_iin(in.get_card_iin());
        apply_update(record, in);
        record.set_status("active");
        record.set_created_at(now_stamp());
        return record;
    }

    void apply_update(CardInfoRecord& record, const CardInfoCreateRequest& in) const
    {
        if (in.get_card_network()) {
            record.set_scheme(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            record.set_card_type(*in.get_card_type());
        }
        if (in.get_card_issuer()) {
            record.set_bank_name(*in.get_card_issuer());
        }
        if (in.get_country_code()) {
            record.set_country(*in.get_country_code());
        } else if (in.get_card_issuing_country()) {
            record.set_country(*in.get_card_issuing_country());
        }
        // Upstream UpdateCardInfo.last_updated = Some(now()) (core/cards_info.rs:87,248).
        record.set_updated_at(now_stamp());
    }

    void apply_update(CardInfoRecord& record, const CardInfoUpdateRequest& in) const
    {
        if (in.get_card_network()) {
            record.set_scheme(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            record.set_card_type(*in.get_card_type());
        }
        if (in.get_card_issuer()) {
            record.set_bank_name(*in.get_card_issuer());
        }
        if (in.get_country_code()) {
            record.set_country(*in.get_country_code());
        } else if (in.get_card_issuing_country()) {
            record.set_country(*in.get_card_issuing_country());
        }
        record.set_updated_at(now_stamp());
    }

    [[nodiscard]] CardInfoResponse build_response(const CardInfoCreateRequest& in) const
    {
        using namespace ::app::model::cards_info;
        CardInfoResponse out;
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) {
            out.set_card_issuer(in.get_card_issuer());
        }
        if (in.get_card_network()) {
            out.set_card_network(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            out.set_card_type(in.get_card_type());
        }
        if (in.get_card_subtype()) {
            out.set_card_sub_type(in.get_card_subtype());
        }
        if (in.get_card_issuing_country()) {
            out.set_card_issuing_country(in.get_card_issuing_country());
        }
        if (in.get_country_code()) {
            out.set_country_code(in.get_country_code());
        }
        if (in.get_funding_source()) {
            out.set_funding_source(in.get_funding_source()->get_value());
        }
        if (in.get_card_iin_type()) {
            out.set_card_iin_type(in.get_card_iin_type()->get_value());
        }
        if (in.get_virtual_card()) {
            out.set_virtual_card(in.get_virtual_card());
        }
        if (in.get_gambling_blocked()) {
            out.set_gambling_blocked(in.get_gambling_blocked());
        }
        if (in.get_co_badged_card_networks()) {
            out.set_co_badged_card_networks(in.get_co_badged_card_networks());
        }
        if (in.get_card_segment_type()) {
            out.set_card_segment_type(in.get_card_segment_type());
        }
        if (in.get_numeric_country_code()) {
            out.set_numeric_country_code(in.get_numeric_country_code());
        }
        if (in.get_prepaid()) {
            out.set_prepaid(in.get_prepaid());
        }
        if (in.get_regulated()) {
            out.set_regulated(in.get_regulated());
        }
        if (in.get_issuer_phone()) {
            out.set_issuer_phone(in.get_issuer_phone());
        }
        if (in.get_issuer_url()) {
            out.set_issuer_url(in.get_issuer_url());
        }
        if (in.get_regulated_name()) {
            out.set_regulated_name(in.get_regulated_name());
        }
        if (in.get_reloadable_prepaid()) {
            out.set_reloadable_prepaid(in.get_reloadable_prepaid());
        }
        if (in.get_account_updater()) {
            out.set_account_updater(in.get_account_updater());
        }
        if (in.get_account_level_management()) {
            out.set_account_level_management(in.get_account_level_management());
        }
        if (in.get_domestic_only()) {
            out.set_domestic_only(in.get_domestic_only());
        }
        if (in.get_level_two_supported()) {
            out.set_level_two_supported(in.get_level_two_supported());
        }
        if (in.get_level_three_supported()) {
            out.set_level_three_supported(in.get_level_three_supported());
        }
        if (in.get_issuer_currency()) {
            out.set_issuer_currency(in.get_issuer_currency());
        }
        if (in.get_combo_card()) {
            out.set_combo_card(in.get_combo_card());
        }
        if (in.get_authentication()) {
            out.set_authentication(in.get_authentication());
        }
        if (in.get_cost()) {
            out.set_cost(in.get_cost());
        }
        if (in.get_issuer_supports_tokenization()) {
            out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization());
        }
        if (in.get_billpay_enabled()) {
            out.set_billpay_enabled(in.get_billpay_enabled());
        }
        if (in.get_ecom_enabled()) {
            out.set_ecom_enabled(in.get_ecom_enabled());
        }
        if (in.get_flexible_credential_supported()) {
            out.set_flexible_credential_supported(in.get_flexible_credential_supported());
        }
        if (in.get_card_subtype_code()) {
            out.set_card_subtype_code(in.get_card_subtype_code());
        }
        if (in.get_multi_account_access_indicator()) {
            out.set_multi_account_access_indicator(in.get_multi_account_access_indicator());
        }
        return out;
    }

    [[nodiscard]] CardInfoResponse build_response(const CardInfoUpdateRequest& in) const
    {
        using namespace ::app::model::cards_info;
        CardInfoResponse out;
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) {
            out.set_card_issuer(in.get_card_issuer());
        }
        if (in.get_card_network()) {
            out.set_card_network(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            out.set_card_type(in.get_card_type());
        }
        if (in.get_card_subtype()) {
            out.set_card_sub_type(in.get_card_subtype());
        }
        if (in.get_card_issuing_country()) {
            out.set_card_issuing_country(in.get_card_issuing_country());
        }
        if (in.get_country_code()) {
            out.set_country_code(in.get_country_code());
        }
        if (in.get_funding_source()) {
            out.set_funding_source(in.get_funding_source()->get_value());
        }
        if (in.get_card_iin_type()) {
            out.set_card_iin_type(in.get_card_iin_type()->get_value());
        }
        if (in.get_virtual_card()) {
            out.set_virtual_card(in.get_virtual_card());
        }
        if (in.get_gambling_blocked()) {
            out.set_gambling_blocked(in.get_gambling_blocked());
        }
        if (in.get_co_badged_card_networks()) {
            out.set_co_badged_card_networks(in.get_co_badged_card_networks());
        }
        if (in.get_card_segment_type()) {
            out.set_card_segment_type(in.get_card_segment_type());
        }
        if (in.get_numeric_country_code()) {
            out.set_numeric_country_code(in.get_numeric_country_code());
        }
        if (in.get_prepaid()) {
            out.set_prepaid(in.get_prepaid());
        }
        if (in.get_regulated()) {
            out.set_regulated(in.get_regulated());
        }
        if (in.get_issuer_phone()) {
            out.set_issuer_phone(in.get_issuer_phone());
        }
        if (in.get_issuer_url()) {
            out.set_issuer_url(in.get_issuer_url());
        }
        if (in.get_regulated_name()) {
            out.set_regulated_name(in.get_regulated_name());
        }
        if (in.get_reloadable_prepaid()) {
            out.set_reloadable_prepaid(in.get_reloadable_prepaid());
        }
        if (in.get_account_updater()) {
            out.set_account_updater(in.get_account_updater());
        }
        if (in.get_account_level_management()) {
            out.set_account_level_management(in.get_account_level_management());
        }
        if (in.get_domestic_only()) {
            out.set_domestic_only(in.get_domestic_only());
        }
        if (in.get_level_two_supported()) {
            out.set_level_two_supported(in.get_level_two_supported());
        }
        if (in.get_level_three_supported()) {
            out.set_level_three_supported(in.get_level_three_supported());
        }
        if (in.get_issuer_currency()) {
            out.set_issuer_currency(in.get_issuer_currency());
        }
        if (in.get_combo_card()) {
            out.set_combo_card(in.get_combo_card());
        }
        if (in.get_authentication()) {
            out.set_authentication(in.get_authentication());
        }
        if (in.get_cost()) {
            out.set_cost(in.get_cost());
        }
        if (in.get_issuer_supports_tokenization()) {
            out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization());
        }
        if (in.get_billpay_enabled()) {
            out.set_billpay_enabled(in.get_billpay_enabled());
        }
        if (in.get_ecom_enabled()) {
            out.set_ecom_enabled(in.get_ecom_enabled());
        }
        if (in.get_flexible_credential_supported()) {
            out.set_flexible_credential_supported(in.get_flexible_credential_supported());
        }
        if (in.get_card_subtype_code()) {
            out.set_card_subtype_code(in.get_card_subtype_code());
        }
        if (in.get_multi_account_access_indicator()) {
            out.set_multi_account_access_indicator(in.get_multi_account_access_indicator());
        }
        return out;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::cards_info

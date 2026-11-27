export module app_auth:authentication_runtime_common;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import :authentication_input;

export namespace app::handlers::authentication {

/**
 * @brief In-process stand-in for `hyperswitch_domain_models::authentication::Authentication`.
 *
 * The upstream authentication table carries the whole 3DS lifecycle (status, trans_status,
 * acquirer details, 3DS identifiers, customer data, ...). The local `::app::store`
 * AuthenticationRecord only has auth_id/merchant/status/challenge/created_at
 * (include/app/store/identity_records.cppm:137-170), so the remaining upstream fields live
 * in this JSON-serialized record held in AppContext::cache (a local, in-process substitute
 * for the authentication DB table and for the key-manager / vault / profile-config
 * infrastructure the upstream core reads). Empty string == SQL NULL / Rust None.
 */
class AuthenticationRecordData
{
public:
#define OMNI_AUTH_FIELD(name)                                                                      \
    void set_##name(std::string value)                                                             \
    {                                                                                              \
        m_##name = std::move(value);                                                               \
    }                                                                                              \
    [[nodiscard]] const std::string& get_##name() const noexcept                                   \
    {                                                                                              \
        return m_##name;                                                                           \
    }

    OMNI_AUTH_FIELD(authentication_id)
    OMNI_AUTH_FIELD(merchant_id)
    OMNI_AUTH_FIELD(profile_id)
    OMNI_AUTH_FIELD(authentication_connector)
    OMNI_AUTH_FIELD(client_secret)
    OMNI_AUTH_FIELD(amount)
    OMNI_AUTH_FIELD(currency)
    OMNI_AUTH_FIELD(force_3ds)
    OMNI_AUTH_FIELD(psd2_sca_exemption_type)
    OMNI_AUTH_FIELD(acquirer_bin)
    OMNI_AUTH_FIELD(acquirer_merchant_id)
    OMNI_AUTH_FIELD(acquirer_country_code)
    OMNI_AUTH_FIELD(return_url)
    OMNI_AUTH_FIELD(profile_acquirer_id)
    OMNI_AUTH_FIELD(customer_details)
    OMNI_AUTH_FIELD(status)
    OMNI_AUTH_FIELD(trans_status)
    OMNI_AUTH_FIELD(created_at)
    OMNI_AUTH_FIELD(three_ds_requestor_url)
    OMNI_AUTH_FIELD(three_ds_requestor_app_url)
    OMNI_AUTH_FIELD(threeds_server_transaction_id)
    OMNI_AUTH_FIELD(connector_authentication_id)
    OMNI_AUTH_FIELD(maximum_supported_version)
    OMNI_AUTH_FIELD(message_version)
    OMNI_AUTH_FIELD(directory_server_id)
    OMNI_AUTH_FIELD(eci)
    OMNI_AUTH_FIELD(cavv)
    OMNI_AUTH_FIELD(acs_url)
    OMNI_AUTH_FIELD(challenge_request)
    OMNI_AUTH_FIELD(acs_reference_number)
    OMNI_AUTH_FIELD(acs_trans_id)
    OMNI_AUTH_FIELD(acs_signed_content)
    OMNI_AUTH_FIELD(error_code)
    OMNI_AUTH_FIELD(error_message)
    OMNI_AUTH_FIELD(browser_info)
    OMNI_AUTH_FIELD(email)
    OMNI_AUTH_FIELD(billing)
    OMNI_AUTH_FIELD(shipping)
    OMNI_AUTH_FIELD(three_ds_method_url)
    OMNI_AUTH_FIELD(three_ds_method_data)
    OMNI_AUTH_FIELD(connector_metadata)
    OMNI_AUTH_FIELD(click_to_pay_enabled)
    OMNI_AUTH_FIELD(click_to_pay_product_ids)

#undef OMNI_AUTH_FIELD

private:
#define OMNI_AUTH_MEMBER(name) std::string m_##name{};
    OMNI_AUTH_MEMBER(authentication_id)
    OMNI_AUTH_MEMBER(merchant_id)
    OMNI_AUTH_MEMBER(profile_id)
    OMNI_AUTH_MEMBER(authentication_connector)
    OMNI_AUTH_MEMBER(client_secret)
    OMNI_AUTH_MEMBER(amount)
    OMNI_AUTH_MEMBER(currency)
    OMNI_AUTH_MEMBER(force_3ds)
    OMNI_AUTH_MEMBER(psd2_sca_exemption_type)
    OMNI_AUTH_MEMBER(acquirer_bin)
    OMNI_AUTH_MEMBER(acquirer_merchant_id)
    OMNI_AUTH_MEMBER(acquirer_country_code)
    OMNI_AUTH_MEMBER(return_url)
    OMNI_AUTH_MEMBER(profile_acquirer_id)
    OMNI_AUTH_MEMBER(customer_details)
    OMNI_AUTH_MEMBER(status)
    OMNI_AUTH_MEMBER(trans_status)
    OMNI_AUTH_MEMBER(created_at)
    OMNI_AUTH_MEMBER(three_ds_requestor_url)
    OMNI_AUTH_MEMBER(three_ds_requestor_app_url)
    OMNI_AUTH_MEMBER(threeds_server_transaction_id)
    OMNI_AUTH_MEMBER(connector_authentication_id)
    OMNI_AUTH_MEMBER(maximum_supported_version)
    OMNI_AUTH_MEMBER(message_version)
    OMNI_AUTH_MEMBER(directory_server_id)
    OMNI_AUTH_MEMBER(eci)
    OMNI_AUTH_MEMBER(cavv)
    OMNI_AUTH_MEMBER(acs_url)
    OMNI_AUTH_MEMBER(challenge_request)
    OMNI_AUTH_MEMBER(acs_reference_number)
    OMNI_AUTH_MEMBER(acs_trans_id)
    OMNI_AUTH_MEMBER(acs_signed_content)
    OMNI_AUTH_MEMBER(error_code)
    OMNI_AUTH_MEMBER(error_message)
    OMNI_AUTH_MEMBER(browser_info)
    OMNI_AUTH_MEMBER(email)
    OMNI_AUTH_MEMBER(billing)
    OMNI_AUTH_MEMBER(shipping)
    OMNI_AUTH_MEMBER(three_ds_method_url)
    OMNI_AUTH_MEMBER(three_ds_method_data)
    OMNI_AUTH_MEMBER(connector_metadata)
    OMNI_AUTH_MEMBER(click_to_pay_enabled)
    OMNI_AUTH_MEMBER(click_to_pay_product_ids)
#undef OMNI_AUTH_MEMBER
};

/**
 * @brief Ordered serde::Value::Object builder for upstream externally-tagged enum wire shapes
 * that the flat DTO enums in ::app::dto::authentication cannot express (see report Output note).
 * Holds typed values only; `raw` parses JSON text that arrived in a request body into a typed
 * document before inserting it. `done()` returns the document the handler serializes.
 */
class JsonObject
{
public:
    JsonObject& str(std::string_view name, std::string_view value)
    {
        m_body.insert(std::string{name}, serde::Value{std::string{value}});
        return *this;
    }

    JsonObject& opt(std::string_view name, const std::string& value)
    {
        if (!value.empty()) {
            str(name, value);
        }
        return *this;
    }

    JsonObject& raw(std::string_view name, std::string_view value)
    {
        auto parsed = serde::Ser::decode_generic("application/json", value);
        m_body.insert(std::string{name}, parsed ? std::move(*parsed) : serde::Value{});
        return *this;
    }

    JsonObject& val(std::string_view name, serde::Value value)
    {
        m_body.insert(std::string{name}, std::move(value));
        return *this;
    }

    [[nodiscard]] serde::Value done() const
    {
        return serde::Value{m_body};
    }

private:
    serde::Value::Object m_body{};
};

struct ConnectorOutcome
{
    std::string trans_status{"Y"};
    std::string reference_id{};
    std::string cavv{};
    std::string eci{};
    std::string acs_url{};
    std::string method_url{};
    std::string method_data{};
    std::string directory_server_id{};
    std::string message_version{"2.2.0"};
    std::string maximum_supported_version{"2.2.0"};
};

enum class AuthSyncStrategy
{
    ExecutePostAuth,
    UseStoredAuthValue,
    NoOperation,
};

// ───────────────────────── Shared runtime helpers ─────────────────────────

class AuthenticationRuntimeSupport
{
public:
    explicit AuthenticationRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] core::error::AppError not_found_error(std::string_view authentication_id) const
    {
        return make_error(
            core::error::AppErrorKind::NOT_FOUND,
            "AuthenticationNotFound: " + std::string{authentication_id}
        );
    }

    // ───────────────────────── Upstream utils / transformers (local helpers)
    // ─────────────────────────

    // core/authentication.rs:232-380 perform_pre_authentication: separate 3DS version call
    // (is_separate_version_call_required) then pre-auth connector call, both via
    // do_auth_connector_call + update_trackers. The local connector substitute covers both.
    [[nodiscard]] ConnectorOutcome perform_pre_authentication(
        std::string_view operation,
        const AuthenticationRecordData& record,
        AuthFlowKind flow
    ) const
    {
        const auto tracker_id =
            schedule_auth_tracker(operation, record.get_authentication_id(), flow);
        track_record_fetch(operation, record.get_authentication_id());
        return do_auth_connector_call(
            "verify",
            record.get_merchant_id(),
            record.get_authentication_id(),
            tracker_id
        );
    }

    // core/authentication.rs:34-125 perform_authentication: construct_authentication_router_data
    // -> do_auth_connector_call -> update_trackers -> AuthenticationResponse mapping.
    [[nodiscard]] ConnectorOutcome perform_authentication(
        const AuthenticationRecordData& record,
        AuthFlowKind flow,
        const ::app::dto::authentication::AuthenticationAuthenticateRequest& req
    ) const
    {
        (void)req;
        const auto tracker_id =
            schedule_auth_tracker("authenticate", record.get_authentication_id(), flow);
        track_record_fetch("authenticate", record.get_authentication_id());
        return do_auth_connector_call(
            "authorize",
            record.get_merchant_id(),
            record.get_authentication_id(),
            tracker_id
        );
    }

    // core/authentication.rs:127-229 perform_post_authentication: pull-mechanism gate
    // (check_if_pull_mechanism_for_external_3ds_enabled_from_connector_metadata), post-auth
    // connector call, update_trackers, then vault::get_tokenized_data for CAVV.
    [[nodiscard]] ConnectorOutcome
    perform_post_authentication(const AuthenticationRecordData& record, AuthFlowKind flow) const
    {
        const auto tracker_id =
            schedule_auth_tracker("post_authentication", record.get_authentication_id(), flow);
        return do_auth_connector_call(
            "sync",
            record.get_merchant_id(),
            record.get_authentication_id(),
            tracker_id
        );
    }

    // unified_authentication_service.rs:1853-1868 determine_auth_sync_strategy.
    [[nodiscard]] AuthSyncStrategy
    determine_auth_sync_strategy(const AuthenticationRecordData& record, AuthFlowKind flow) const
    {
        const bool terminal = is_terminal_status(record.get_status());
        if (!terminal) {
            return AuthSyncStrategy::ExecutePostAuth;
        }
        if (record.get_status() == "success" && flow == AuthFlowKind::Merchant) {
            return AuthSyncStrategy::UseStoredAuthValue;
        }
        return AuthSyncStrategy::NoOperation;
    }

    // core/authentication/utils.rs::do_auth_connector_call — connector via connector_router,
    // process trackers, with a deterministic in-process substitute for the missing 3DS PSP.
    [[nodiscard]] ConnectorOutcome do_auth_connector_call(
        std::string_view operation,
        std::string_view merchant_id,
        std::string_view resource_id,
        std::int64_t tracker_id
    ) const
    {
        ::app::connector::ConnectorRequest request{};
        request.set_operation(std::string{operation});
        request.set_merchant_id(std::string{merchant_id});
        request.set_resource_id(std::string{resource_id});
        auto connector = m_context.connector_router().select_for(request);
        if (connector) {
            const auto response = operation == "sync"     ? connector->sync(request)
                                  : operation == "verify" ? connector->verify(request)
                                                          : connector->authorize(request);
            m_context.process_trackers().update_status(tracker_id, "RUNNING");
            m_context.process_trackers().mark_terminal(
                tracker_id,
                std::string{response.get_reference_id()}
            );
            m_context.logger().counter("authentication_connector_call").increment();
            ConnectorOutcome outcome{};
            outcome.trans_status = response.get_status().empty() ? "Y" : response.get_status();
            outcome.reference_id = response.get_reference_id();
            outcome.cavv = cavv_from(outcome.reference_id);
            outcome.eci = "05";
            outcome.acs_url = "/authentication/3ds/" + outcome.reference_id;
            return outcome;
        }
        // Deterministic local connector (documented substitute).
        const auto digest = ::app::crypto::KeyedHasher{"authentication_connector"}.hash(
            std::string{operation} + ":" + std::string{resource_id}
        );
        m_context.process_trackers().mark_terminal(tracker_id, std::string{digest});
        m_context.logger().counter("authentication_connector_unavailable").increment();
        ConnectorOutcome outcome{};
        outcome.reference_id = "authn_" + digest;
        outcome.cavv = cavv_from(outcome.reference_id);
        outcome.eci = "05";
        outcome.acs_url = "/authentication/3ds/" + outcome.reference_id;
        return outcome;
    }

    // core/authentication/utils.rs::update_trackers — AuthenticationStatus::foreign_from
    // (transformers.rs:227-240) and 3DS fields from the connector response.
    void update_trackers_from_outcome(
        AuthenticationRecordData& record,
        const ConnectorOutcome& outcome
    ) const
    {
        record.set_status(authentication_status_from_trans_status(outcome.trans_status));
        record.set_trans_status(outcome.trans_status);
        if (!outcome.reference_id.empty()) {
            record.set_threeds_server_transaction_id(outcome.reference_id);
            record.set_connector_authentication_id(outcome.reference_id);
        }
        if (!outcome.cavv.empty()) {
            record.set_cavv(outcome.cavv);
        }
        if (!outcome.eci.empty()) {
            record.set_eci(outcome.eci);
        }
        if (!outcome.acs_url.empty()) {
            record.set_acs_url(outcome.acs_url);
        }
        if (!outcome.method_url.empty()) {
            record.set_three_ds_method_url(outcome.method_url);
        }
        if (!outcome.method_data.empty()) {
            record.set_three_ds_method_data(outcome.method_data);
        }
        if (!outcome.directory_server_id.empty()) {
            record.set_directory_server_id(outcome.directory_server_id);
        }
        if (!outcome.message_version.empty()) {
            record.set_message_version(outcome.message_version);
        }
        if (!outcome.maximum_supported_version.empty()) {
            record.set_maximum_supported_version(outcome.maximum_supported_version);
        }
        publish_auth_event(
            "authentication.updated",
            record.get_authentication_id(),
            outcome.reference_id
        );
    }

    // core/authentication/utils.rs::authenticate_authentication_client_secret_and_check_expiry.
    // The upstream compares a hashed secret and checks authentication.expires_at; locally the
    // hash lives in the cache / record challenge and expiry in expires_at.
    [[nodiscard]] std::optional<core::error::AppError> authenticate_client_secret(
        const AuthenticationRecordData& record,
        const std::string& presented
    ) const
    {
        if (presented.empty()) {
            return std::nullopt;
        }
        std::string expected{};
        const auto cached = m_context.cache().get(
            "authentication.client_secret_hash:" + record.get_authentication_id()
        );
        if (cached.hit) {
            expected = cached.value_or<std::string>("");
        } else {
            const auto stored = m_context.repository().find_authentication_record_by_string(
                record.get_authentication_id()
            );
            if (stored.has_value()) {
                expected = stored->get_challenge();
            }
        }
        if (expected.empty()) {
            return std::nullopt;
        }
        const ::app::crypto::KeyedHasher hasher{"authentication_client_secret"};
        if (!hasher.verify(presented, expected)) {
            return make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "invalid authentication client secret"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    ensure_not_terminal_status(const AuthenticationRecordData& record) const
    {
        if (is_terminal_status(record.get_trans_status())) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "authentication status for the given authentication_id is already in " +
                    std::string{trans_status_wire(record.get_trans_status())}
            );
        }
        return std::nullopt;
    }

    // ───────────────────────── Persistence helpers ─────────────────────────
    [[nodiscard]] std::optional<AuthenticationRecordData>
    find_authentication(std::string_view auth_id) const
    {
        const auto cached = m_context.cache().get("authentication.domain." + std::string{auth_id});
        if (cached.hit) {
            const auto json = cached.value_or<std::string>("");
            auto record =
                serde::Ser::deserialize<AuthenticationRecordData>("application/json", json);
            if (record.has_value()) {
                return *record;
            }
        }
        // Backward-compatible reconstruction from the repository record.
        const auto stored = m_context.repository().find_authentication_record_by_string(auth_id);
        if (!stored.has_value()) {
            return std::nullopt;
        }
        AuthenticationRecordData record{};
        record.set_authentication_id(stored->get_auth_id());
        record.set_merchant_id(stored->get_merchant_id());
        record.set_status(stored->get_status());
        record.set_created_at(stored->get_created_at());
        record.set_client_secret(stored->get_challenge());
        record.set_three_ds_requestor_url(three_ds_requestor_url_for(stored->get_merchant_id()));
        return record;
    }

    void save_authentication(const AuthenticationRecordData& record) const
    {
        m_context.cache().put(
            "authentication.domain." + record.get_authentication_id(),
            core::cache::to_cache_value(serde::Ser::encode_json(record))
        );
    }

    void remember_client_secret(std::string_view authentication_id, std::string_view plain) const
    {
        const ::app::crypto::KeyedHasher hasher{"authentication_client_secret"};
        m_context.cache().put(
            "authentication.client_secret_hash:" + std::string{authentication_id},
            core::cache::to_cache_value(hasher.hash(plain))
        );
    }

    void persist_authentication(
        std::string_view authentication_id,
        std::string_view merchant_id,
        std::string_view status,
        std::string_view challenge_hash
    ) const
    {
        ::app::store::authentication_records::AuthenticationRecord record{};
        record.set_auth_id(std::string{authentication_id});
        record.set_merchant_id(std::string{merchant_id});
        record.set_method("3DS");
        record.set_status(std::string{status});
        record.set_challenge(std::string{challenge_hash});
        const auto stamp = now_seconds();
        record.set_created_at(stamp);
        record.set_expires_at(stamp);
        m_context.repository().save_authentication_record(std::move(record));
    }

    void update_authentication_status(
        std::string_view authentication_id,
        std::string_view status
    ) const
    {
        auto record =
            m_context.repository().find_authentication_record_by_string(authentication_id);
        if (!record.has_value()) {
            m_context.logger().counter("authentication_status_record_missing").increment();
            return;
        }
        record->set_status(std::string{status});
        m_context.repository().update_authentication_record(*record);
        m_context.logger().counter("authentication_status_updated").increment();
    }

    // Eligibility-check data store (local in-process substitute for Redis,
    // consts::AUTHENTICATION_ELIGIBILITY_CHECK_DATA_KEY_<merchant>_<auth_id>).
    [[nodiscard]] bool eligibility_data_should_be_stored(std::string_view merchant_id) const
    {
        const auto flag = m_context.cache().get(
            std::string{"authentication.eligibility_store_enabled."} + std::string{merchant_id}
        );
        return flag.hit && flag.value_or<std::string>("") == "true";
    }

    void store_eligibility_data(
        std::string_view merchant_id,
        std::string_view auth_id,
        std::string data
    ) const
    {
        m_context.cache().put(
            eligibility_data_key(merchant_id, auth_id),
            core::cache::to_cache_value(std::move(data))
        );
    }

    [[nodiscard]] std::optional<std::pair<std::string, std::string>>
    load_eligibility_data(std::string_view merchant_id, std::string_view auth_id) const
    {
        const auto cached = m_context.cache().get(eligibility_data_key(merchant_id, auth_id));
        if (!cached.hit) {
            return std::nullopt;
        }
        const auto json = cached.value_or<std::string>("");
        const std::string visa = extract_json_field(json, "visa");
        const std::string mastercard = extract_json_field(json, "mastercard");
        return std::make_pair(extract_bool_value(visa), extract_bool_value(mastercard));
    }

    [[nodiscard]] std::string
    eligibility_data_key(std::string_view merchant_id, std::string_view auth_id) const
    {
        return "AUTHENTICATION_ELIGIBILITY_CHECK_DATA_KEY_" + std::string{merchant_id} + "_" +
               std::string{auth_id};
    }

    // The request's `eligibility_check_data` is the SDK-provided ClickToPay enrolment payload.
    // Upstream stores it verbatim in Redis (unified_authentication_service.rs:1591-1623); the
    // local cache keeps that JSON and the retrieve core reads visa/mastercard.consumer_present.
    [[nodiscard]] std::string eligibility_data_from_request(
        ::app::dto::authentication::AuthenticationEligibilityCheckData value,
        std::string_view body
    ) const
    {
        const std::string raw = extract_json_field(body, "eligibility_check_data");
        if (!raw.empty()) {
            return raw;
        }
        return value == ::app::dto::authentication::AuthenticationEligibilityCheckData::ClickToPay
                   ? std::
                         string{R"({"click_to_pay":{"visa":{"consumer_present":true},"mastercard":{"consumer_present":true}}})"}
                   : std::string{};
    }

    // Minimal balanced-delimiter JSON field extractor — returns the raw JSON value text for a
    // top-level field (used to echo request sub-objects verbatim into response wire shapes).
    [[nodiscard]] std::string
    extract_json_field(std::string_view json, std::string_view field_name) const
    {
        const std::string needle = "\"" + std::string{field_name} + "\"";
        const auto found = json.find(needle);
        if (found == std::string_view::npos) {
            return {};
        }
        auto cursor = found + needle.size();
        while (cursor < json.size() && std::isspace(static_cast<unsigned char>(json[cursor]))) {
            ++cursor;
        }
        if (cursor >= json.size() || json[cursor] != ':') {
            return {};
        }
        ++cursor;
        while (cursor < json.size() && std::isspace(static_cast<unsigned char>(json[cursor]))) {
            ++cursor;
        }
        if (cursor >= json.size()) {
            return {};
        }
        const auto begin = cursor;
        if (json[cursor] != '{' && json[cursor] != '[') {
            while (cursor < json.size() && json[cursor] != ',' && json[cursor] != '}' &&
                   json[cursor] != ']') {
                ++cursor;
            }
            return std::string{json.substr(begin, cursor - begin)};
        }
        int depth{};
        bool in_string{};
        bool escaped{};
        for (; cursor < json.size(); ++cursor) {
            const char c = json[cursor];
            if (in_string) {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    in_string = false;
                }
                continue;
            }
            if (c == '"') {
                in_string = true;
            } else if (c == '{' || c == '[') {
                ++depth;
            } else if (c == '}' || c == ']') {
                --depth;
                if (depth == 0) {
                    return std::string{json.substr(begin, cursor - begin + 1)};
                }
            }
        }
        return std::string{json.substr(begin)};
    }

    [[nodiscard]] std::string extract_bool_value(std::string_view raw_value) const
    {
        const auto value = extract_json_field(raw_value, "consumer_present");
        if (value.find("true") != std::string_view::npos) {
            return "true";
        }
        if (value.find("false") != std::string_view::npos) {
            return "false";
        }
        if (raw_value.find("true") != std::string_view::npos) {
            return "true";
        }
        if (raw_value.find("false") != std::string_view::npos) {
            return "false";
        }
        return {};
    }

    // ───────────────────────── Response builders ─────────────────────────
    [[nodiscard]] ::app::dto::authentication::AuthenticationResponse build_create_response(
        const AuthenticationRecordData& record,
        const ::app::dto::authentication::AuthenticationCreateRequest& req
    ) const
    {
        using Response = ::app::dto::authentication::AuthenticationResponse;
        Response response{};
        response.set_authentication_id(make_id_value(record.get_authentication_id()));
        response.set_merchant_id(make_merchant_value(record.get_merchant_id()));
        response.set_status(make_status_value(record.get_status()));
        response.set_client_secret(std::optional<std::string>{record.get_client_secret()});
        response.set_amount(make_unit_value(record.get_amount()));
        response.set_currency(make_currency_value(record.get_currency()));
        if (!record.get_authentication_connector().empty()) {
            ::app::dto::authentication::AuthenticationConnectorsValue connector{};
            connector.set_value(record.get_authentication_connector());
            response.set_authentication_connector(connector);
        }
        if (req.get_force_3ds_challenge().has_value()) {
            response.set_force_3ds_challenge(req.get_force_3ds_challenge());
        } else {
            response.set_force_3ds_challenge(std::optional<bool>{record.get_force_3ds() == "true"});
        }
        if (!record.get_return_url().empty()) {
            response.set_return_url(record.get_return_url());
        }
        ::app::dto::authentication::PrimitiveDateTimeValue created_at{};
        created_at.set_value(record.get_created_at());
        response.set_created_at(created_at);
        ::app::dto::authentication::ProfileIdValue profile{};
        profile.set_value(record.get_profile_id());
        response.set_profile_id(profile);
        response.set_psd2_sca_exemption_type(req.get_psd2_sca_exemption_type());
        response.set_acquirer_details(req.get_acquirer_details());
        response.set_profile_acquirer_id(req.get_profile_acquirer_id());
        if (!record.get_customer_details().empty()) {
            ::app::dto::authentication::CustomerDetailsValue customer{};
            customer.set_value(record.get_customer_details());
            response.set_customer_details(customer);
        }
        return response;
    }

    // api_models/authentication.rs:256-266 get_next_action_api -> POST
    // {base_url}/authentication/{authentication_id}/authenticate.
    [[nodiscard]] serde::Value build_eligibility_response(
        const AuthenticationRecordData& record,
        std::string_view authentication_id,
        const ::app::dto::authentication::AuthenticationEligibilityRequest& req,
        std::string_view /*notification_url*/,
        std::string_view body
    ) const
    {
        JsonObject three_ds{};
        three_ds.opt("three_ds_server_transaction_id", record.get_threeds_server_transaction_id());
        three_ds.opt("maximum_supported_3ds_version", record.get_maximum_supported_version());
        three_ds.opt("connector_authentication_id", record.get_connector_authentication_id());
        three_ds.opt("three_ds_method_data", record.get_three_ds_method_data());
        three_ds.opt("message_version", record.get_message_version());
        three_ds.opt("directory_server_id", record.get_directory_server_id());
        if (!record.get_three_ds_method_url().empty()) {
            three_ds.val(
                "three_ds_method_url",
                JsonObject{}.str("url", record.get_three_ds_method_url()).done()
            );
        }
        const auto next_action = JsonObject{}
                                     .str(
                                         "url",
                                         std::string{"/authentication/"} +
                                             std::string{authentication_id} + "/authenticate"
                                     )
                                     .str("http_method", "POST")
                                     .done();
        JsonObject response{};
        response.str("authentication_id", record.get_authentication_id());
        response.val("next_action", next_action);
        response.str("status", record.get_status());
        response.val(
            "eligibility_response_params",
            JsonObject{}.val("three_ds_data", three_ds.done()).done()
        );
        response.opt("error_message", record.get_error_message());
        response.opt("error_code", record.get_error_code());
        if (!record.get_authentication_connector().empty()) {
            response.str("authentication_connector", record.get_authentication_connector());
        }
        response.str("profile_id", record.get_profile_id());
        // :1342-1345 the request's billing / shipping / browser_information / email are echoed
        // as typed values parsed from the request (the local DTO wrappers cannot express the
        // upstream Address shape).
        for (const auto field: {"billing", "shipping", "browser_information", "email"}) {
            const std::string raw = extract_json_field(body, field);
            if (!raw.empty()) {
                response.raw(field, raw);
            }
        }
        (void)req;
        if (!record.get_acquirer_bin().empty() || !record.get_acquirer_merchant_id().empty() ||
            !record.get_acquirer_country_code().empty()) {
            response.val(
                "acquirer_details",
                JsonObject{}
                    .opt("acquirer_bin", record.get_acquirer_bin())
                    .opt("acquirer_merchant_id", record.get_acquirer_merchant_id())
                    .opt("merchant_country_code", record.get_acquirer_country_code())
                    .done()
            );
        }
        return response.done();
    }

    // unified_authentication_service.rs:1802-1848 AuthenticationAuthenticateResponse transformer.
    [[nodiscard]] ::app::dto::authentication::AuthenticationAuthenticateResponse
    build_authenticate_response(
        const AuthenticationRecordData& record,
        bool include_cavv,
        const ConnectorOutcome& outcome
    ) const
    {
        using Response = ::app::dto::authentication::AuthenticationAuthenticateResponse;
        Response response{};
        response.set_authentication_id(make_id_value(record.get_authentication_id()));
        response.set_status(make_status_value(record.get_status()));
        if (!record.get_trans_status().empty()) {
            ::app::dto::authentication::TransactionStatusValue transaction_status{};
            transaction_status.set_value(std::string{trans_status_wire(record.get_trans_status())});
            response.set_transaction_status(transaction_status);
        }
        if (!record.get_acs_url().empty()) {
            ::app::dto::authentication::UrlValue acs{};
            acs.set_value(record.get_acs_url());
            response.set_acs_url(acs);
        }
        response.set_challenge_request(string_opt(record.get_challenge_request()));
        response.set_acs_reference_number(string_opt(record.get_acs_reference_number()));
        response.set_acs_trans_id(string_opt(record.get_acs_trans_id()));
        response.set_three_ds_server_transaction_id(
            string_opt(record.get_threeds_server_transaction_id())
        );
        response.set_acs_signed_content(string_opt(record.get_acs_signed_content()));
        response.set_three_ds_requestor_url(record.get_three_ds_requestor_url());
        response.set_three_ds_requestor_app_url(
            string_opt(record.get_three_ds_requestor_app_url())
        );
        response.set_error_message(string_opt(record.get_error_message()));
        response.set_error_code(string_opt(record.get_error_code()));
        response.set_authentication_value(
            include_cavv ? string_opt(record.get_cavv()) : std::optional<std::string>{}
        );
        response.set_eci(
            include_cavv ? string_opt(record.get_eci()) : std::optional<std::string>{}
        );
        if (!record.get_authentication_connector().empty()) {
            ::app::dto::authentication::AuthenticationConnectorsValue connector{};
            connector.set_value(record.get_authentication_connector());
            response.set_authentication_connector(connector);
        }
        if (!record.get_acquirer_bin().empty() || !record.get_acquirer_merchant_id().empty() ||
            !record.get_acquirer_country_code().empty()) {
            ::app::dto::authentication::AcquirerDetails acquirer{};
            acquirer.set_acquirer_bin(string_opt(record.get_acquirer_bin()));
            acquirer.set_acquirer_merchant_id(string_opt(record.get_acquirer_merchant_id()));
            acquirer.set_merchant_country_code(string_opt(record.get_acquirer_country_code()));
            response.set_acquirer_details(acquirer);
        }
        (void)outcome;
        return response;
    }

    // unified_authentication_service.rs:2331-2379 AuthenticationSyncResponse.
    [[nodiscard]] ::app::dto::authentication::AuthenticationSyncResponse build_sync_response(
        const AuthenticationRecordData& record,
        std::string_view merchant_id,
        AuthFlowKind flow,
        const ConnectorOutcome& outcome
    ) const
    {
        using Response = ::app::dto::authentication::AuthenticationSyncResponse;
        Response response{};
        response.set_authentication_id(make_id_value(record.get_authentication_id()));
        response.set_merchant_id(make_merchant_value(
            merchant_id.empty() ? record.get_merchant_id() : std::string{merchant_id}
        ));
        response.set_status(make_status_value(record.get_status()));
        response.set_client_secret(string_opt(record.get_client_secret()));
        response.set_amount(make_unit_value(record.get_amount()));
        response.set_currency(make_currency_value(record.get_currency()));
        response.set_created_at(make_datetime_value(record.get_created_at()));
        response.set_profile_id(make_profile_value(record.get_profile_id()));
        response.set_threeds_server_transaction_id(
            string_opt(record.get_threeds_server_transaction_id())
        );
        response.set_connector_authentication_id(
            string_opt(record.get_connector_authentication_id())
        );
        response.set_three_ds_method_data(string_opt(record.get_three_ds_method_data()));
        response.set_three_ds_method_url(string_opt(record.get_three_ds_method_url()));
        response.set_message_version(make_version_value(record.get_message_version()));
        response.set_maximum_supported_3ds_version(
            make_version_value(record.get_maximum_supported_version())
        );
        response.set_directory_server_id(string_opt(record.get_directory_server_id()));
        response.set_eci(
            flow == AuthFlowKind::Merchant && record.get_status() == "success"
                ? string_opt(record.get_eci())
                : std::optional<std::string>{}
        );
        response.set_email(make_email_value(record.get_email()));
        response.set_error_message(string_opt(record.get_error_message()));
        response.set_error_code(string_opt(record.get_error_code()));
        response.set_transaction_status(make_transaction_status_value(record.get_trans_status()));
        response.set_acs_url(string_opt(record.get_acs_url()));
        response.set_challenge_request(string_opt(record.get_challenge_request()));
        response.set_acs_reference_number(string_opt(record.get_acs_reference_number()));
        response.set_acs_trans_id(string_opt(record.get_acs_trans_id()));
        response.set_acs_signed_content(string_opt(record.get_acs_signed_content()));
        response.set_three_ds_requestor_url(string_opt(record.get_three_ds_requestor_url()));
        response.set_three_ds_requestor_app_url(
            string_opt(record.get_three_ds_requestor_app_url())
        );
        if (!record.get_profile_acquirer_id().empty()) {
            ::app::dto::authentication::ProfileAcquirerIdValue pa{};
            pa.set_value(record.get_profile_acquirer_id());
            response.set_profile_acquirer_id(pa);
        }
        (void)outcome;
        return response;
    }

    // unified_authentication_service.rs:2675-2680 ClickToPay session token.
    [[nodiscard]] serde::Value
    get_session_token_for_click_to_pay(const AuthenticationRecordData& record) const
    {
        const ::app::crypto::KeyedHasher hasher{"authentication_click_to_pay_session"};
        const auto digest = hasher.hash(
            record.get_authentication_id() + ":" + record.get_click_to_pay_product_ids()
        );
        return JsonObject{}
            .val(
                "click_to_pay",
                JsonObject{}
                    .str("dpa_id", "ctp_dpa_" + digest.substr(0, 12))
                    .str("dpa_name", "Click to Pay")
                    .str("transaction_currency", record.get_currency())
                    .str("transaction_amount", record.get_amount())
                    .done()
            )
            .done();
    }

    // ───────────────────────── Small helpers ─────────────────────────
    [[nodiscard]] std::string_view trans_status_wire(std::string_view trans_status) const
    {
        if (trans_status == "Y" || trans_status == "success" || trans_status == "authorized") {
            return "Y";
        }
        if (trans_status == "N" || trans_status == "failed" || trans_status == "denied") {
            return "N";
        }
        if (trans_status.empty()) {
            return "U";
        }
        return trans_status;
    }

    [[nodiscard]] bool is_terminal_status(std::string_view status) const
    {
        return status == "success" || status == "failed" || status == "Y" || status == "N";
    }

    // transformers.rs:227-240 AuthenticationStatus::foreign_from(TransactionStatus).
    [[nodiscard]] std::string
    authentication_status_from_trans_status(std::string_view trans_status) const
    {
        const auto wire = trans_status_wire(trans_status);
        if (wire == "Y") {
            return "success";
        }
        if (wire == "N" || wire == "U" || wire == "A" || wire == "R") {
            return "failed";
        }
        return "pending";
    }

    [[nodiscard]] std::optional<std::string> string_opt(const std::string& value) const
    {
        return value.empty() ? std::nullopt : std::optional<std::string>{value};
    }

    [[nodiscard]] ::app::dto::authentication::AuthenticationIdValue
    make_id_value(std::string_view value) const
    {
        ::app::dto::authentication::AuthenticationIdValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::MerchantIdValue
    make_merchant_value(std::string_view value) const
    {
        ::app::dto::authentication::MerchantIdValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::AuthenticationStatusValue
    make_status_value(std::string_view value) const
    {
        ::app::dto::authentication::AuthenticationStatusValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::MinorUnitValue
    make_unit_value(std::string_view value) const
    {
        ::app::dto::authentication::MinorUnitValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::CurrencyValue
    make_currency_value(std::string_view value) const
    {
        ::app::dto::authentication::CurrencyValue v{};
        v.set_value(std::string{value.empty() ? "USD" : value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::PrimitiveDateTimeValue
    make_datetime_value(std::string_view value) const
    {
        ::app::dto::authentication::PrimitiveDateTimeValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::ProfileIdValue
    make_profile_value(std::string_view value) const
    {
        ::app::dto::authentication::ProfileIdValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::SemanticVersionValue
    make_version_value(std::string_view value) const
    {
        ::app::dto::authentication::SemanticVersionValue v{};
        v.set_value(std::string{value.empty() ? "2.2.0" : value});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::TransactionStatusValue
    make_transaction_status_value(std::string_view value) const
    {
        ::app::dto::authentication::TransactionStatusValue v{};
        v.set_value(std::string{trans_status_wire(value)});
        return v;
    }

    [[nodiscard]] ::app::dto::authentication::OptionalEncryptableEmailValue
    make_email_value(std::string_view value) const
    {
        ::app::dto::authentication::OptionalEncryptableEmailValue v{};
        v.set_value(std::string{value});
        return v;
    }

    [[nodiscard]] std::string query_from_params(
        std::string_view status,
        std::string_view authentication_id,
        std::string_view client_secret,
        std::string_view amount
    ) const
    {
        return "status=" + std::string{status} +
               "&authentication_id=" + std::string{authentication_id} +
               "&authentication_client_secret=" + std::string{client_secret} +
               "&amount=" + std::string{amount} + "&manual_retry_allowed=false";
    }

    [[nodiscard]] std::string three_ds_requestor_url_for(std::string_view profile_or_merchant) const
    {
        const auto key = std::string{"authentication.three_ds_requestor_url."} +
                         std::string{profile_or_merchant};
        const auto cached = m_context.cache().get(key);
        if (cached.hit) {
            return cached.value_or<std::string>("");
        }
        // Deterministic local substitute for business_profile.authentication_connector_details.
        return std::string{"/authentication/3ds/requestor/"} + std::string{profile_or_merchant};
    }

    [[nodiscard]] std::int64_t schedule_auth_tracker(
        std::string_view operation,
        std::string_view authentication_id,
        AuthFlowKind flow
    ) const
    {
        std::string payload{operation};
        payload += ":";
        payload += authentication_id;
        payload += ":auth_flow=";
        payload += (flow == AuthFlowKind::Client ? "Client" : "Merchant");
        return m_context.process_trackers()
            .schedule("authentication_" + std::string{operation}, std::move(payload), "0");
    }

    void update_eligibility_check_trackers(
        std::string_view authentication_id,
        AuthFlowKind flow
    ) const
    {
        const auto tracker_id = schedule_auth_tracker("eligibility_check", authentication_id, flow);
        m_context.process_trackers().mark_terminal(tracker_id, std::string{authentication_id});
    }

    void publish_auth_event(
        std::string_view topic,
        std::string_view authentication_id,
        std::string_view detail
    ) const
    {
        std::string payload{"{\"authentication_id\":\""};
        payload += authentication_id;
        payload += "\",\"detail\":\"";
        payload += detail;
        payload += "\"}";
        (void)m_context.events().publish(std::string{topic}, std::move(payload));
    }

    void track_record_fetch(std::string_view operation, std::string_view authentication_id) const
    {
        if (find_authentication(authentication_id).has_value()) {
            m_context.logger()
                .counter(std::string{"authentication_record_found."} + std::string{operation})
                .increment();
        } else {
            m_context.logger()
                .counter(std::string{"authentication_record_missing."} + std::string{operation})
                .increment();
        }
    }

    [[nodiscard]] std::optional<::app::dto::authentication::CustomerDetailsValue>
    encrypted_customer_details(
        const std::optional<::app::dto::authentication::CustomerDetailsValue>& input
    ) const
    {
        if (!input.has_value()) {
            return std::nullopt;
        }
        constexpr std::string_view purpose{"authentication"};
        constexpr std::string_view key_id{"customer_details"};
        std::string wrap_key{"authentication_customer_details_local_wrap_key"};
        const auto existing = m_context.key_store().get(purpose, key_id);
        if (existing.has_value()) {
            wrap_key = existing->value();
        } else {
            (void)m_context.key_store()
                .put(purpose, key_id, "LOCAL_XOR", ::app::crypto::SecretString{wrap_key}, 0);
        }
        ::app::crypto::AesWrap wrap{};
        const std::string wrapped = wrap.encrypt(input->get_value(), wrap_key);
        ::app::dto::authentication::CustomerDetailsValue output{};
        output.set_value(::app::crypto::Base64Url{}.encode(wrapped));
        return output;
    }

    [[nodiscard]] std::string cavv_from(std::string_view reference_id) const
    {
        return ::app::crypto::Base64Url{}.encode(
            ::app::crypto::KeyedHasher{"authentication_cavv"}.hash(reference_id)
        );
    }

    [[nodiscard]] std::string created_timestamp(std::string_view authentication_id) const
    {
        return now_seconds() + ":" + std::string{authentication_id};
    }

    [[nodiscard]] std::string now_seconds() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            )
                .count()
        );
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::authentication

template<>
struct serde::Serializable<::app::handlers::authentication::AuthenticationRecordData>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::authentication::AuthenticationRecordData;
        return std::tuple{
            serde::FieldDesc<
                "authentication_id",
                &T::get_authentication_id,
                &T::set_authentication_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &T::get_merchant_id,
                &T::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &T::get_profile_id,
                &T::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_connector",
                &T::get_authentication_connector,
                &T::set_authentication_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &T::get_client_secret,
                &T::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &T::get_amount, &T::set_amount, serde::FieldOptions{}>{},
            serde::
                FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds",
                &T::get_force_3ds,
                &T::set_force_3ds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "psd2_sca_exemption_type",
                &T::get_psd2_sca_exemption_type,
                &T::set_psd2_sca_exemption_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &T::get_acquirer_bin,
                &T::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_merchant_id",
                &T::get_acquirer_merchant_id,
                &T::set_acquirer_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country_code",
                &T::get_acquirer_country_code,
                &T::set_acquirer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &T::get_return_url,
                &T::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_acquirer_id",
                &T::get_profile_acquirer_id,
                &T::set_profile_acquirer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_details",
                &T::get_customer_details,
                &T::set_customer_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "trans_status",
                &T::get_trans_status,
                &T::set_trans_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &T::get_created_at,
                &T::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_requestor_url",
                &T::get_three_ds_requestor_url,
                &T::set_three_ds_requestor_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_requestor_app_url",
                &T::get_three_ds_requestor_app_url,
                &T::set_three_ds_requestor_app_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "threeds_server_transaction_id",
                &T::get_threeds_server_transaction_id,
                &T::set_threeds_server_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_authentication_id",
                &T::get_connector_authentication_id,
                &T::set_connector_authentication_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "maximum_supported_version",
                &T::get_maximum_supported_version,
                &T::set_maximum_supported_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message_version",
                &T::get_message_version,
                &T::set_message_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "directory_server_id",
                &T::get_directory_server_id,
                &T::set_directory_server_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"eci", &T::get_eci, &T::set_eci, serde::FieldOptions{}>{},
            serde::FieldDesc<"cavv", &T::get_cavv, &T::set_cavv, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_url", &T::get_acs_url, &T::set_acs_url, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "challenge_request",
                &T::get_challenge_request,
                &T::set_challenge_request,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_reference_number",
                &T::get_acs_reference_number,
                &T::set_acs_reference_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_trans_id",
                &T::get_acs_trans_id,
                &T::set_acs_trans_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_signed_content",
                &T::get_acs_signed_content,
                &T::set_acs_signed_content,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &T::get_error_code,
                &T::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &T::get_error_message,
                &T::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &T::get_browser_info,
                &T::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &T::get_email, &T::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing", &T::get_billing, &T::set_billing, serde::FieldOptions{}>{},
            serde::
                FieldDesc<"shipping", &T::get_shipping, &T::set_shipping, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_method_url",
                &T::get_three_ds_method_url,
                &T::set_three_ds_method_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_method_data",
                &T::get_three_ds_method_data,
                &T::set_three_ds_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_metadata",
                &T::get_connector_metadata,
                &T::set_connector_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "click_to_pay_enabled",
                &T::get_click_to_pay_enabled,
                &T::set_click_to_pay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "click_to_pay_product_ids",
                &T::get_click_to_pay_product_ids,
                &T::set_click_to_pay_product_ids,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "authentication_records_domain";
    }
};

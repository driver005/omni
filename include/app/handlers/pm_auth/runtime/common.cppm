module;
#include <cctype>
#include <ctime>
export module app_handlers_pm_auth:runtime_common;

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
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::pm_auth {

struct ConnectorChoice
{
    bool key_exists{false};
    bool matched{false};
    std::string connector_name{};
    std::string mca_id{};
};

/** @brief get_connector_auth_type + PlaidAuthType BodyKey extraction substitute. */
struct ConnectorAuth
{
    std::string scheme{};
    std::string client_id{};
    std::string secret{};
    bool body_key{false};
};

/** @brief One normalized bank account returned by the connector. */
struct BankCredential
{
    std::string method{};
    std::string account_number{};
    std::string routing{};
    std::string account_type{};
    std::string account_name{};
    std::string payment_method_type{};
    std::string account_id{};
};

/** @brief Shared business helpers for the pm_auth sub-runtimes. */
class PmAuthRuntimeSupport
{
public:
    explicit PmAuthRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_payment_intent(
        std::string_view payment_id,
        std::string_view merchant_id,
        bool client_secret_present,
        std::optional<app::store::payments::PaymentIntentRecord>& out
    ) const
    {
        auto intent = m_context.repository().find_payment_by_string(payment_id);
        if (!intent) {
            if (client_secret_present) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::NOT_FOUND,
                    "payment intent not found"
                );
            }
            return std::nullopt;
        }
        if (!merchant_id.empty() && !intent->get_merchant_id().empty() &&
            std::string_view{intent->get_merchant_id()} != merchant_id) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "payment intent belongs to another merchant"
            );
        }
        const auto status = intent->get_status();
        if (status == "cancelled" || status == "canceled" || status == "failed") {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payment intent is not in a linkable state"
            );
        }
        out = std::move(intent);
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    check_payment_intent_age(std::string_view payment_id) const
    {
        const std::string key = "pm_auth.pi_seen." + std::string{payment_id};
        const std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));
        const auto cached = m_context.cache().get(key);
        if (cached.hit) {
            const std::int64_t seen = core::cache::value_as<std::int64_t>(cached.value);
            if (seen > 0 && now - seen > 86'400) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    "payment intent client secret expired"
                );
            }
            return std::nullopt;
        }
        m_context.cache().put(key, core::cache::to_cache_value(now), core::cache::Ttl{}.hours(24));
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_client_secret(
        std::string_view payment_id,
        const std::optional<std::string>& presented
    ) const
    {
        const auto cache_key = std::string{"pm_auth.client_secret_hash:"} + std::string{payment_id};
        const auto cached = m_context.cache().get(cache_key);
        if (!cached.hit || !presented.has_value()) {
            return std::nullopt;
        }
        const auto expected = cached.value_or<std::string>("");
        const app::crypto::KeyedHasher hasher{"pm_auth.client_secret"};
        if (!hasher
                 .verify_with_domain(app::crypto::HashDomain::ClientSecret, *presented, expected)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "invalid client secret"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] const interfaces::Value*
    json_child(const interfaces::Value& node, std::string_view key) const
    {
        if (const auto* object = std::get_if<interfaces::Value::Object>(&node.get())) {
            for (const auto& [name, value]: *object) {
                if (name == key) {
                    return &value;
                }
            }
            for (const auto& [name, value]: *object) {
                if (const auto* nested = json_child(value, key)) {
                    return nested;
                }
            }
        } else if (const auto* array = std::get_if<interfaces::Value::Array>(&node.get())) {
            for (const auto& value: *array) {
                if (const auto* nested = json_child(value, key)) {
                    return nested;
                }
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::optional<std::string>
    json_string(const interfaces::Value& node, std::string_view key) const
    {
        const auto* child = json_child(node, key);
        if (child == nullptr) {
            return std::nullopt;
        }
        if (const auto* scalar = std::get_if<std::string>(&child->get())) {
            return *scalar;
        }
        if (const auto* wrapped = std::get_if<interfaces::Value::Object>(&child->get())) {
            for (const auto& [name, value]: *wrapped) {
                if (name == "value") {
                    if (const auto* inner = std::get_if<std::string>(&value.get())) {
                        return *inner;
                    }
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string derive_billing_country(
        const std::optional<app::store::payments::PaymentIntentRecord>& intent,
        const std::optional<app::store::merchant_connectors::MerchantConnectorRecord>& mca
    ) const
    {
        std::string country{};
        if (intent.has_value()) {
            const auto& metadata = intent->get_metadata();
            if (!metadata.empty()) {
                if (const auto document =
                        serde::Ser::decode_generic("application/json", metadata)) {
                    if (const auto value = json_string(*document, "billing_country")) {
                        country = *value;
                    } else if (const auto* address = json_child(*document, "billing_address")) {
                        if (const auto value = json_string(*address, "country")) {
                            country = *value;
                        }
                    }
                }
            }
        }
        if (country.empty() && mca.has_value() && !mca->get_business_country().empty()) {
            country = mca->get_business_country();
        }
        // No billing_address_id column exists on the local PI record, so an absent
        // address yields no country_codes entry instead of upstream MissingRequiredField.
        return country;
    }

    [[nodiscard]] ConnectorAuth resolve_connector_auth(
        const std::optional<app::store::merchant_connectors::MerchantConnectorRecord>& mca
    ) const
    {
        ConnectorAuth result{};
        if (!mca.has_value()) {
            return result;
        }
        result.scheme =
            mca->get_connector_type().empty() ? std::string{"body_key"} : mca->get_connector_type();
        const auto details = mca->get_connector_account_details();
        if (!details.empty()) {
            if (const auto document = serde::Ser::decode_generic("application/json", details)) {
                if (const auto value = json_string(*document, "api_key")) {
                    result.client_id = *value;
                }
                if (const auto value = json_string(*document, "key1")) {
                    result.secret = *value;
                }
                if (result.client_id.empty()) {
                    if (const auto value = json_string(*document, "client_id")) {
                        result.client_id = *value;
                    }
                }
                if (result.secret.empty()) {
                    if (const auto value = json_string(*document, "secret")) {
                        result.secret = *value;
                    }
                }
                if (const auto value = json_string(*document, "auth_type")) {
                    result.scheme = *value;
                }
            }
        }
        if (result.client_id.empty() || result.secret.empty()) {
            if (const auto stored =
                    m_context.key_store().get("connector-auth", mca->get_merchant_connector_id())) {
                if (const auto document =
                        serde::Ser::decode_generic("application/json", stored->value())) {
                    if (result.client_id.empty()) {
                        if (const auto value = json_string(*document, "api_key")) {
                            result.client_id = *value;
                        }
                    }
                    if (result.secret.empty()) {
                        if (const auto value = json_string(*document, "key1")) {
                            result.secret = *value;
                        }
                    }
                }
            }
        }
        result.body_key = !result.client_id.empty() && !result.secret.empty();
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError> connector_failure(
        const app::connector::ConnectorResponse& response,
        std::string_view connector_name
    ) const
    {
        const auto lower = [](std::string_view text)
        {
            std::string out{};
            out.reserve(text.size());
            for (const char c: text) {
                out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            return out;
        };
        const std::string status = lower(response.get_status());
        const std::string message = lower(response.get_message());
        const bool failed = status.find("error") != std::string::npos ||
                            status.find("fail") != std::string::npos ||
                            message.find("error") != std::string::npos;
        if (!failed) {
            return std::nullopt;
        }
        std::string detail{connector_name};
        if (!response.get_message().empty()) {
            detail += ": " + response.get_message();
        }
        return core::error::AppErrorBuilder{}(core::error::AppErrorKind::CONNECTOR_ERROR, detail);
    }

    [[nodiscard]] std::string
    connector_value(const app::connector::ConnectorResponse& response) const
    {
        return response.get_raw_json().empty() ? response.get_status() : response.get_raw_json();
    }

    [[nodiscard]] std::vector<BankCredential> parse_bank_credentials(const std::string& raw) const
    {
        std::vector<BankCredential> result{};
        if (raw.empty()) {
            return result;
        }
        const auto document = serde::Ser::decode_generic("application/json", raw);
        if (!document) {
            return result;
        }
        const interfaces::Value::Array* credentials{nullptr};
        if (const auto* object = std::get_if<interfaces::Value::Object>(&document->get())) {
            for (const auto& [name, value]: *object) {
                if (name == "credentials") {
                    credentials = std::get_if<interfaces::Value::Array>(&value.get());
                    break;
                }
            }
        } else if (const auto* array = std::get_if<interfaces::Value::Array>(&document->get())) {
            credentials = array;
        }
        if (credentials == nullptr) {
            return result;
        }
        for (const auto& element: *credentials) {
            const auto* object = std::get_if<interfaces::Value::Object>(&element.get());
            if (object == nullptr) {
                continue;
            }
            BankCredential credential{};
            credential.account_id = json_string(element, "account_id").value_or("");
            credential.account_type = json_string(element, "account_type").value_or("");
            credential.account_name = json_string(element, "account_name").value_or("");
            credential.payment_method_type =
                json_string(element, "payment_method_type").value_or("bank_debit");
            const auto* details = json_child(element, "account_details");
            if (details != nullptr) {
                for (const auto* method: {"ach", "bacs", "sepa"}) {
                    if (json_child(*details, method) == nullptr) {
                        continue;
                    }
                    credential.method = method;
                    if (std::string_view{method} == "sepa") {
                        credential.account_number = json_string(*details, "iban").value_or("");
                    } else {
                        credential.account_number =
                            json_string(*details, "account_number").value_or("");
                    }
                    credential.routing = std::string{method} == "bacs"
                                             ? json_string(*details, "sort_code").value_or("")
                                             : json_string(*details, "routing_number").value_or("");
                    break;
                }
            }
            if (!credential.account_number.empty()) {
                result.push_back(std::move(credential));
            }
        }
        return result;
    }

    [[nodiscard]] std::string bank_hash(const BankCredential& credential) const
    {
        std::string material{};
        if (credential.method == "ach") {
            material = credential.account_number + "-" + credential.routing + "-ach";
        } else if (credential.method == "bacs") {
            material = credential.account_number + "-" + credential.routing + "-bacs";
        } else if (credential.method == "sepa") {
            material = credential.account_number + "-sepa";
        } else {
            material = credential.account_number;
        }
        // core/pm_auth.rs:370-425 — HMAC-SHA256 over the account/route key.
        return app::crypto::KeyedHasher{"pm_auth-bank-dedupe"}.hash_with_domain(
            "pm_auth_bank_dedupe",
            material
        );
    }

    [[nodiscard]] std::string mask_account(std::string_view account_number) const
    {
        if (account_number.size() <= 4) {
            return std::string{account_number};
        }
        return std::string{account_number.substr(account_number.size() - 4)};
    }

    [[nodiscard]] std::string bank_details_json(
        const BankCredential& credential,
        std::string_view hash,
        std::string_view connectors
    ) const
    {
        return "{\"hash\":" + serde::Ser::encode_json(std::string{hash}) +
               ",\"method\":" + serde::Ser::encode_json(credential.method) +
               ",\"mask\":" + serde::Ser::encode_json(mask_account(credential.account_number)) +
               ",\"account_type\":" + serde::Ser::encode_json(credential.account_type) +
               ",\"account_name\":" + serde::Ser::encode_json(credential.account_name) +
               ",\"payment_method_type\":" +
               serde::Ser::encode_json(credential.payment_method_type) +
               ",\"account_id\":" + serde::Ser::encode_json(credential.account_id) +
               ",\"account_number\":" + serde::Ser::encode_json(credential.account_number) +
               ",\"routing\":" + serde::Ser::encode_json(credential.routing) +
               ",\"connectors\":" + serde::Ser::encode_json(std::string{connectors}) + "}";
    }

    [[nodiscard]] std::string json_field(const std::string& document, std::string_view key) const
    {
        const auto parsed = serde::Ser::decode_generic("application/json", document);
        if (!parsed) {
            return {};
        }
        return json_string(*parsed, key).value_or("");
    }

    [[nodiscard]] std::string merge_connectors(
        std::string_view existing,
        std::string_view connector_name,
        std::string_view mca_id,
        std::string_view access_token,
        std::string_view account_id
    ) const
    {
        const std::string entry = std::string{connector_name} + "|" + std::string{mca_id} + "|" +
                                  std::string{access_token} + "|" + std::string{account_id};
        std::string result{};
        if (!mca_id.empty()) {
            const std::string prefix = std::string{mca_id} + "|";
            std::size_t start{0};
            while (start < existing.size()) {
                const std::size_t end = existing.find(';', start);
                const std::string item = std::string{existing.substr(
                    start,
                    end == std::string_view::npos ? std::string_view::npos : end - start
                )};
                if (item.rfind(prefix, 0) != 0) {
                    if (!result.empty()) {
                        result.push_back(';');
                    }
                    result += item;
                }
                if (end == std::string_view::npos) {
                    break;
                }
                start = end + 1;
            }
        }
        if (!result.empty()) {
            result.push_back(';');
        }
        result += entry;
        return result;
    }

    void store_bank_details(
        std::string_view merchant_id,
        const std::optional<app::store::payments::PaymentIntentRecord>& intent,
        std::string_view connector_name,
        std::string_view mca_id,
        std::string_view profile_id,
        std::string_view access_token,
        const std::vector<BankCredential>& credentials
    ) const
    {
        if (credentials.empty() || !intent.has_value() || intent->get_customer_id().empty()) {
            return;
        }
        const std::string customer_id = intent->get_customer_id();
        auto repository = m_context.repository();
        std::map<std::string, app::store::payment_methods::PaymentMethodRecord> by_hash{};
        for (const auto& method: repository.list_payment_methods()) {
            if (std::string_view{method.get_merchant_id()} != merchant_id ||
                method.get_customer_id() != customer_id || method.get_vault_id().empty()) {
                continue;
            }
            const std::string plaintext =
                app::crypto::AesWrap{}.decrypt(method.get_vault_id(), "pm_auth-bank-detail-key");
            const std::string hash = json_field(plaintext, "hash");
            if (!hash.empty()) {
                by_hash[hash] = method;
            }
        }
        std::vector<app::store::payment_methods::PaymentMethodRecord> update_entries{};
        std::vector<app::store::payment_methods::PaymentMethodRecord> new_entries{};
        for (const auto& credential: credentials) {
            const std::string hash = bank_hash(credential);
            const auto found = by_hash.find(hash);
            if (found != by_hash.end()) {
                auto method = found->second;
                const std::string plaintext = app::crypto::AesWrap{}.decrypt(
                    method.get_vault_id(),
                    "pm_auth-bank-detail-key"
                );
                const std::string connectors = merge_connectors(
                    json_field(plaintext, "connectors"),
                    connector_name,
                    mca_id,
                    access_token,
                    credential.account_id
                );
                method.set_vault_id(
                    encrypt_secret(bank_details_json(credential, hash, connectors))
                );
                method.set_last4(mask_account(credential.account_number));
                update_entries.push_back(std::move(method));
                continue;
            }
            app::store::payment_methods::PaymentMethodRecord method{};
            method.set_payment_method_id(m_context.ids().payment_id("pm_"));
            method.set_merchant_id(std::string{merchant_id});
            method.set_customer_id(customer_id);
            method.set_profile_id(std::string{profile_id});
            method.set_type(
                credential.payment_method_type.empty() ? std::string{"bank_debit"}
                                                       : credential.payment_method_type
            );
            method.set_network(credential.method);
            method.set_last4(mask_account(credential.account_number));
            const std::string connectors =
                merge_connectors({}, connector_name, mca_id, access_token, credential.account_id);
            method.set_vault_id(encrypt_secret(bank_details_json(credential, hash, connectors)));
            method.set_status("active");
            method.set_created_at(std::to_string(static_cast<std::int64_t>(std::time(nullptr))));
            new_entries.push_back(std::move(method));
        }
        store_in_db(update_entries, new_entries);
    }

    /** @brief core/pm_auth.rs:579-616 store_in_db — updates and inserts applied as two
     *  batches. The local repository is synchronous, so the two join_all sets become two
     *  loops; per-item errors are logged upstream and do not abort the batch.
     */
    void store_in_db(
        std::vector<app::store::payment_methods::PaymentMethodRecord>& update_entries,
        std::vector<app::store::payment_methods::PaymentMethodRecord>& new_entries
    ) const
    {
        auto repository = m_context.repository();
        for (auto& method: update_entries) {
            repository.update_payment_method(method);
        }
        for (auto& method: new_entries) {
            repository.save_payment_method(std::move(method));
        }
    }

    void parse_choice_config(
        std::string_view json,
        std::string_view payment_method,
        std::string_view payment_method_type,
        std::string& connector_name,
        std::string& mca_id
    ) const
    {
        const auto doc = serde::Ser::decode_generic("application/json", std::string{json});
        if (!doc) {
            return;
        }
        const interfaces::Value::Array* choices{nullptr};
        if (const auto* array = std::get_if<interfaces::Value::Array>(&doc->get())) {
            choices = array;
        } else if (const auto* object = std::get_if<interfaces::Value::Object>(&doc->get())) {
            for (const auto& [name, field]: *object) {
                if (name == "enabled_payment_methods") {
                    choices = std::get_if<interfaces::Value::Array>(&field.get());
                    break;
                }
            }
        }
        if (choices == nullptr) {
            return;
        }
        for (const auto& element: *choices) {
            const auto* choice = std::get_if<interfaces::Value::Object>(&element.get());
            if (!choice) {
                continue;
            }
            std::string choice_method;
            std::string choice_type;
            std::string choice_connector;
            std::string choice_mca;
            for (const auto& [name, field]: *choice) {
                std::string text;
                if (const auto* scalar = std::get_if<std::string>(&field.get())) {
                    text = *scalar;
                } else if (
                    const auto* wrapped = std::get_if<interfaces::Value::Object>(&field.get())
                ) {
                    for (const auto& [inner_name, inner]: *wrapped) {
                        if (inner_name == "value") {
                            if (const auto* inner_text = std::get_if<std::string>(&inner.get())) {
                                text = *inner_text;
                            }
                        }
                    }
                }
                if (name == "payment_method") {
                    choice_method = text;
                } else if (name == "payment_method_type") {
                    choice_type = text;
                } else if (name == "connector_name") {
                    choice_connector = text;
                } else if (name == "mca_id") {
                    choice_mca = text;
                }
            }
            if (std::string_view{choice_method} == payment_method &&
                std::string_view{choice_type} == payment_method_type && !choice_connector.empty()) {
                connector_name = std::move(choice_connector);
                mca_id = std::move(choice_mca);
                return;
            }
        }
    }

    [[nodiscard]] ConnectorChoice resolve_connector_choice(
        std::string_view merchant_id,
        std::string_view payment_id,
        std::string_view payment_method,
        std::string_view payment_method_type
    ) const
    {
        ConnectorChoice result{};
        const std::string config_key{"pm_auth_" + std::string{payment_id}};
        const std::string legacy_key{"pm_auth.config." + std::string{payment_id}};

        std::string config_json;
        auto hit = m_context.cache().get(config_key);
        if (hit.hit) {
            result.key_exists = true;
            config_json = core::cache::value_as<std::string>(hit.value);
        } else {
            hit = m_context.cache().get(legacy_key);
            if (hit.hit) {
                result.key_exists = true;
                config_json = core::cache::value_as<std::string>(hit.value);
            }
        }
        if (!config_json.empty()) {
            parse_choice_config(
                config_json,
                payment_method,
                payment_method_type,
                result.connector_name,
                result.mca_id
            );
            result.matched = !result.connector_name.empty();
        }

        if (!result.matched) {
            auto repository = m_context.repository();
            for (const auto& mca: repository.list_merchant_connectors()) {
                if (!merchant_id.empty() &&
                    std::string_view{mca.get_merchant_id()} != merchant_id) {
                    continue;
                }
                const auto& persisted = mca.get_pm_auth_config();
                if (persisted.empty()) {
                    continue;
                }
                std::string name;
                std::string id;
                parse_choice_config(persisted, payment_method, payment_method_type, name, id);
                if (!name.empty()) {
                    result.key_exists = true;
                    result.matched = true;
                    result.connector_name = std::move(name);
                    result.mca_id = std::move(id);
                    if (result.mca_id.empty()) {
                        result.mca_id = mca.get_merchant_connector_id();
                    }
                    break;
                }
            }
        }

        // core/pm_auth.rs:76-93,726-769 — no synthetic connector choice: upstream only
        // selects from the stored Vec<PaymentMethodAuthConnectorChoice> and errors when no
        // entry matches the requested payment_method / payment_method_type.
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_merchant_connector(
        std::string_view merchant_id,
        std::string_view mca_id,
        std::string& connector_name,
        std::optional<app::store::merchant_connectors::MerchantConnectorRecord>& out
    ) const
    {
        if (mca_id.empty()) {
            return std::nullopt;
        }
        const auto record = m_context.repository().find_merchant_connector_by_string(mca_id);
        if (!record) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant connector account not found"
            );
        }
        if (!merchant_id.empty() && std::string_view{record->get_merchant_id()} != merchant_id) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant connector account belongs to another merchant"
            );
        }
        if (!record->get_connector_name().empty()) {
            connector_name = record->get_connector_name();
        }
        out = record;
        return std::nullopt;
    }

    [[nodiscard]] std::string generate_token(std::string_view prefix, std::string_view salt) const
    {
        const app::crypto::SecretString raw{
            std::string{prefix} + std::string{salt} + "_" + m_context.ids().uuid_v4()
        };
        const auto digest = app::crypto::KeyedHasher{"pm_auth.token"}.hash(raw.value());
        return std::string{prefix} + digest;
    }

    [[nodiscard]] std::string encrypt_secret(std::string_view value) const
    {
        return app::crypto::AesWrap{}.encrypt(value, "pm_auth-bank-detail-key");
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::pm_auth

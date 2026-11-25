export module app_handlers_superposition_sdk_config:runtime_common;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_model;
import app_store;
import app_auth;
import app_crypto;
import app_metrics;
import app_services;
import core_cache;

export namespace app::handlers::superposition_sdk_config {

using PmKey = std::pair<std::string, std::string>;
using ConnectorList = std::vector<std::string>;

struct SdkPaymentContext
{
    std::optional<app::store::payment_attempts::PaymentAttemptRecord> payment_attempt{};
    bool is_cit_transaction{false};
};

// Upstream MerchantEnabledPmsContext (core/payment_methods/cards.rs). The five maps mirror
// the upstream fields; required_fields/pmt_to_auth_connector/installment/wallet/sdk_next_action
// are empty defaults in both handlers at these call sites (core:274-289).
struct MerchantEnabledPmsContext
{
    std::map<PmKey, std::map<std::string, ConnectorList>> payment_experiences{};
    std::map<PmKey, std::map<std::string, ConnectorList>> card_networks{};
    std::map<std::string, ConnectorList> banks{};
    std::map<std::string, ConnectorList> bank_debits{};
    std::map<std::string, ConnectorList> bank_transfers{};

    // Upstream get_eligible_connectors(): every connector referenced by the context.
    [[nodiscard]] ConnectorList get_eligible_connectors() const
    {
        ConnectorList result{};
        const auto add = [&result](const ConnectorList& connectors)
        {
            for (const auto& connector: connectors) {
                if (std::find(result.begin(), result.end(), connector) == result.end()) {
                    result.push_back(connector);
                }
            }
        };
        for (const auto& [key, pe_map]: payment_experiences) {
            (void)key;
            for (const auto& [experience, connectors]: pe_map) {
                (void)experience;
                add(connectors);
            }
        }
        for (const auto& [key, network_map]: card_networks) {
            (void)key;
            for (const auto& [network, connectors]: network_map) {
                (void)network;
                add(connectors);
            }
        }
        for (const auto& [pmt, connectors]: banks) {
            (void)pmt;
            add(connectors);
        }
        for (const auto& [pmt, connectors]: bank_debits) {
            (void)pmt;
            add(connectors);
        }
        for (const auto& [pmt, connectors]: bank_transfers) {
            (void)pmt;
            add(connectors);
        }
        return result;
    }
};

struct SdkBankCode
{
    std::vector<std::string> bank_names{};
    ConnectorList eligible_connectors{};
};

struct DimensionFilter
{
    serde::Value value{};
    std::string key{};
};

/**
 * @brief Shared business helpers for the superposition_sdk_config sub-runtimes.
 *
 * External infrastructure (the superposition HTTP service, the bank catalogue, the address
 * store) is replaced by in-process facilities that implement the same control flow against
 * AppContext/AppRepository/SuperpositionClient, as required by the local parity contract.
 */
class SuperpositionSdkConfigRuntimeSupport
{
public:
    explicit SuperpositionSdkConfigRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── payment id / context ─────────────────────────

    // Upstream core::get_payment_context (core:35-137). Attempt lookup, customer and mandate
    // state map onto AppRepository records; the local PaymentIntentRecord has no
    // shipping/billing address ids or setup_future_usage column, and no address store exists,
    // so is_cit_transaction uses the mandate-present branch of upstream's OR expression.
    [[nodiscard]] SdkPaymentContext get_payment_context(
        std::string_view payment_id,
        const app::store::payments::PaymentIntentRecord& payment
    ) const
    {
        SdkPaymentContext ctx{};
        ctx.payment_attempt = find_payment_attempt(payment_id);
        const bool has_customer =
            !payment.get_customer_id().empty() &&
            m_context.repository().find_customer_by_string(payment.get_customer_id()).has_value();
        const bool has_mandate =
            has_customer &&
            !m_context.repository().find_mandates_by_customer_id(payment.get_customer_id()).empty();
        ctx.is_cit_transaction = has_mandate;
        return ctx;
    }

    [[nodiscard]] std::optional<app::store::payment_attempts::PaymentAttemptRecord>
    find_payment_attempt(std::string_view payment_id) const
    {
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<app::store::merchant_connectors::MerchantConnectorRecord>
    list_enabled_merchant_connector_accounts(
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        std::vector<app::store::merchant_connectors::MerchantConnectorRecord> result{};
        for (const auto& mca: m_context.repository().list_merchant_connectors()) {
            if (mca.get_disabled() || (!mca.get_status().empty() && mca.get_status() != "active")) {
                continue;
            }
            if (!merchant_id.empty() && mca.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id.empty() && !mca.get_profile_id().empty() &&
                mca.get_profile_id() != profile_id) {
                continue;
            }
            result.push_back(mca);
        }
        return result;
    }

    // ───────────────────────── json helpers ─────────────────────────

    [[nodiscard]] std::optional<serde::Value> parse_json(std::string_view text) const
    {
        if (text.empty()) {
            return std::nullopt;
        }
        auto decoded = serde::Ser::decode_generic("application/json", text);
        if (!decoded) {
            return std::nullopt;
        }
        return std::move(*decoded);
    }

    [[nodiscard]] std::string json_string(const serde::Value& value) const
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            return *text;
        }
        return {};
    }

    [[nodiscard]] std::string
    json_string_field(const std::optional<serde::Value>& object, std::string_view key) const
    {
        if (!object) {
            return {};
        }
        return json_string_field(*object, key);
    }

    [[nodiscard]] std::string
    json_string_field(const serde::Value& value, std::string_view key) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return {};
        }
        return json_string(field.value());
    }

    [[nodiscard]] std::vector<serde::Value>
    json_array(const serde::Value& value, std::string_view key) const
    {
        std::vector<serde::Value> result{};
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return result;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return result;
        }
        const auto* array = std::get_if<serde::Value::Array>(&field.value().get());
        if (array == nullptr) {
            return result;
        }
        for (const auto& element: *array) {
            result.push_back(element);
        }
        return result;
    }

    [[nodiscard]] std::string json_quote(std::string_view value) const
    {
        std::string out{"\""};
        for (const char ch: value) {
            switch (ch) {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out.push_back(ch);
                    break;
            }
        }
        out.push_back('"');
        return out;
    }

    // ───────────────────────── PM context / translation ─────────────────────────

    void add_connector(ConnectorList& connectors, std::string_view connector) const
    {
        if (connector.empty() ||
            std::find(connectors.begin(), connectors.end(), connector) != connectors.end()) {
            return;
        }
        connectors.emplace_back(connector);
    }

    // Upstream build_merchant_enabled_pms_context (core:172-289, 386-397). MCA
    // payment_methods_enabled JSON is consolidated into payment-experience / card-network /
    // bank maps keyed exactly like the upstream HashMaps.
    [[nodiscard]] MerchantEnabledPmsContext build_merchant_enabled_pms_context(
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& mcas
    ) const
    {
        MerchantEnabledPmsContext ctx{};
        for (const auto& mca: mcas) {
            const auto parsed = parse_json(mca.get_payment_methods_enabled());
            if (!parsed) {
                continue;
            }
            const auto* methods = std::get_if<serde::Value::Array>(&parsed->get());
            if (methods == nullptr) {
                continue;
            }
            const std::string connector = mca.get_connector_name();
            for (const auto& method_value: *methods) {
                const std::string payment_method =
                    json_string_field(method_value, "payment_method");
                if (payment_method.empty()) {
                    continue;
                }
                for (const auto& type_value: json_array(method_value, "payment_method_types")) {
                    const std::string pmt = json_string_field(type_value, "payment_method_type");
                    if (pmt.empty()) {
                        continue;
                    }
                    const PmKey key{payment_method, pmt};

                    const std::string experience =
                        json_string_field(type_value, "payment_experience");
                    if (!experience.empty()) {
                        add_connector(ctx.payment_experiences[key][experience], connector);
                    }
                    for (const auto& network: json_array(type_value, "card_networks")) {
                        const std::string name = json_string(network);
                        if (!name.empty()) {
                            add_connector(ctx.card_networks[key][name], connector);
                        }
                    }
                    if (payment_method == "bank_redirect") {
                        add_connector(ctx.banks[pmt], connector);
                    } else if (payment_method == "bank_debit") {
                        add_connector(ctx.bank_debits[pmt], connector);
                    } else if (payment_method == "bank_transfer") {
                        add_connector(ctx.bank_transfers[pmt], connector);
                    }
                }
            }
        }
        return ctx;
    }

    // Upstream get_banks(state, pmt, connectors) (core/payment_methods/cards.rs). No bank
    // catalogue is exposed by AppContext; each enabled connector contributes one deterministic
    // bank name. A real catalogue should replace this local substitute (shared contract request).
    [[nodiscard]] std::vector<SdkBankCode>
    get_banks(std::string_view /*payment_method_type*/, const ConnectorList& connectors) const
    {
        std::vector<SdkBankCode> result{};
        for (const auto& connector: connectors) {
            result.push_back(SdkBankCode{{connector}, {connector}});
        }
        return result;
    }

    // Upstream format_criteria_value (core:542-562).
    [[nodiscard]] std::string format_criteria_value(
        std::optional<std::string> card_network,
        std::optional<std::string> bank_name,
        std::optional<std::string> payment_experience
    ) const
    {
        std::vector<std::string> parts{};
        if (card_network && !card_network->empty()) {
            parts.push_back(*card_network);
        }
        if (bank_name && !bank_name->empty()) {
            parts.push_back(*bank_name);
        }
        if (payment_experience && !payment_experience->empty()) {
            parts.push_back(*payment_experience);
        }
        if (parts.empty()) {
            return "default";
        }
        std::string out{};
        for (std::size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) {
                out.push_back('/');
            }
            out += parts[i];
        }
        return out;
    }

    // Upstream translate_to_sdk_payment_methods (core:564-688). The criteria is fixed by the
    // first source that inserts a (payment_method, payment_method_type) key: experiences, then
    // card networks, then banks, bank debits and bank transfers. Types are sorted by
    // payment_method_type and methods by payment_method, exactly as upstream does.
    [[nodiscard]] std::vector<app::dto::superposition_sdk_config::SdkPaymentMethod>
    translate_to_sdk_payment_methods(const MerchantEnabledPmsContext& pms_ctx) const
    {
        using namespace app::dto::superposition_sdk_config;

        struct Rules
        {
            std::optional<PaymentMethodCriteria> criteria{};
            std::vector<SdkCriteriaRule> rules{};
        };

        std::map<PmKey, Rules> consolidated{};
        const auto entry_for =
            [&consolidated](PmKey key, std::optional<PaymentMethodCriteria> criteria) -> Rules&
        {
            auto [it, inserted] = consolidated.try_emplace(std::move(key));
            if (inserted) {
                it->second.criteria = criteria;
            }
            return it->second;
        };

        // 1. payment experiences.
        for (const auto& [key, pe_map]: pms_ctx.payment_experiences) {
            auto& rules = entry_for(key, PaymentMethodCriteria::PaymentExperience);
            for (const auto& [experience, connectors]: pe_map) {
                rules.rules.push_back(
                    SdkCriteriaRule{
                        format_criteria_value(std::nullopt, std::nullopt, experience),
                        connectors
                    }
                );
            }
        }
        // 2. card networks.
        for (const auto& [key, network_map]: pms_ctx.card_networks) {
            auto& rules = entry_for(key, PaymentMethodCriteria::CardNetwork);
            for (const auto& [network, connectors]: network_map) {
                rules.rules.push_back(
                    SdkCriteriaRule{
                        format_criteria_value(network, std::nullopt, std::nullopt),
                        connectors
                    }
                );
            }
        }
        // 3. banks (bank redirect).
        for (const auto& [pmt, connectors]: pms_ctx.banks) {
            auto& rules = entry_for(PmKey{"bank_redirect", pmt}, PaymentMethodCriteria::BankName);
            bool has_banks{false};
            for (const auto& bank_code: get_banks(pmt, connectors)) {
                for (const auto& bank_name: bank_code.bank_names) {
                    has_banks = true;
                    rules.rules.push_back(
                        SdkCriteriaRule{
                            format_criteria_value(std::nullopt, bank_name, std::nullopt),
                            bank_code.eligible_connectors
                        }
                    );
                }
            }
            if (!has_banks) {
                rules.criteria = std::nullopt;
                rules.rules.push_back(
                    SdkCriteriaRule{
                        format_criteria_value(std::nullopt, std::nullopt, std::nullopt),
                        connectors
                    }
                );
            }
        }
        // 4. bank debits.
        for (const auto& [pmt, connectors]: pms_ctx.bank_debits) {
            auto& rules = entry_for(PmKey{"bank_debit", pmt}, std::nullopt);
            rules.rules.push_back(
                SdkCriteriaRule{
                    format_criteria_value(std::nullopt, std::nullopt, std::nullopt),
                    connectors
                }
            );
        }
        // 5. bank transfers.
        for (const auto& [pmt, connectors]: pms_ctx.bank_transfers) {
            auto& rules = entry_for(PmKey{"bank_transfer", pmt}, std::nullopt);
            rules.rules.push_back(
                SdkCriteriaRule{
                    format_criteria_value(std::nullopt, std::nullopt, std::nullopt),
                    connectors
                }
            );
        }

        std::map<std::string, std::vector<SdkPaymentMethodType>> methods_map{};
        for (const auto& [key, rules]: consolidated) {
            if (rules.rules.empty()) {
                continue;
            }
            SdkPaymentMethodType type{};
            PaymentMethodTypeValue type_value{};
            type_value.set_value(key.second);
            type.set_payment_method_type(std::move(type_value));
            type.set_payment_method_criteria(rules.criteria);
            type.set_criteria_rules(rules.rules);
            methods_map[key.first].push_back(std::move(type));
        }

        std::vector<SdkPaymentMethod> result{};
        for (auto& [payment_method, types]: methods_map) {
            std::sort(
                types.begin(),
                types.end(),
                [](const auto& lhs, const auto& rhs)
                {
                    return lhs.get_payment_method_type().get_value() <
                           rhs.get_payment_method_type().get_value();
                }
            );
            SdkPaymentMethod method{};
            PaymentMethodValue method_value{};
            method_value.set_value(payment_method);
            method.set_payment_method(std::move(method_value));
            method.set_payment_method_types(std::move(types));
            result.push_back(std::move(method));
        }
        return result;
    }

    // ───────────────────────── dimension filter / superposition ─────────────────────────

    [[nodiscard]] std::string
    organization_id(std::string_view merchant_id, const app::auth::AuthContext& auth_ctx) const
    {
        if (!auth_ctx.org_id().empty()) {
            return auth_ctx.org_id();
        }
        const auto merchant = m_context.repository().find_merchant_account_by_string(merchant_id);
        return merchant ? merchant->get_organization_id() : std::string{};
    }

    [[nodiscard]] std::vector<std::string> enabled_connectors(
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& mcas
    ) const
    {
        ConnectorList result{};
        for (const auto& mca: mcas) {
            add_connector(result, mca.get_connector_name());
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    // Upstream dimension filter (core:291-322, 399-433). [DYNAMIC_FIELDS, CHECKOUT_SDK] is
    // represented by the namespace/key pair passed to SuperpositionClient::resolve below.
    [[nodiscard]] DimensionFilter build_dimension_filter(
        std::string_view profile_id,
        std::string_view processor_merchant_id,
        std::string_view provider_merchant,
        std::string_view organization,
        const std::vector<std::string>& connectors
    ) const
    {
        std::string connectors_json{"["};
        for (std::size_t i = 0; i < connectors.size(); ++i) {
            if (i > 0) {
                connectors_json.push_back(',');
            }
            connectors_json += json_quote(connectors[i]);
        }
        connectors_json.push_back(']');

        const std::string json = std::format(
            R"({{"profile_id":{},"processor_merchant_id":{},"provider_merchant_id":{},"organization_id":{},"connector":{}}})",
            json_quote(profile_id),
            json_quote(processor_merchant_id),
            json_quote(provider_merchant),
            json_quote(organization),
            connectors_json
        );
        DimensionFilter filter{};
        auto decoded = parse_json(json);
        filter.value = decoded ? std::move(*decoded) : serde::Value{};
        // get_cached_config keys the superposition lookup by the dimension filter context; the
        // deterministic scoped hash is the local in-process equivalent.
        filter.key = app::crypto::KeyedHasher{"superposition_sdk_config"}.hash(json);
        return filter;
    }

    // Local substitute for the superposition service's eligibility dimension: the in-process
    // client scopes a config by a single context string, so CIT state is appended to the hash.
    [[nodiscard]] std::string
    filter_key_with_cit(std::string_view dimension_key, bool is_cit_transaction) const
    {
        return is_cit_transaction ? std::string{dimension_key} + ":cit"
                                  : std::string{dimension_key};
    }

    // Upstream superposition_service.get_cached_config([DYNAMIC_FIELDS, CHECKOUT_SDK], filter)
    // (core:324-336, 435-447). SuperpositionClient is the repository-backed local service.
    // The stored document is interpreted as superposition_types::Config
    // (docs.rs 0.119.0: contexts, overrides, default_configs, dimensions); a bare config object
    // is placed in default_configs with the other three fields at their upstream Default values.
    [[nodiscard]] app::dto::superposition_sdk_config::Config
    resolve_raw_config(std::string_view dimension_key) const
    {
        using Config = app::dto::superposition_sdk_config::Config;
        Config config{};
        const auto record =
            m_context.superposition().resolve("dynamic_fields", "checkout_sdk", dimension_key);
        const std::string stored =
            (record && !record->get_value().empty()) ? record->get_value() : std::string{R"({})"};
        const auto decoded = parse_json(stored);
        if (!decoded) {
            return config;
        }

        const auto field = [&decoded](std::string_view key) -> std::optional<serde::Value>
        {
            if (const auto* object = std::get_if<serde::Value::Object>(&decoded->get())) {
                const auto found = object->get(std::string{key});
                if (found && !std::holds_alternative<std::nullopt_t>(found->get())) {
                    return found;
                }
            }
            return std::nullopt;
        };

        if (const auto contexts = field("contexts")) {
            config.set_contexts(*contexts);
        }
        if (const auto overrides = field("overrides")) {
            config.set_overrides(*overrides);
        }
        if (const auto dimensions = field("dimensions")) {
            config.set_dimensions(*dimensions);
        }
        if (const auto defaults = field("default_configs")) {
            config.set_default_configs(*defaults);
        } else {
            config.set_default_configs(*decoded);
        }
        return config;
    }

    // ───────────────────────── account config ─────────────────────────

    // Upstream get_feature_config (core/utils.rs) and get_should_perform_sdk_vaulting
    // (core/payment_methods/utils.rs), resolved from the local feature-config store.
    [[nodiscard]] bool
    feature_flag(std::string_view config_namespace, std::string_view key, bool fallback) const
    {
        const auto record = m_context.superposition().get_config(config_namespace, key);
        if (!record) {
            return fallback;
        }
        return record->get_value() == "true" || record->get_value() == "1";
    }

    [[nodiscard]] bool should_perform_sdk_vaulting(std::string_view organization_id) const
    {
        return feature_flag(
            "feature_config",
            std::string{organization_id} + ".should_perform_sdk_vaulting",
            true
        );
    }

    [[nodiscard]] bool is_payment_method_modular_allowed(std::string_view merchant_id) const
    {
        return feature_flag(
            "feature_config",
            std::string{merchant_id} + ".is_payment_method_modular_allowed",
            true
        );
    }

    // Upstream resolve_vaulting_action (core:531-540).
    [[nodiscard]] app::dto::superposition_sdk_config::VaultingAction
    resolve_vaulting_action(bool should_call_modular, bool should_perform_sdk_vaulting_flag) const
    {
        using VaultingAction = app::dto::superposition_sdk_config::VaultingAction;
        return should_call_modular && should_perform_sdk_vaulting_flag ? VaultingAction::Tokenize
                                                                       : VaultingAction::Skip;
    }

    // Upstream build_profile_account_config (core:500-523). The local ProfileRecord has no
    // wallet-collection columns, so the four wallet flags are read from the profile-scoped
    // superposition config (unwrap_or(false) in both implementations).
    [[nodiscard]] app::dto::superposition_sdk_config::ProfileAccountConfig
    build_profile_account_config(std::string_view profile_id, bool modular, bool vaulting) const
    {
        using namespace app::dto::superposition_sdk_config;
        const auto profile_flag = [this, profile_id](std::string_view flag)
        {
            const std::string key{std::string{profile_id} + "." + std::string{flag}};
            return feature_flag("sdk_account_config", key, false);
        };
        ProfileAccountConfig profile{};
        profile.set_collect_shipping_details_from_wallet_connector(
            profile_flag("collect_shipping_details_from_wallet_connector")
        );
        profile.set_collect_billing_details_from_wallet_connector(
            profile_flag("collect_billing_details_from_wallet_connector")
        );
        profile.set_always_collect_billing_details_from_wallet_connector(
            profile_flag("always_collect_billing_details_from_wallet_connector")
        );
        profile.set_always_collect_shipping_details_from_wallet_connector(
            profile_flag("always_collect_shipping_details_from_wallet_connector")
        );
        profile.set_vaulting_action(resolve_vaulting_action(modular, vaulting));
        return profile;
    }

    // Upstream build_account_config (core:465-497).
    [[nodiscard]] app::dto::superposition_sdk_config::AccountConfig build_account_config(
        const app::store::profiles::ProfileRecord& /*profile*/,
        std::string_view profile_id,
        std::string_view merchant_id,
        const app::auth::AuthContext& auth_ctx
    ) const
    {
        using namespace app::dto::superposition_sdk_config;
        std::string org_id{};
        if (!auth_ctx.org_id().empty()) {
            org_id = auth_ctx.org_id();
        } else {
            const auto merchant =
                m_context.repository().find_merchant_account_by_string(merchant_id);
            if (merchant) {
                org_id = merchant->get_organization_id();
            }
        }
        const bool modular = is_payment_method_modular_allowed(merchant_id);
        const bool vaulting = should_perform_sdk_vaulting(org_id);
        AccountConfig account_config{};
        account_config.set_profile(build_profile_account_config(profile_id, modular, vaulting));
        return account_config;
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::superposition_sdk_config

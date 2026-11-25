export module app_handlers_routing:runtime_common;

import std;
import core_error;
import core_serde;
import core_cache;
import app_context;
import app_model;
import app_dto;

import :input;
import :result;

export namespace app::handlers::routing {

/**
 * @brief Shared business helpers for the routing sub-runtimes.
 *
 * Everything here works on plain values (ids, profiles, strings, DTOs) and AppContext.
 * No IRequest / IResponse / send callback crosses into the runtime layer.
 */
class RoutingRuntimeSupport
{
public:
    explicit RoutingRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    template<typename T>
    [[nodiscard]] T wrap(std::string value) const
    {
        T result{};
        result.set_value(std::move(value));
        return result;
    }

    [[nodiscard]] std::int64_t now_unix() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    [[nodiscard]] std::string new_routing_id() const
    {
        return m_context.ids().payment_id("routing_");
    }

    [[nodiscard]] std::string
    algorithm_name(app::model::routing::StaticRoutingAlgorithm value) const
    {
        switch (value) {
            case app::model::routing::StaticRoutingAlgorithm::Single:
                return "single";
            case app::model::routing::StaticRoutingAlgorithm::Priority:
                return "priority";
            case app::model::routing::StaticRoutingAlgorithm::VolumeSplit:
                return "volume_split";
            case app::model::routing::StaticRoutingAlgorithm::Advanced:
                return "advanced";
            case app::model::routing::StaticRoutingAlgorithm::ThreeDsDecisionRule:
                return "three_ds_decision_rule";
        }
        return "unknown";
    }

    [[nodiscard]] std::string dashboard_url() const
    {
        return "https://decisionengine.hyperswitch.io";
    }

    [[nodiscard]] std::string dashboard_path(std::string_view target) const
    {
        if (target == "volume") {
            return "volume-based-routing";
        }
        if (target == "rule") {
            return "rule-based-routing";
        }
        if (target == "multi_objective") {
            return "multi-objective-routing";
        }
        if (target == "debit") {
            return "debit-routing";
        }
        return "routing";
    }

    [[nodiscard]] std::string transaction_type_of(std::string_view config_json) const
    {
        return config_json.find("\"algorithm_for\":\"payout\"") != std::string::npos ? "payout"
                                                                                     : "payment";
    }

    /** core/utils.rs:2889-2912 validate_profile_id_from_auth_layer (pure). */
    [[nodiscard]] std::optional<std::string>
    profile_validation_error(std::string_view auth_profile, std::string_view profile) const
    {
        if (!auth_profile.empty() && !profile.empty() && auth_profile != profile) {
            return std::string{
                "Profile id authentication failed. Please use the correct JWT token"
            };
        }
        return std::nullopt;
    }

    [[nodiscard]] RoutingRecord config_record(
        std::string_view merchant_id,
        std::string_view config_id,
        std::string_view name,
        std::string_view description,
        std::string_view algorithm,
        std::string_view profile,
        std::string_view config_json
    ) const
    {
        RoutingRecord record{};
        record.set_config_id(std::string{config_id});
        record.set_merchant_id(std::string{merchant_id});
        record.set_profile_id(std::string{profile});
        record.set_name(std::string{name});
        record.set_description(std::string{description});
        record.set_algorithm(std::string{algorithm});
        record.set_config_json(std::string{config_json});
        record.set_created_at(std::to_string(now_unix()));
        return record;
    }

    [[nodiscard]] std::optional<RoutingRecord> find_config(std::string_view config_id) const
    {
        return m_context.repository().find_routing_config_by_string(config_id);
    }

    [[nodiscard]] std::optional<RoutingRecord>
    find_config_by_algorithm(std::string_view algorithm_id) const
    {
        if (const auto direct = find_config(algorithm_id)) {
            return direct;
        }
        for (auto& record: m_context.repository().list_routing_configs()) {
            if (record.get_algorithm_id() == algorithm_id) {
                return record;
            }
        }
        return std::nullopt;
    }

    void upsert_config(RoutingRecord record) const
    {
        auto repository = m_context.repository();
        if (const auto existing =
                repository.find_routing_config_by_string(record.get_config_id())) {
            record.set_id(existing->get_id());
            repository.update_routing_config(record);
        } else {
            repository.save_routing_config(std::move(record));
        }
    }

    bool remove_config(std::string_view config_id) const
    {
        auto repository = m_context.repository();
        const auto existing = repository.find_routing_config_by_string(config_id);
        return existing && repository.remove_routing_config(existing->get_id());
    }

    [[nodiscard]] std::vector<RoutingRecord> paginate(
        std::vector<RoutingRecord> records,
        const app::model::routing::RoutingRetrieveQuery& filter
    ) const
    {
        const std::size_t offset = filter.get_offset() ? *filter.get_offset() : 0;
        if (offset >= records.size()) {
            return {};
        }
        auto begin = records.begin() + static_cast<std::ptrdiff_t>(offset);
        if (filter.get_limit()) {
            const std::size_t limit = *filter.get_limit();
            if (limit < static_cast<std::size_t>(records.end() - begin)) {
                return std::vector<RoutingRecord>{
                    begin,
                    begin + static_cast<std::ptrdiff_t>(limit)
                };
            }
        }
        return std::vector<RoutingRecord>{begin, records.end()};
    }

    [[nodiscard]] DictionaryRecord dictionary_record(const RoutingRecord& record) const
    {
        return dictionary_record(
            record.get_config_id(),
            record.get_profile_id(),
            record.get_name(),
            record.get_description(),
            record.get_algorithm(),
            transaction_type_of(record.get_config_json())
        );
    }

    [[nodiscard]] DictionaryRecord dictionary_record(
        std::string_view id,
        std::string_view profile,
        std::string_view name,
        std::string_view description,
        std::string_view algorithm,
        std::string_view txn
    ) const
    {
        DictionaryRecord item{};
        item.set_id(wrap<app::model::routing::RoutingIdValue>(std::string{id}));
        item.set_profile_id(wrap<app::model::routing::ProfileIdValue>(std::string{profile}));
        item.set_name(std::string{name});
        item.set_description(std::string{description});
        item.set_kind(app::model::routing::RoutingAlgorithmKind::Single);
        item.set_created_at(now_unix());
        item.set_modified_at(now_unix());
        item.set_algorithm_for(wrap<app::model::routing::TransactionTypeValue>(std::string{txn}));
        item.set_decision_engine_routing_id(std::nullopt);
        return item;
    }

    [[nodiscard]] std::optional<std::string> active_id_for(
        const std::optional<std::string>& from_query,
        std::string_view profile,
        std::string_view txn
    ) const
    {
        if (from_query) {
            return *from_query;
        }
        const std::string scope = std::string{profile} + "|" + std::string{txn};
        const auto resolved =
            m_context.superposition().resolve("routing", "active_algorithm", scope);
        if (resolved && !resolved->get_value().empty()) {
            return resolved->get_value();
        }
        return std::nullopt;
    }

    void set_active_id(std::string_view profile, std::string_view txn, std::string value) const
    {
        const std::string scope = std::string{profile} + "|" + std::string{txn};
        m_context.superposition()
            .put_config("routing", "active_algorithm", std::move(value), scope);
    }

    [[nodiscard]] bool is_cutover(std::string_view merchant, std::string_view profile) const
    {
        const auto global = m_context.superposition().resolve("routing", "de_cutover", merchant);
        const auto per_profile = m_context.superposition().resolve(
            "routing",
            "de_cutover",
            std::string{merchant} + "|" + std::string{profile}
        );
        return (global && global->get_value() == "true") ||
               (per_profile && per_profile->get_value() == "true");
    }

    [[nodiscard]] std::string decision_key(std::string_view profile, std::string_view kind) const
    {
        return profile.empty() ? std::string{kind} : std::string{profile} + ":" + std::string{kind};
    }

    void set_active_algo_ref(
        std::string_view profile,
        std::string_view field,
        std::string value
    ) const
    {
        m_context.superposition()
            .put_config("routing", std::string{field}, std::move(value), std::string{profile});
    }

    [[nodiscard]] std::string generic_string(
        const serde::Value& value,
        std::string_view key,
        std::string_view fallback_key = {}
    ) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        auto field = object->get(std::string{key});
        if (!field && !fallback_key.empty()) {
            field = object->get(std::string{fallback_key});
        }
        if (!field) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&field->get())) {
            return *text;
        }
        return {};
    }

    [[nodiscard]] bool valid_program(std::string_view program) const
    {
        auto generic = serde::Ser::decode_generic("application/json", program);
        return generic.has_value() && std::holds_alternative<serde::Value::Object>(generic->get());
    }

    [[nodiscard]] std::string default_key(std::string_view profile) const
    {
        return decision_key(profile, "default");
    }

    [[nodiscard]] Connectors default_config(std::string_view profile) const
    {
        const auto stored = find_config(default_key(profile));
        if (!stored) {
            return {};
        }
        auto parsed =
            serde::Ser::deserialize<Connectors>("application/json", stored->get_config_json());
        return parsed ? std::move(*parsed) : Connectors{};
    }

    void store_default_config(
        std::string_view merchant_id,
        std::string_view profile,
        const Connectors& connectors
    ) const
    {
        const auto key = default_key(profile);
        RoutingRecord record{};
        record.set_config_id(key);
        record.set_merchant_id(std::string{merchant_id});
        record.set_profile_id(std::string{profile});
        record.set_name("default");
        record.set_description("fallback routing");
        record.set_algorithm("default");
        record.set_config_json(serde::Ser::encode_json(connectors));
        record.set_created_at(std::to_string(now_unix()));
        upsert_config(std::move(record));
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_default_connectors(std::string_view profile, const Connectors& updated) const
    {
        const auto existing = default_config(profile);
        if (existing.size() != updated.size()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "current config and updated config have different lengths"
            );
        }
        std::vector<std::string> a, b;
        for (const auto& c: existing) {
            a.push_back(connector_key(c));
        }
        for (const auto& c: updated) {
            b.push_back(connector_key(c));
        }
        std::ranges::sort(a);
        std::ranges::sort(b);
        std::vector<std::string> diff;
        std::ranges::set_symmetric_difference(a, b, std::back_inserter(diff));
        if (!diff.empty()) {
            std::string joined;
            for (std::size_t i = 0; i < diff.size(); ++i) {
                if (i) {
                    joined += ", ";
                }
                joined += diff[i];
            }
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format("connector mismatch between old and new configs ({})", joined)
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string
    connector_key(const app::model::routing::RoutableConnectorChoice& c) const
    {
        return std::string{c.get_connector().get_value()};
    }

    // create_specific_dynamic_routing shared by success-based and elimination routes.
    [[nodiscard]] std::expected<serde::Value, core::error::AppError> dynamic_create(
        std::string_view merchant_id,
        std::string_view profile,
        std::string_view auth_profile,
        std::string_view body,
        std::string_view feature,
        std::string_view operation,
        std::string_view name,
        std::string_view algorithm,
        std::string_view description
    ) const
    {
        if (feature == "none") {
            put_dynamic_ref(
                profile,
                R"({"success_based_algorithm":null,"elimination_routing_algorithm":null})"
            );
            track(operation);
            return ResponseDocument::empty(); // StatusOk document
        }
        if (body.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            )};
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            )};
        }
        if (const auto validation_error = profile_validation_error(auth_profile, profile)) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                *validation_error
            )};
        }
        const auto algorithm_id = new_routing_id();
        auto record =
            config_record(merchant_id, algorithm_id, name, description, algorithm, profile, body);
        record.set_algorithm_id(algorithm_id);
        upsert_config(std::move(record));
        const auto key = algorithm == "success_rate_based" ? "success_based_algorithm"
                                                           : "elimination_routing_algorithm";
        put_dynamic_ref(
            profile,
            std::format(
                R"({{"{}":{{"algorithm_id_with_timestamp":{{"algorithm_id":"{}","timestamp":{}}},"enabled_feature":"{}"}}}})",
                key,
                algorithm_id,
                now_unix(),
                feature
            )
        );
        track(operation);
        return ResponseDocument::from(
            dictionary_record(algorithm_id, profile, name, description, algorithm, "payment")
        );
    }

    void put_dynamic_ref(std::string_view profile, std::string value) const
    {
        m_context.superposition()
            .put_config("routing", "dynamic_algorithm_ref", std::move(value), std::string{profile});
    }

    [[nodiscard]] bool dynamic_ref_has_feature(
        std::string_view profile,
        std::string_view key,
        std::string_view feature
    ) const
    {
        const auto ref = m_context.superposition()
                             .resolve("routing", "dynamic_algorithm_ref", std::string{profile});
        if (!ref) {
            return false;
        }
        return ref->get_value().find(std::string{key}) != std::string::npos &&
               ref->get_value().find(std::string{feature}) != std::string::npos;
    }

    void update_dynamic_volume_split(std::string_view profile, std::uint8_t split) const
    {
        const auto value =
            std::format(R"({{"dynamic_routing_volume_split":{}}})", static_cast<unsigned>(split));
        m_context.superposition()
            .put_config("routing", "dynamic_algorithm_ref", value, std::string{profile});
    }

    [[nodiscard]] std::uint8_t dynamic_volume_split(std::string_view profile) const
    {
        const auto ref = m_context.superposition()
                             .resolve("routing", "dynamic_algorithm_ref", std::string{profile});
        if (!ref) {
            return 0;
        }
        const auto value = generic_number(ref->get_value(), "dynamic_routing_volume_split");
        return value ? static_cast<std::uint8_t>(*value) : 0;
    }

    [[nodiscard]] std::optional<std::int64_t>
    generic_number(std::string_view json, std::string_view key) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", json);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&parsed->get());
        if (!object) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return std::nullopt;
        }
        if (const auto* n = std::get_if<std::int64_t>(&field->get())) {
            return *n;
        }
        if (const auto* n = std::get_if<double>(&field->get())) {
            return static_cast<std::int64_t>(*n);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<std::string> de_rule_ids(std::string_view profile) const
    {
        const auto stored =
            m_context.superposition().resolve("routing.de_rules", "ids", std::string{profile});
        if (!stored || stored->get_value().empty()) {
            return {};
        }
        std::vector<std::string> ids;
        for (auto part: std::views::split(stored->get_value(), ',')) {
            std::string id{std::ranges::begin(part), std::ranges::end(part)};
            if (!id.empty()) {
                ids.push_back(std::move(id));
            }
        }
        return ids;
    }

    void add_de_rule_id(std::string_view profile, std::string_view id) const
    {
        auto ids = de_rule_ids(profile);
        if (std::ranges::find(ids, id) != ids.end()) {
            return;
        }
        ids.emplace_back(id);
        std::string joined;
        for (std::size_t i = 0; i < ids.size(); ++i) {
            if (i) {
                joined += ',';
            }
            joined += ids[i];
        }
        m_context.superposition()
            .put_config("routing.de_rules", "ids", std::move(joined), std::string{profile});
    }

    [[nodiscard]] std::vector<DictionaryRecord> list_impl(
        std::string_view merchant,
        std::string_view profile,
        std::string_view txn,
        bool profile_scoped,
        std::string_view flow,
        const app::model::routing::RoutingRetrieveQuery& filter
    ) const
    {
        auto records = m_context.repository().list_routing_configs();
        std::vector<RoutingRecord> selected;
        for (auto& record: records) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (profile_scoped && !profile.empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (!record.get_config_id().ends_with(":default") &&
                transaction_type_of(record.get_config_json()) != txn) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        selected = paginate(std::move(selected), filter);

        // DE list merge degrades to Hyperswitch records locally, exactly like upstream on DE
        // failure.
        std::vector<DictionaryRecord> items;
        items.reserve(selected.size());
        for (const auto& record: selected) {
            items.push_back(dictionary_record(record));
        }
        track(flow);
        return items;
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    void track(std::string_view flow) const
    {
        const auto metric = std::string{"routing."} + std::string{flow};
        m_context.metrics().counter(metric).increment();
        m_context.metrics().record_flow(metric);
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::routing

module;
export module app_connector:selector;

import std;
import app_store;
import app_metrics;
import :request;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/** @brief Ranked connector candidates gathered from AppRepository config and records. */
struct ConnectorCandidates
{
    std::vector<std::string> routing{};
    std::vector<std::string> merchant{};
};

/**
 * @brief Chooses a connector from merchant id, payment method type, currency and the routing
 * config held by AppRepository. Every choice is recorded in app_metrics.
 */
class ConnectorSelector
{
public:
    ConnectorSelector() = default;

    ConnectorSelector(
        app::store::AppRepository repository,
        app::metrics::MetricsRegistry& metrics
    ) :
        m_repository{std::make_shared<app::store::AppRepository>(std::move(repository))},
        m_metrics{&metrics}
    {
    }

    [[nodiscard]] std::optional<std::string> select(
        std::string_view merchant_id,
        std::string_view payment_method_type,
        std::string_view currency
    ) const
    {
        const auto candidates = candidate_connectors(merchant_id, payment_method_type, currency);
        auto chosen = choose(candidates, currency);
        if (chosen.has_value()) {
            record_choice(merchant_id, *chosen);
        }
        return chosen;
    }

    [[nodiscard]] std::optional<std::string> select_for(const ConnectorRequest& request) const
    {
        return select(
            request.get_merchant_id(),
            request.get_payment_method_type(),
            request.get_currency()
        );
    }

    /** @brief Collects routing-configured names first, then enabled merchant connectors. */
    [[nodiscard]] ConnectorCandidates candidate_connectors(
        std::string_view merchant_id,
        std::string_view payment_method_type,
        [[maybe_unused]] std::string_view currency
    ) const
    {
        ConnectorCandidates result{};
        if (!m_repository) {
            return result;
        }

        for (const auto& config: m_repository->list_routing_configs()) {
            if (config.get_merchant_id() != merchant_id) {
                continue;
            }
            const auto name = extract_connector_name(config.get_config_json());
            if (!name.empty()) {
                result.routing.push_back(name);
            }
        }

        for (const auto& connector: m_repository->list_merchant_connectors()) {
            if (connector.get_merchant_id() != merchant_id || connector.get_disabled()) {
                continue;
            }
            if (!payment_method_type.empty() && !connector.get_payment_methods_enabled().empty() &&
                !connector.get_payment_methods_enabled().contains(payment_method_type)) {
                continue;
            }
            result.merchant.push_back(connector.get_connector_name());
        }
        return result;
    }

    /** @brief Routing preference wins; otherwise a deterministic currency bucket rotates. */
    [[nodiscard]] std::optional<std::string>
    choose(const ConnectorCandidates& candidates, std::string_view currency) const
    {
        if (!candidates.routing.empty()) {
            return candidates.routing.front();
        }
        if (candidates.merchant.empty()) {
            return std::nullopt;
        }
        if (currency.empty()) {
            return candidates.merchant.front();
        }
        const std::uint64_t bucket = hash(currency) % candidates.merchant.size();
        return candidates.merchant[static_cast<std::size_t>(bucket)];
    }

private:
    [[nodiscard]] std::string extract_connector_name(std::string_view config_json) const
    {
        const std::array<std::string_view, 2> keys{"\"connector_name\"", "\"connector\""};
        for (std::string_view key: keys) {
            const auto at = config_json.find(key);
            if (at == std::string_view::npos) {
                continue;
            }
            const auto colon = config_json.find(':', at + key.size());
            if (colon == std::string_view::npos) {
                continue;
            }
            const auto quote = config_json.find('"', colon + 1);
            if (quote == std::string_view::npos) {
                continue;
            }
            const auto end = config_json.find('"', quote + 1);
            if (end == std::string_view::npos) {
                continue;
            }
            return std::string{config_json.substr(quote + 1, end - quote - 1)};
        }
        return {};
    }

    [[nodiscard]] std::uint64_t hash(std::string_view value) const
    {
        std::uint64_t result{14'695'979'041'396'522'278ULL};
        for (const char character: value) {
            result ^= static_cast<std::uint64_t>(static_cast<unsigned char>(character));
            result *= 1'099'511'628'121ULL;
        }
        return result;
    }

    void record_choice(std::string_view merchant_id, std::string_view connector) const
    {
        if (m_metrics == nullptr) {
            return;
        }
        m_metrics->record_flow("connector.select");
        m_metrics->counter(std::string{"connector.select."} + std::string{connector}).increment();
        m_metrics->counter(std::string{"connector.select.merchant."} + std::string{merchant_id})
            .increment();
    }

    std::shared_ptr<app::store::AppRepository> m_repository{};
    app::metrics::MetricsRegistry* m_metrics{nullptr};
};

} // namespace app::connector

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

suite<"app_connector_selector"> selector_suite = []
{
    "routing preference wins over merchant connectors"_test = []
    {
        ConnectorCandidates candidates{};
        candidates.routing.push_back("route_connector");
        candidates.merchant.push_back("merchant_connector");
        const auto chosen = ConnectorSelector{}.choose(candidates, "USD");
        expect(chosen.has_value());
        expect(*chosen == "route_connector");
    };

    "empty candidates select nothing"_test = []
    {
        expect(!ConnectorSelector{}.choose(ConnectorCandidates{}, "USD").has_value());
    };

    "currency selects a stable merchant connector bucket"_test = []
    {
        ConnectorCandidates candidates{};
        candidates.merchant.push_back("a");
        candidates.merchant.push_back("b");
        const auto first = ConnectorSelector{}.choose(candidates, "USD");
        const auto second = ConnectorSelector{}.choose(candidates, "USD");
        expect(first.has_value());
        expect(first == second);
    };
};

} // namespace app::connector::tests
#endif

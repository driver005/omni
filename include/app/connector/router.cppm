module;
export module app_connector:router;

import std;
import core_error;
import app_metrics;
import app_services;
import :interface;
import :request;
import :response;
import :selector;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/**
 * @brief Wraps a connector so every response is flagged as shadow-killed. The wrapped
 * connector still executes normally — only the response flag changes.
 */
class ShadowConnector final : public IConnector
{
public:
    explicit ShadowConnector(std::shared_ptr<IConnector> inner) :
        m_inner{std::move(inner)}
    {
    }

    [[nodiscard]] std::string_view name() const noexcept override
    {
        return m_inner->name();
    }

    [[nodiscard]] ConnectorResponse authorize(const ConnectorRequest& request) override
    {
        return shadow(m_inner->authorize(request));
    }

    [[nodiscard]] ConnectorResponse capture(const ConnectorRequest& request) override
    {
        return shadow(m_inner->capture(request));
    }

    [[nodiscard]] ConnectorResponse refund(const ConnectorRequest& request) override
    {
        return shadow(m_inner->refund(request));
    }

    [[nodiscard]] ConnectorResponse cancel(const ConnectorRequest& request) override
    {
        return shadow(m_inner->cancel(request));
    }

    [[nodiscard]] ConnectorResponse verify(const ConnectorRequest& request) override
    {
        return shadow(m_inner->verify(request));
    }

    [[nodiscard]] ConnectorResponse sync(const ConnectorRequest& request) override
    {
        return shadow(m_inner->sync(request));
    }

    [[nodiscard]] ConnectorResponse accept_dispute(const ConnectorRequest& request) override
    {
        return shadow(m_inner->accept_dispute(request));
    }

    [[nodiscard]] ConnectorResponse submit_evidence(const ConnectorRequest& request) override
    {
        return shadow(m_inner->submit_evidence(request));
    }

    [[nodiscard]] ConnectorResponse fetch_disputes(const ConnectorRequest& request) override
    {
        return shadow(m_inner->fetch_disputes(request));
    }

    [[nodiscard]] ConnectorResponse mandate_revoke(const ConnectorRequest& request) override
    {
        return shadow(m_inner->mandate_revoke(request));
    }

    [[nodiscard]] ConnectorResponse tokenize(const ConnectorRequest& request) override
    {
        return shadow(m_inner->tokenize(request));
    }

    [[nodiscard]] ConnectorResponse delete_token(const ConnectorRequest& request) override
    {
        return shadow(m_inner->delete_token(request));
    }

    [[nodiscard]] ConnectorResponse create_link(const ConnectorRequest& request) override
    {
        return shadow(m_inner->create_link(request));
    }

    [[nodiscard]] ConnectorResponse exchange_token(const ConnectorRequest& request) override
    {
        return shadow(m_inner->exchange_token(request));
    }

    [[nodiscard]] ConnectorResponse check_feature_matrix(const ConnectorRequest& request) override
    {
        return shadow(m_inner->check_feature_matrix(request));
    }

    [[nodiscard]] ConnectorResponse webhook_source_verify(const ConnectorRequest& request) override
    {
        return shadow(m_inner->webhook_source_verify(request));
    }

private:
    [[nodiscard]] ConnectorResponse shadow(ConnectorResponse response) const
    {
        response.set_shadow_killed(true);
        return response;
    }

    std::shared_ptr<IConnector> m_inner{};
};

/**
 * @brief Holds connectors in insertion order and selects one for a merchant. Selection
 * consults AppRepository-backed kill-switch and rollout config records before returning.
 */
class ConnectorRouter
{
public:
    ConnectorRouter() = default;

    void add_connector(std::shared_ptr<IConnector> connector)
    {
        if (!connector) {
            return;
        }
        m_by_name[std::string{connector->name()}] = connector;
        m_connectors.push_back(std::move(connector));
    }

    void set_config_store(app::services::ConfigStore store)
    {
        m_config = std::move(store);
    }

    void set_selector(ConnectorSelector selector)
    {
        m_selector = std::move(selector);
    }

    void set_metrics(app::metrics::MetricsRegistry& metrics) noexcept
    {
        m_metrics = &metrics;
    }

    [[nodiscard]] std::shared_ptr<IConnector> select(std::string_view merchant_id) const
    {
        if (m_connectors.empty()) {
            m_last_error = connector_error("no connector registered");
            return nullptr;
        }
        return apply_policy(m_connectors.front(), merchant_id);
    }

    [[nodiscard]] std::shared_ptr<IConnector> select_for(const ConnectorRequest& request) const
    {
        if (m_connectors.empty()) {
            m_last_error = connector_error("no connector registered");
            return nullptr;
        }
        const auto chosen = m_selector.select_for(request);
        std::shared_ptr<IConnector> base = chosen ? find_by_name(*chosen) : nullptr;
        if (!base) {
            base = m_connectors.front();
        }
        return apply_policy(std::move(base), request.get_merchant_id());
    }

    [[nodiscard]] std::expected<std::shared_ptr<IConnector>, core::error::AppError>
    select_checked(std::string_view merchant_id) const
    {
        auto selected = select(merchant_id);
        if (selected) {
            return selected;
        }
        if (m_last_error.has_value()) {
            return std::unexpected{*m_last_error};
        }
        return std::unexpected{connector_error("no connector available")};
    }

    [[nodiscard]] std::optional<core::error::AppError> last_error() const
    {
        return m_last_error;
    }

private:
    [[nodiscard]] core::error::AppError connector_error(std::string message) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::CONNECTOR_ERROR,
            std::move(message)
        );
    }

    [[nodiscard]] std::shared_ptr<IConnector> find_by_name(std::string_view name) const
    {
        const auto it = m_by_name.find(std::string{name});
        if (it == m_by_name.end()) {
            return nullptr;
        }
        return it->second;
    }

    [[nodiscard]] std::shared_ptr<IConnector> apply_policy(std::shared_ptr<IConnector> base, std::string_view merchant_id) const
    {
        if (!base) {
            return nullptr;
        }
        const std::string connector{base->name()};

        if (m_config.is_killed(connector)) {
            m_last_error = connector_error("connector killed: " + connector);
            record_flow("connector.killed");
            return nullptr;
        }

        const int rollout_percentage = m_config.rollout(connector);
        if (rollout_percentage < 100) {
            const int bucket = static_cast<int>(hash(merchant_id) % 100);
            if (bucket >= rollout_percentage) {
                m_last_error = connector_error("connector excluded by rollout: " + connector);
                record_flow("connector.rollout.excluded");
                return nullptr;
            }
        }

        record_flow("connector.selected");
        if (m_config.is_shadow_killed(connector)) {
            return std::make_shared<ShadowConnector>(std::move(base));
        }
        return base;
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

    void record_flow(std::string_view flow) const
    {
        if (m_metrics != nullptr) {
            m_metrics->record_flow(flow);
            m_metrics->counter(flow).increment();
        }
    }

    std::vector<std::shared_ptr<IConnector>> m_connectors{};
    std::map<std::string, std::shared_ptr<IConnector>> m_by_name{};
    app::services::ConfigStore m_config{};
    ConnectorSelector m_selector{};
    app::metrics::MetricsRegistry* m_metrics{nullptr};
    mutable std::optional<core::error::AppError> m_last_error{};
};

} // namespace app::connector

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

class NamedConnector : public IConnector
{
public:
    explicit NamedConnector(std::string value) :
        m_name{std::move(value)}
    {
    }

    [[nodiscard]] std::string_view name() const noexcept override
    {
        return m_name;
    }

    [[nodiscard]] ConnectorResponse authorize(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse capture(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse refund(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse cancel(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse verify(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse sync(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse accept_dispute(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse submit_evidence(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse fetch_disputes(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse mandate_revoke(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse tokenize(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse delete_token(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse create_link(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse exchange_token(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse check_feature_matrix(const ConnectorRequest&) override
    {
        return {};
    }

    [[nodiscard]] ConnectorResponse webhook_source_verify(const ConnectorRequest&) override
    {
        return {};
    }

private:
    std::string m_name{};
};

suite<"app_connector_router"> router_suite = []
{
    "empty router selects nothing"_test = []
    {
        ConnectorRouter router{};
        expect(router.select("merchant_1") == nullptr);
    };

    "select returns the first registered connector for any merchant"_test = []
    {
        ConnectorRouter router{};
        router.add_connector(std::make_shared<NamedConnector>("first"));
        router.add_connector(std::make_shared<NamedConnector>("second"));

        auto selected = router.select("merchant_1");
        expect(selected != nullptr);
        expect(selected->name() == "first");
        expect(router.select("other_merchant")->name() == "first");
    };

    "killed connector is not selected and yields CONNECTOR_ERROR"_test = []
    {
        app::services::ConfigStore config{};
        config.set_kill_switch("first", true);

        ConnectorRouter router{};
        router.set_config_store(config);
        router.add_connector(std::make_shared<NamedConnector>("first"));

        expect(router.select("merchant_1") == nullptr);
        const auto error = router.last_error();
        expect(error.has_value());
        expect(error->get_kind() == core::error::AppErrorKind::CONNECTOR_ERROR);
        expect(!router.select_checked("merchant_1").has_value());
    };

    "shadow-killed connector still executes but flags responses"_test = []
    {
        app::services::ConfigStore config{};
        config.set_shadow_kill("first", true);

        ConnectorRouter router{};
        router.set_config_store(config);
        router.add_connector(std::make_shared<NamedConnector>("first"));

        auto selected = router.select("merchant_1");
        expect(selected != nullptr);
        ConnectorRequest request{};
        expect(selected->authorize(request).get_shadow_killed());
    };
};

} // namespace app::connector::tests
#endif

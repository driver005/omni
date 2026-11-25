module;
export module app_connector:interface;

import std;
import :request;
import :response;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/** @brief Payment connector contract — every operation is an instance virtual. */
class IConnector
{
public:
    IConnector() = default;
    virtual ~IConnector() = default;

    IConnector(const IConnector&) = default;
    IConnector& operator=(const IConnector&) = default;
    IConnector(IConnector&&) = default;
    IConnector& operator=(IConnector&&) = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    // Core payment lifecycle.
    [[nodiscard]] virtual ConnectorResponse authorize(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse capture(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse refund(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse cancel(const ConnectorRequest& request) = 0;

    // Verification / sync.
    [[nodiscard]] virtual ConnectorResponse verify(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse sync(const ConnectorRequest& request) = 0;

    // Disputes.
    [[nodiscard]] virtual ConnectorResponse accept_dispute(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse submit_evidence(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse fetch_disputes(const ConnectorRequest& request) = 0;

    // Mandates.
    [[nodiscard]] virtual ConnectorResponse mandate_revoke(const ConnectorRequest& request) = 0;

    // Tokenization.
    [[nodiscard]] virtual ConnectorResponse tokenize(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse delete_token(const ConnectorRequest& request) = 0;

    // Link tokens and OAuth exchange.
    [[nodiscard]] virtual ConnectorResponse create_link(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse exchange_token(const ConnectorRequest& request) = 0;

    // Capability discovery and inbound webhook verification.
    [[nodiscard]] virtual ConnectorResponse
    check_feature_matrix(const ConnectorRequest& request) = 0;
    [[nodiscard]] virtual ConnectorResponse
    webhook_source_verify(const ConnectorRequest& request) = 0;
};

} // namespace app::connector

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

class StubConnector : public IConnector
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "stub";
    }

    [[nodiscard]] ConnectorResponse authorize(const ConnectorRequest&) override
    {
        return with_status("authorized");
    }

    [[nodiscard]] ConnectorResponse capture(const ConnectorRequest&) override
    {
        return with_status("captured");
    }

    [[nodiscard]] ConnectorResponse refund(const ConnectorRequest&) override
    {
        return with_status("refunded");
    }

    [[nodiscard]] ConnectorResponse cancel(const ConnectorRequest&) override
    {
        return with_status("cancelled");
    }

    [[nodiscard]] ConnectorResponse verify(const ConnectorRequest&) override
    {
        return with_status("verified");
    }

    [[nodiscard]] ConnectorResponse sync(const ConnectorRequest&) override
    {
        return with_status("synced");
    }

    [[nodiscard]] ConnectorResponse accept_dispute(const ConnectorRequest&) override
    {
        return with_status("dispute_accepted");
    }

    [[nodiscard]] ConnectorResponse submit_evidence(const ConnectorRequest&) override
    {
        return with_status("evidence_submitted");
    }

    [[nodiscard]] ConnectorResponse fetch_disputes(const ConnectorRequest&) override
    {
        return with_status("disputes_fetched");
    }

    [[nodiscard]] ConnectorResponse mandate_revoke(const ConnectorRequest&) override
    {
        return with_status("mandate_revoked");
    }

    [[nodiscard]] ConnectorResponse tokenize(const ConnectorRequest&) override
    {
        return with_status("tokenized");
    }

    [[nodiscard]] ConnectorResponse delete_token(const ConnectorRequest&) override
    {
        return with_status("token_deleted");
    }

    [[nodiscard]] ConnectorResponse create_link(const ConnectorRequest&) override
    {
        return with_status("link_created");
    }

    [[nodiscard]] ConnectorResponse exchange_token(const ConnectorRequest&) override
    {
        return with_status("token_exchanged");
    }

    [[nodiscard]] ConnectorResponse check_feature_matrix(const ConnectorRequest&) override
    {
        return with_status("feature_matrix_checked");
    }

    [[nodiscard]] ConnectorResponse webhook_source_verify(const ConnectorRequest&) override
    {
        return with_status("webhook_verified");
    }

private:
    [[nodiscard]] ConnectorResponse with_status(std::string status) const
    {
        ConnectorResponse response{};
        response.set_status(std::move(status));
        return response;
    }
};

suite<"app_connector_interface"> interface_suite = []
{
    "all four core operations dispatch through the base reference"_test = []
    {
        auto owned = std::make_shared<StubConnector>();
        IConnector& connector = *owned;
        ConnectorRequest request{};

        expect(connector.name() == "stub");
        expect(connector.authorize(request).get_status() == "authorized");
        expect(connector.capture(request).get_status() == "captured");
        expect(connector.refund(request).get_status() == "refunded");
        expect(connector.cancel(request).get_status() == "cancelled");
    };

    "extended capabilities dispatch through the base reference"_test = []
    {
        auto owned = std::make_shared<StubConnector>();
        IConnector& connector = *owned;
        ConnectorRequest request{};

        expect(connector.verify(request).get_status() == "verified");
        expect(connector.sync(request).get_status() == "synced");
        expect(connector.accept_dispute(request).get_status() == "dispute_accepted");
        expect(connector.submit_evidence(request).get_status() == "evidence_submitted");
        expect(connector.fetch_disputes(request).get_status() == "disputes_fetched");
        expect(connector.mandate_revoke(request).get_status() == "mandate_revoked");
        expect(connector.tokenize(request).get_status() == "tokenized");
        expect(connector.delete_token(request).get_status() == "token_deleted");
        expect(connector.create_link(request).get_status() == "link_created");
        expect(connector.exchange_token(request).get_status() == "token_exchanged");
        expect(connector.check_feature_matrix(request).get_status() == "feature_matrix_checked");
        expect(connector.webhook_source_verify(request).get_status() == "webhook_verified");
    };
};

} // namespace app::connector::tests
#endif

module;
export module app_connector:local;

import std;
import :interface;
import :request;
import :response;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/**
 * @brief In-process connector: every operation succeeds with a fixed status and a sequential
 * reference. No network, no crypto, no randomness — same call order yields same references.
 */
class LocalConnector final : public IConnector
{
public:
    LocalConnector() = default;

    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "local";
    }

    [[nodiscard]] ConnectorResponse authorize(const ConnectorRequest& request) override
    {
        return next("authorize", "authorized", m_auth_count, request);
    }

    [[nodiscard]] ConnectorResponse capture(const ConnectorRequest& request) override
    {
        return next("capture", "captured", m_capture_count, request);
    }

    [[nodiscard]] ConnectorResponse refund(const ConnectorRequest& request) override
    {
        return next("refund", "refunded", m_refund_count, request);
    }

    [[nodiscard]] ConnectorResponse cancel(const ConnectorRequest& request) override
    {
        return next("cancel", "cancelled", m_cancel_count, request);
    }

    [[nodiscard]] ConnectorResponse verify(const ConnectorRequest& request) override
    {
        return next("verify", "verified", m_verify_count, request);
    }

    [[nodiscard]] ConnectorResponse sync(const ConnectorRequest& request) override
    {
        return next("sync", "synced", m_sync_count, request);
    }

    [[nodiscard]] ConnectorResponse accept_dispute(const ConnectorRequest& request) override
    {
        return next("accept_dispute", "dispute_accepted", m_accept_dispute_count, request);
    }

    [[nodiscard]] ConnectorResponse submit_evidence(const ConnectorRequest& request) override
    {
        return next("submit_evidence", "evidence_submitted", m_submit_evidence_count, request);
    }

    [[nodiscard]] ConnectorResponse fetch_disputes(const ConnectorRequest& request) override
    {
        return next("fetch_disputes", "disputes_fetched", m_fetch_disputes_count, request);
    }

    [[nodiscard]] ConnectorResponse mandate_revoke(const ConnectorRequest& request) override
    {
        return next("mandate_revoke", "mandate_revoked", m_mandate_revoke_count, request);
    }

    [[nodiscard]] ConnectorResponse tokenize(const ConnectorRequest& request) override
    {
        return next("tokenize", "tokenized", m_tokenize_count, request);
    }

    [[nodiscard]] ConnectorResponse delete_token(const ConnectorRequest& request) override
    {
        return next("delete_token", "token_deleted", m_delete_token_count, request);
    }

    [[nodiscard]] ConnectorResponse create_link(const ConnectorRequest& request) override
    {
        return next("create_link", "link_created", m_create_link_count, request);
    }

    [[nodiscard]] ConnectorResponse exchange_token(const ConnectorRequest& request) override
    {
        return next("exchange_token", "token_exchanged", m_exchange_token_count, request);
    }

    [[nodiscard]] ConnectorResponse check_feature_matrix(const ConnectorRequest& request) override
    {
        return next("check_feature_matrix", "feature_matrix_checked", m_feature_matrix_count, request);
    }

    [[nodiscard]] ConnectorResponse webhook_source_verify(const ConnectorRequest& request) override
    {
        return next("webhook_source_verify", "webhook_verified", m_webhook_verify_count, request);
    }

private:
    [[nodiscard]] ConnectorResponse next(
        std::string_view operation,
        std::string_view status,
        std::uint64_t& counter,
        const ConnectorRequest& request
    )
    {
        ++counter;
        return make_response(
            operation,
            status,
            std::format("local_{}_{}", operation, counter),
            request
        );
    }

    [[nodiscard]] ConnectorResponse make_response(
        std::string_view operation,
        std::string_view status,
        std::string reference_id,
        const ConnectorRequest& request
    )
    {
        ConnectorResponse response{};
        response.set_status(std::string{status});
        response.set_reference_id(std::move(reference_id));
        response.set_message(std::format("local {} accepted", operation));
        response.set_raw_json(std::format(
            "{{\"operation\":\"{}\",\"reference_id\":\"{}\",\"status\":\"{}\",\"resource_id\":\"{}\"}}",
            operation,
            response.get_reference_id(),
            status,
            request.get_resource_id()
        ));
        response.set_retryable(false);
        response.set_shadow_killed(false);
        response.set_attempt(1);
        return response;
    }

    std::uint64_t m_auth_count{0};
    std::uint64_t m_capture_count{0};
    std::uint64_t m_refund_count{0};
    std::uint64_t m_cancel_count{0};
    std::uint64_t m_verify_count{0};
    std::uint64_t m_sync_count{0};
    std::uint64_t m_accept_dispute_count{0};
    std::uint64_t m_submit_evidence_count{0};
    std::uint64_t m_fetch_disputes_count{0};
    std::uint64_t m_mandate_revoke_count{0};
    std::uint64_t m_tokenize_count{0};
    std::uint64_t m_delete_token_count{0};
    std::uint64_t m_create_link_count{0};
    std::uint64_t m_exchange_token_count{0};
    std::uint64_t m_feature_matrix_count{0};
    std::uint64_t m_webhook_verify_count{0};
};

} // namespace app::connector

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

suite<"app_connector_local"> local_suite = []
{
    "core operations produce the documented deterministic references"_test = []
    {
        LocalConnector connector{};
        ConnectorRequest request{};
        request.set_resource_id{"payment_1"};

        expect(connector.name() == "local");
        expect(connector.authorize(request).get_reference_id() == "local_authorize_1");
        expect(connector.capture(request).get_reference_id() == "local_capture_1");
        expect(connector.refund(request).get_reference_id() == "local_refund_1");
        expect(connector.cancel(request).get_reference_id() == "local_cancel_1");
    };

    "extended operations produce operation-shaped references"_test = []
    {
        LocalConnector connector{};
        ConnectorRequest request{};
        request.set_resource_id("payment_1");

        expect(connector.verify(request).get_reference_id() == "local_verify_1");
        expect(connector.sync(request).get_reference_id() == "local_sync_1");
        expect(connector.accept_dispute(request).get_reference_id() == "local_accept_dispute_1");
        expect(connector.submit_evidence(request).get_reference_id() == "local_submit_evidence_1");
        expect(connector.fetch_disputes(request).get_reference_id() == "local_fetch_disputes_1");
        expect(connector.mandate_revoke(request).get_reference_id() == "local_mandate_revoke_1");
        expect(connector.tokenize(request).get_reference_id() == "local_tokenize_1");
        expect(connector.delete_token(request).get_reference_id() == "local_delete_token_1");
        expect(connector.create_link(request).get_reference_id() == "local_create_link_1");
        expect(connector.exchange_token(request).get_reference_id() == "local_exchange_token_1");
        expect(connector.check_feature_matrix(request).get_reference_id() == "local_check_feature_matrix_1");
        expect(connector.webhook_source_verify(request).get_reference_id() == "local_webhook_source_verify_1");
    };

    "statuses form a fixed operation-to-status mapping"_test = []
    {
        LocalConnector connector{};
        ConnectorRequest request{};

        expect(connector.authorize(request).get_status() == "authorized");
        expect(connector.capture(request).get_status() == "captured");
        expect(connector.refund(request).get_status() == "refunded");
        expect(connector.cancel(request).get_status() == "cancelled");
        expect(connector.verify(request).get_status() == "verified");
        expect(connector.sync(request).get_status() == "synced");
        expect(connector.tokenize(request).get_status() == "tokenized");
    };

    "references advance per operation, independently of each other"_test = []
    {
        LocalConnector connector{};
        ConnectorRequest request{};

        expect(connector.authorize(request).get_reference_id() == "local_authorize_1");
        expect(connector.authorize(request).get_reference_id() == "local_authorize_2");
        expect(connector.capture(request).get_reference_id() == "local_capture_1");
    };

    "raw JSON carries operation, reference, status and resource id"_test = []
    {
        LocalConnector connector{};
        ConnectorRequest request{};
        request.set_resource_id("payment_7");

        const auto response = connector.authorize(request);
        expect(response.get_raw_json().contains("\"operation\":\"authorize\""));
        expect(response.get_raw_json().contains("\"reference_id\":\"local_authorize_1\""));
        expect(response.get_raw_json().contains("\"status\":\"authorized\""));
        expect(response.get_raw_json().contains("\"resource_id\":\"payment_7\""));
        expect(!response.get_retryable());
        expect(!response.get_shadow_killed());
        expect(response.get_attempt() == 1);
    };
};

} // namespace app::connector::tests
#endif

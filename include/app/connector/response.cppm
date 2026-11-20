module;
export module app_connector:response;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/** @brief Plain result of one connector operation, with a raw JSON payload kept verbatim. */
class ConnectorResponse
{
public:
    ConnectorResponse() = default;

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_reference_id(std::string value)
    {
        m_reference_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_reference_id() const noexcept
    {
        return m_reference_id;
    }

    void set_message(std::string value)
    {
        m_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return m_message;
    }

    void set_raw_json(std::string value)
    {
        m_raw_json = std::move(value);
    }

    [[nodiscard]] const std::string& get_raw_json() const noexcept
    {
        return m_raw_json;
    }

    /** @brief Whether the caller should retry this operation (data, not a sleep). */
    void set_retryable(bool value) noexcept
    {
        m_retryable = value;
    }

    [[nodiscard]] const bool& get_retryable() const noexcept
    {
        return m_retryable;
    }

    /** @brief Set when a connector is shadow-killed: call succeeded but is flagged. */
    void set_shadow_killed(bool value) noexcept
    {
        m_shadow_killed = value;
    }

    [[nodiscard]] const bool& get_shadow_killed() const noexcept
    {
        return m_shadow_killed;
    }

    /** @brief One-based attempt number that produced this response. */
    void set_attempt(std::int64_t value) noexcept
    {
        m_attempt = value;
    }

    [[nodiscard]] const std::int64_t& get_attempt() const noexcept
    {
        return m_attempt;
    }

private:
    std::string m_status{};
    std::string m_reference_id{};
    std::string m_message{};
    std::string m_raw_json{};
    bool m_retryable{};
    bool m_shadow_killed{};
    std::int64_t m_attempt{};
};

} // namespace app::connector

template<>
struct serde::Serializable<app::connector::ConnectorResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "status",
                &app::connector::ConnectorResponse::get_status,
                &app::connector::ConnectorResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference_id",
                &app::connector::ConnectorResponse::get_reference_id,
                &app::connector::ConnectorResponse::set_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::connector::ConnectorResponse::get_message,
                &app::connector::ConnectorResponse::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "raw_json",
                &app::connector::ConnectorResponse::get_raw_json,
                &app::connector::ConnectorResponse::set_raw_json,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "retryable",
                &app::connector::ConnectorResponse::get_retryable,
                &app::connector::ConnectorResponse::set_retryable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shadow_killed",
                &app::connector::ConnectorResponse::get_shadow_killed,
                &app::connector::ConnectorResponse::set_shadow_killed,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt",
                &app::connector::ConnectorResponse::get_attempt,
                &app::connector::ConnectorResponse::set_attempt,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

suite<"app_connector_response"> response_suite = []
{
    "response defaults to empty fields"_test = []
    {
        ConnectorResponse response{};
        expect(response.get_status().empty());
        expect(response.get_reference_id().empty());
        expect(response.get_message().empty());
        expect(response.get_raw_json().empty());
        expect(!response.get_retryable());
        expect(!response.get_shadow_killed());
        expect(response.get_attempt() == 0);
    };

    "response reflects every setter"_test = []
    {
        ConnectorResponse response{};
        response.set_status("authorized");
        response.set_reference_id("local_auth_1");
        response.set_message("local authorize accepted");
        response.set_raw_json("{\"status\":\"authorized\"}");
        response.set_retryable(true);
        response.set_shadow_killed(true);
        response.set_attempt(3);

        expect(response.get_status() == "authorized");
        expect(response.get_reference_id() == "local_auth_1");
        expect(response.get_message() == "local authorize accepted");
        expect(response.get_raw_json() == "{\"status\":\"authorized\"}");
        expect(response.get_retryable());
        expect(response.get_shadow_killed());
        expect(response.get_attempt() == 3);
    };

    "response is serde serializable with seven reflected fields"_test = []
    {
        expect(std::tuple_size_v<
                   decltype(serde::Serializable<ConnectorResponse>::fields())> == 7);
        expect(serde::Serializable<ConnectorResponse>::table_name() == "connector_response");
    };
};

} // namespace app::connector::tests
#endif

export module app_payment:poll_runtime_common;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;

export namespace app::handlers::poll {

/**
 * @brief Shared support for the poll sub-runtime.
 *
 * Carries the AppContext reference, the two typed AppError instances and the
 * core/poll.rs helpers. Upstream codes HE_02 / HE_00 live in the AppError
 * kind_name so the serialized error field preserves them, with the real
 * human-readable text in the message field.
 */
class PollRuntimeSupport
{
public:
    explicit PollRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** PollNotFound -> 404, kind_name "HE_02", message "Poll does not exist in our records". */
    [[nodiscard]] core::error::AppError poll_not_found_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            "Poll does not exist in our records"
        );
        error.set_kind_name("HE_02");
        return error;
    }

    /** InternalServerError -> 500, kind_name "HE_00", message "Something went wrong". */
    [[nodiscard]] core::error::AppError internal_server_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL,
            "Something went wrong"
        );
        error.set_kind_name("HE_00");
        return error;
    }

    // super::utils::get_poll_id (core/poll.rs:24-25, utils.rs:2452-2457):
    // prefix "poll_{processor_merchant_id}_" to the free-form request poll id.
    [[nodiscard]] std::string
    get_poll_id(std::string_view processor_merchant_id, std::string_view unique_id) const
    {
        return std::string{"poll_"} + std::string{processor_merchant_id} + "_" +
               std::string{unique_id};
    }

    // StringExt::parse_enum("PollStatus") (core/poll.rs:39-42). snake_case is
    // the upstream wire form; PascalCase is accepted only to tolerate local
    // cache writers. Unknown values never default silently.
    [[nodiscard]] std::optional<::app::dto::poll::PollStatus>
    parse_poll_status(std::string_view raw) const noexcept
    {
        if (raw == "pending" || raw == "Pending") {
            return ::app::dto::poll::PollStatus::Pending;
        }
        if (raw == "completed" || raw == "Completed") {
            return ::app::dto::poll::PollStatus::Completed;
        }
        if (raw == "not_found" || raw == "NotFound" || raw == "NOT_FOUND") {
            return ::app::dto::poll::PollStatus::NotFound;
        }
        return std::nullopt;
    }

    // api_models/poll.rs:5-28 wire shape with snake_case status.
    [[nodiscard]] ::app::dto::poll::PollResponseWire
    encode_poll_response(std::string_view poll_id, ::app::dto::poll::PollStatus status) const
    {
        ::app::dto::poll::PollResponseWire wire;
        wire.set_poll_id(std::string{poll_id});
        wire.set_status(std::string{::app::dto::poll::PollStatusWireName{}(status)});
        return wire;
    }

    void record_retrieve_metrics(std::string_view request_poll_id) const
    {
        m_context.metrics().counter("poll_retrieve_total").increment();
        m_context.metrics().counter(std::format("poll_response.{}", request_poll_id)).increment();
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::poll

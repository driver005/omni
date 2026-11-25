export module app_handlers_poll:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::poll {

/**
 * @brief api_models/poll.rs:5-28 PollResponse wire shape.
 *
 * Inherits the exact fields from PollResponseWire, so the {"poll_id":...,
 * "status":"<snake_case>"} JSON stays byte-identical to the old hand-built text.
 */
class RetrievePollStatusResult : public app::dto::poll::PollResponseWire
{
public:
    RetrievePollStatusResult() = default;

    explicit RetrievePollStatusResult(app::dto::poll::PollResponseWire value) noexcept :
        app::dto::poll::PollResponseWire{std::move(value)}
    {
    }
};

} // namespace app::handlers::poll

template<>
struct serde::Serializable<app::handlers::poll::RetrievePollStatusResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::poll::PollResponseWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "retrieve_poll_status_result";
    }
};

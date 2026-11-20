module;
export module app_dto:poll;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::poll {

enum class PollStatus
{
    Pending,
    Completed,
    NotFound,
};

class PollResponse
{
public:
    void set_poll_id(std::string value) { poll_id = std::move(value); }
    [[nodiscard]] const std::string& get_poll_id() const noexcept { return poll_id; }
    void set_status(PollStatus value) { status = std::move(value); }
    [[nodiscard]] const PollStatus& get_status() const noexcept { return status; }
    std::string poll_id{};
    PollStatus status{};
};
} // namespace app::dto::poll

template<>
struct serde::Serializable<app::dto::poll::PollResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"poll_id", &app::dto::poll::PollResponse::get_poll_id, &app::dto::poll::PollResponse::set_poll_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::poll::PollResponse::get_status, &app::dto::poll::PollResponse::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "poll_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::poll::tests {
using namespace boost::ut;
suite<"app_dto_poll"> app_dto_poll_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::poll::PollResponse>::fields())> > 0);
    };
};
}
#endif

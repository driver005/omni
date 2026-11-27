module;
export module app_dto:poll;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
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

/**
 * @brief Input DTO mirroring upstream types/api/poll.rs:1-4.
 * Upstream `web::Path<String>` (routes/poll.rs:31,34-36) is wrapped into this
 * single-field struct before it is handed to the core function.
 */
class PollId : public core::behavior::Serializer<PollId>
{
public:
    void set_poll_id(std::string value)
    {
        poll_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_poll_id() const noexcept
    {
        return poll_id;
    }

    std::string poll_id{};
};

class PollResponse : public core::behavior::Serializer<PollResponse>
{
public:
    void set_poll_id(std::string value)
    {
        poll_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_poll_id() const noexcept
    {
        return poll_id;
    }

    void set_status(PollStatus value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const PollStatus& get_status() const noexcept
    {
        return status;
    }

    std::string poll_id{};
    PollStatus status{};
};

/**
 * @brief Wire response carrying the exact upstream snake_case status spelling.
 *
 * Upstream `api_models/poll.rs:13-20` uses strum/serde
 * `serialize_all = "snake_case"` / `rename_all = "snake_case"`, producing
 * "pending"/"completed"/"not_found". rfl reflects the C++ enum by enumerator
 * name ("Pending"/...), so `PollResponse` alone cannot emit the upstream wire
 * value. This typed shape keeps the field names/types from
 * `api_models/poll.rs:5-11` and fixes the status spelling without depending on
 * an external serializer feature.
 */
class PollResponseWire : public core::behavior::Serializer<PollResponseWire>
{
public:
    void set_poll_id(std::string value)
    {
        poll_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_poll_id() const noexcept
    {
        return poll_id;
    }

    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    std::string poll_id{};
    std::string status{};
};

/** @brief snake_case status names from api_models/poll.rs:13-20 (strum/serde). */
class PollStatusWireName : public core::behavior::Serializer<PollStatusWireName>
{
public:
    [[nodiscard]] const char* operator()(PollStatus status) const noexcept
    {
        switch (status) {
            case PollStatus::Pending:
                return "pending";
            case PollStatus::Completed:
                return "completed";
            case PollStatus::NotFound:
                return "not_found";
        }
        return "pending";
    }
};

} // namespace app::dto::poll

template<>
struct serde::Serializable<app::dto::poll::PollId>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "poll_id",
            &app::dto::poll::PollId::get_poll_id,
            &app::dto::poll::PollId::set_poll_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "poll_id";
    }
};

template<>
struct serde::Serializable<app::dto::poll::PollResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "poll_id",
                &app::dto::poll::PollResponse::get_poll_id,
                &app::dto::poll::PollResponse::set_poll_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::dto::poll::PollResponse::get_status,
                &app::dto::poll::PollResponse::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "poll_response";
    }
};

template<>
struct serde::Serializable<app::dto::poll::PollResponseWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "poll_id",
                &app::dto::poll::PollResponseWire::get_poll_id,
                &app::dto::poll::PollResponseWire::set_poll_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::dto::poll::PollResponseWire::get_status,
                &app::dto::poll::PollResponseWire::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "poll_response_wire";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::poll::tests {
using namespace boost::ut;
suite<"app_dto_poll"> app_dto_poll_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::poll::PollResponse>::fields())> > 0
        );
        expect(
            std::tuple_size_v<decltype(serde::Serializable<app::dto::poll::PollId>::fields())> > 0
        );
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::poll::PollResponseWire>::fields())> > 0
        );
    };
    "snake_case status names match upstream"_test = []
    {
        const PollStatusWireName name;
        expect(std::string_view{name(PollStatus::Pending)} == "pending");
        expect(std::string_view{name(PollStatus::Completed)} == "completed");
        expect(std::string_view{name(PollStatus::NotFound)} == "not_found");
    };
};
} // namespace app::dto::poll::tests
#endif

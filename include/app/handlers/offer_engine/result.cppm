export module app_handlers_offer_engine:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::offer_engine {

/** @brief Port of upstream `OfferEngineConnectivityResponse` (connectivity.rs:13-24). */
class ConnectivityCheckResult
{
public:
    void set_enabled(bool value) noexcept
    {
        enabled = value;
    }

    [[nodiscard]] const bool& get_enabled() const noexcept
    {
        return enabled;
    }

    void set_reachable(std::optional<bool> value)
    {
        reachable = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_reachable() const noexcept
    {
        return reachable;
    }

    void set_status_code(std::optional<std::int64_t> value)
    {
        status_code = value;
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_status_code() const noexcept
    {
        return status_code;
    }

    void set_detail(std::string value)
    {
        detail = std::move(value);
    }

    [[nodiscard]] const std::string& get_detail() const noexcept
    {
        return detail;
    }

    bool enabled{};
    std::optional<bool> reachable{};
    std::optional<std::int64_t> status_code{};
    std::string detail{};
};

/**
 * @brief Result of browse_offers. Success carries the typed BrowseOffersResponse DTO.
 * The two upstream API error shapes (IR_22 AccessForbidden / HE_00 InternalServerError)
 * are returned through the runtime's `std::unexpected<core::error::AppError>` channel:
 * real message in `message`, upstream code in `kind_name`, status from `get_status()`.
 */
class BrowseOffersResult
{
public:
    BrowseOffersResult() = default;

    explicit BrowseOffersResult(app::dto::offer_engine::BrowseOffersResponse value) noexcept :
        m_response{std::move(value)}
    {
    }

    [[nodiscard]] const app::dto::offer_engine::BrowseOffersResponse& get_response() const noexcept
    {
        return m_response;
    }

private:
    app::dto::offer_engine::BrowseOffersResponse m_response{};
};

} // namespace app::handlers::offer_engine

template<>
struct serde::Serializable<app::handlers::offer_engine::ConnectivityCheckResult>
{
    static constexpr auto fields()
    {
        using T = app::handlers::offer_engine::ConnectivityCheckResult;
        return std::tuple{
            serde::FieldDesc<"enabled", &T::get_enabled, &T::set_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reachable",
                &T::get_reachable,
                &T::set_reachable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status_code",
                &T::get_status_code,
                &T::set_status_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"detail", &T::get_detail, &T::set_detail, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "offer_engine_connectivity_response";
    }
};

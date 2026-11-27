export module app_routing:offer_engine_result;

import std;
import core_serde;

export namespace app::handlers::offer_engine {

/**
 * @brief Port of upstream `OfferEngineConnectivityResponse` (connectivity.rs:13-24).
 * Wire field list: enabled, reachable, status_code, detail — unchanged from the
 * removed ConnectivityCheckResult wrapper.
 */
class ConnectivityCheckResponse : public serde::Serializer<ConnectivityCheckResponse>
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

// The removed BrowseOffersResult wrapped the typed BrowseOffersResponse DTO;
// runtimes now return that DTO directly. Both wrappers' field lists are captured
// here so handlers keep the original wire keys and declaration order.
template<class T>
[[nodiscard]] consteval auto wire_fields()
{
    using fields_type = decltype(serde::Serializable<T>::fields());
    return []<std::size_t... I>(std::index_sequence<I...>)
    {
        return std::array<std::string_view, sizeof...(I)>{
            std::tuple_element_t<I, fields_type>::name.string_view()...
        };
    }(std::make_index_sequence<std::tuple_size_v<fields_type>>{});
}

} // namespace app::handlers::offer_engine

template<>
struct serde::Serializable<::app::handlers::offer_engine::ConnectivityCheckResponse>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::ConnectivityCheckResponse;
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

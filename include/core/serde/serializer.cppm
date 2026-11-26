export module core_serde:serializer;

import std;
import interfaces;
import :core;
import :json_codec;
import :error;

namespace serializer_detail {

template<class V>
struct field_value
{
    using type = V;
};

template<class V>
    requires requires { typename V::value_type; }
struct field_value<V>
{
    using type = typename field_value<typename V::value_type>::type;
};

template<class V>
    requires requires { typename V::element_type; }
struct field_value<V>
{
    using type = typename field_value<typename V::element_type>::type;
};

template<class Field, class Other, bool WantMany>
[[nodiscard]] consteval bool field_matches()
{
    using getter_type = std::remove_cvref_t<decltype(Field::getter)>;
    using value_type = typename serde::MFPTraits<getter_type>::value_t;
    using inner_type = typename field_value<value_type>::type;
    constexpr bool is_other = std::same_as<std::remove_cvref_t<inner_type>, Other>;
    constexpr bool is_many = std::ranges::range<value_type> &&
                             !std::same_as<std::remove_cvref_t<value_type>, std::string>;
    return is_other && (is_many == WantMany);
}

template<class Side, class Other, bool WantMany>
[[nodiscard]] consteval std::size_t count_fields()
{
    using fields_type = decltype(serde::Serializable<Side>::fields());
    return []<std::size_t... I>(std::index_sequence<I...>)
    {
        return (
            std::size_t{0} + ... +
            (field_matches<std::tuple_element_t<I, fields_type>, Other, WantMany>() ? 1u : 0u)
        );
    }(std::make_index_sequence<std::tuple_size_v<fields_type>>{});
}

template<class Fields, std::size_t... I>
auto field_types_of(std::index_sequence<I...>) -> std::tuple<typename serde::MFPTraits<
    std::remove_cvref_t<decltype(std::tuple_element_t<I, Fields>::getter)>>::value_t...>;

} // namespace serializer_detail

export namespace serde {

enum class Relation
{
    none,
    one_to_one,
    one_to_many,
    many_to_one,
    many_to_many,
};

template<class T>
class Serializer
{
public:
    static constexpr std::string_view content_type = json_content_type;

    struct metadata
    {
        using fields_type = decltype(serde::Serializable<T>::fields());

        static constexpr std::size_t field_count()
        {
            return std::tuple_size_v<fields_type>;
        }

        static constexpr auto field_names()
        {
            return []<std::size_t... I>(std::index_sequence<I...>)
            {
                return std::array<std::string_view, sizeof...(I)>{
                    std::tuple_element_t<I, fields_type>::name...
                };
            }(std::make_index_sequence<std::tuple_size_v<fields_type>>{});
        }

        using field_types = decltype(serializer_detail::field_types_of<fields_type>(
            std::make_index_sequence<std::tuple_size_v<fields_type>>{}
        ));
    };

    template<class Other>
    [[nodiscard]] static consteval Relation relationship()
    {
        constexpr auto left_single = serializer_detail::count_fields<T, Other, false>();
        constexpr auto left_many = serializer_detail::count_fields<T, Other, true>();
        constexpr auto right_single = serializer_detail::count_fields<Other, T, false>();
        constexpr auto right_many = serializer_detail::count_fields<Other, T, true>();

        if constexpr (left_many > 0 && right_many > 0) {
            return Relation::many_to_many;
        } else if constexpr (left_many > 0) {
            return Relation::one_to_many;
        } else if constexpr (right_many > 0) {
            return Relation::many_to_one;
        } else if constexpr (left_single > 0 && right_single > 0) {
            return Relation::one_to_one;
        } else {
            return Relation::none;
        }
    }

    [[nodiscard]] static std::expected<T, core::error::AppError>
    to_model(std::string_view type, std::string_view data)
    {
        if (data.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            )};
        }
        auto parsed = Ser::deserialize<T>(type.empty() ? content_type : type, data);
        if (!parsed) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                parsed.error()
            )};
        }
        return std::move(*parsed);
    }

    [[nodiscard]] std::expected<std::string, core::error::AppError>
    from_model(std::string_view type) const
    {
        const auto bytes =
            Ser::serialize(type.empty() ? content_type : type, static_cast<const T&>(*this));
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }

    [[nodiscard]] static std::expected<T, core::error::AppError>
    from_request(interfaces::io::IRequest& req)
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            )};
        }
        const auto ct = req.get_content_type();
        return to_model(ct, body);
    }

    void to_response(interfaces::io::IResponse& res, std::function<void()> send) const
    {
        auto text = from_model(content_type);
        if (!text) {
            text.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(Ser::serialize(content_type, *text));
        send();
    }
};

} // namespace serde

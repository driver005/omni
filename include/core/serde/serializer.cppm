module;
#include <rfl.hpp>
#include <rfl/to_generic.hpp>

export module core_serde:serializer;

import std;
import interfaces;
import :core;
import :converter;
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

/**
 * @brief Detects a model-owned `validate()` returning something bool-testable.
 * @note `Serializer<T>::validate()` itself returns `std::expected<void, AppError>`, whose bool
 * conversion is explicit — so an inherited copy does not satisfy `convertible_to<bool>` and
 * cannot recurse into this concept. Only a genuinely model-declared predicate matches.
 */
template<class T>
concept IHasValidate = requires(const T& value) {
    { value.validate() } -> std::convertible_to<bool>;
};

/** @brief `required` rule: optionals must be engaged, pointers non-null, others are present. */
template<class V>
[[nodiscard]] constexpr bool present(const V& value)
{
    if constexpr (requires { value.has_value(); }) {
        return value.has_value();
    } else if constexpr (std::is_pointer_v<V>) {
        return value != nullptr;
    } else {
        (void)value;
        return true;
    }
}

/** @brief `non_empty` rule: containers/optionals must hold something; scalars pass. */
template<class V>
[[nodiscard]] constexpr bool empty(const V& value)
{
    if constexpr (requires { value.empty(); }) {
        return value.empty();
    } else if constexpr (requires { value.has_value(); }) {
        return !value.has_value();
    } else {
        (void)value;
        return false;
    }
}

/** @brief Effective wire key for a field — the override when set, else the declared name. */
template<class Fd>
[[nodiscard]] std::string_view wire_key()
{
    const std::string_view override_name = Fd::options.m_wire_name;
    return override_name.empty() ? Fd::name.string_view() : override_name;
}

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

    /**
     * @brief Builds the field-filtered wire document for `object`.
     * @param object the model instance to read fields from.
     * @param fields declared field names to include; empty means every field in declaration
     * order. Matching uses `FieldDesc::name`; emitted keys honour `FieldOptions::wire_name`.
     * @return a typed `serde::Value::Object` document, ready for `Ser::serialize`.
     */
    [[nodiscard]] static serde::Value
    build_value(const T& object, std::span<const std::string_view> fields = {})
    {
        serde::Value::Object document{};
        const auto wanted = [fields](std::string_view name)
        {
            if (fields.empty()) {
                return true;
            }
            return std::ranges::find(fields, name) != fields.end();
        };

        std::apply(
            [&](auto... field)
            {
                (
                    [&]
                    {
                        using Fd = decltype(field);
                        if (!wanted(Fd::name.string_view())) {
                            return;
                        }
                        // Keep the whole normalize -> FieldConverter -> rfl::to_generic chain in
                        // one full-expression: getters may return temporaries.
                        auto converted = FieldConverter<
                            std::remove_cvref_t<decltype(Fd::normalize((object.*Fd::getter)()))>>::
                            to_rfl(Fd::normalize((object.*Fd::getter)()));
                        document.insert(
                            std::string{serializer_detail::wire_key<Fd>()},
                            serde::Value{rfl::to_generic(converted)}
                        );
                    }(),
                    ...);
            },
            serde::Serializable<T>::fields()
        );
        return serde::Value{std::move(document)};
    }

    /**
     * @brief Serializes only the requested declared fields, in declaration order.
     * @param fields field names to include; empty means all fields.
     * @return the encoded document, or an error payload from `Ser::serialize`.
     */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    serialize(std::span<const std::string_view> fields = {}) const
    {
        const auto bytes =
            Ser::serialize(content_type, build_value(static_cast<const T&>(*this), fields));
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }

    /**
     * @brief Runs the model's own bool-returning `validate()` (when present), then every
     * `FieldOptions` rule in declaration order, returning the first failure.
     */
    [[nodiscard]] std::expected<void, core::error::AppError> validate() const
    {
        return validate_value(static_cast<const T&>(*this));
    }

    /// @brief Static counterpart of `validate()`, usable without a CRTP instance.
    [[nodiscard]] static std::expected<void, core::error::AppError> validate_value(const T& object)
    {
        std::expected<void, core::error::AppError> outcome{};

        if constexpr (serializer_detail::IHasValidate<T>) {
            if (!object.validate()) {
                return std::unexpected{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::VALIDATION_FAILED,
                    "validation failed"
                )};
            }
        }

        std::apply(
            [&](auto... field)
            {
                (
                    [&]
                    {
                        if (!outcome) {
                            return;
                        }
                        using Fd = decltype(field);
                        const std::string name{serializer_detail::wire_key<Fd>()};
                        const auto check = [&](const auto& value)
                        {
                            if (Fd::options.m_required && !serializer_detail::present(value)) {
                                outcome = std::unexpected{core::error::AppErrorBuilder{}(
                                    core::error::AppErrorKind::VALIDATION_FAILED,
                                    std::format("required field '{}' is missing", name)
                                )};
                                return;
                            }
                            if (Fd::options.m_non_empty && serializer_detail::empty(value)) {
                                outcome = std::unexpected{core::error::AppErrorBuilder{}(
                                    core::error::AppErrorKind::VALIDATION_FAILED,
                                    std::format("field '{}' must not be empty", name)
                                )};
                                return;
                            }
                            if (Fd::options.m_validator != nullptr &&
                                !Fd::options.m_validator(
                                    static_cast<const void*>(std::addressof(value))
                                )) {
                                outcome = std::unexpected{core::error::AppErrorBuilder{}(
                                    core::error::AppErrorKind::VALIDATION_FAILED,
                                    std::format("field '{}' failed validation", name)
                                )};
                            }
                        };
                        check((object.*Fd::getter)());
                    }(),
                    ...);
            },
            serde::Serializable<T>::fields()
        );
        return outcome;
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
        T model = std::move(*parsed);
        auto validated = validate_value(model);
        if (!validated) {
            return std::unexpected{std::move(validated.error())};
        }
        return model;
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
        auto validated = validate_value(static_cast<const T&>(*this));
        if (!validated) {
            validated.error().send(res, send);
            return;
        }
        auto text = from_model(content_type);
        if (!text) {
            text.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        std::vector<std::byte> body_bytes{};
        body_bytes.reserve(text->size());
        for (const char ch : *text) {
            body_bytes.push_back(static_cast<std::byte>(ch));
        }
        res.set_body(std::move(body_bytes));
        send();
    }

    void to_response(interfaces::io::ResponseSink& sink) const
    {
        auto validated = validate_value(static_cast<const T&>(*this));
        if (!validated) {
            validated.error().send(sink);
            return;
        }
        auto text = from_model(content_type);
        if (!text) {
            text.error().send(sink);
            return;
        }
        sink.set_status(interfaces::io::types::Status::OK);
        std::vector<std::byte> body_bytes{};
        body_bytes.reserve(text->size());
        for (const char ch : *text) {
            body_bytes.push_back(static_cast<std::byte>(ch));
        }
        sink.set_body(std::move(body_bytes));
        sink.send();
    }

    /**
     * @brief Parses and validates a request into T; on failure writes the error to `sink` and
     * returns nullopt so the caller can bail out before routing.
     */
    [[nodiscard]] static std::optional<T>
    pass(interfaces::io::IRequest& req, interfaces::io::ResponseSink& sink)
    {
        auto parsed = from_request(req);
        if (!parsed) {
            parsed.error().send(sink);
            return std::nullopt;
        }
        auto validated = validate_value(*parsed);
        if (!validated) {
            validated.error().send(sink);
            return std::nullopt;
        }
        return std::move(*parsed);
    }
};

/**
 * @brief CRTP serializer for vectors of serializable models — each element is reduced to the
 * same field-filtered object `Serializer<T>::serialize(fields)` emits, collected in a
 * `serde::Value::Array`, then encoded as one document.
 */
template<class T>
class Serializer<std::vector<T>> : public std::vector<T>
{
public:
    static constexpr std::string_view content_type = json_content_type;

    [[nodiscard]] std::expected<std::string, core::error::AppError>
    serialize(std::span<const std::string_view> fields = {}) const
    {
        return serialize_values(static_cast<const std::vector<T>&>(*this), fields);
    }

    [[nodiscard]] static std::expected<std::string, core::error::AppError>
    serialize_values(const std::vector<T>& elements, std::span<const std::string_view> fields = {})
    {
        serde::Value::Array array{};
        for (const auto& element: elements) {
            // Per-element field-filtered serialization, kept as a Value so no extra wire
            // round-trip is needed before the whole array is encoded.
            array.emplace_back(Serializer<T>::build_value(element, fields));
        }
        const auto bytes = Ser::serialize(content_type, serde::Value{std::move(array)});
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }
};

} // namespace serde

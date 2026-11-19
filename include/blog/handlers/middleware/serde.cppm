export module blog_handlers_middleware:serde;

import std;
import interfaces;
import core_serde;
import :error;

export namespace blog::handlers::middleware {

/** @brief Normalized media type header left on the request by the middleware. */
inline constexpr std::string_view kContentTypeHeader{"x-blog-content-type"};
inline constexpr std::string_view kJsonMediaType{"application/json"};

/**
 * @brief Ambient media type selected from the request headers. Middleware is a plain
 * function pointer, so the extracted type is stored here for workflow steps that have
 * no direct access to IRequest.
 */
class MediaTypeRegistry
{
public:
    static void set_active(std::string_view type) noexcept
    {
        s_active = type;
    }

    [[nodiscard]] static std::string_view active() noexcept
    {
        return s_active;
    }

private:
    static inline std::string s_active{std::string{kJsonMediaType}};
};

/** @brief Built-in JSON serde format; no dlopen'd plugin required for the Ghost API. */
class JsonSerdeFormat final : public interfaces::ISerdeFormat
{
public:
    [[nodiscard]] std::string_view content_type() const noexcept override
    {
        return kJsonMediaType;
    }

    [[nodiscard]] std::string_view format_name() const noexcept override
    {
        return "json";
    }

    [[nodiscard]] std::expected<std::string, std::string>
    encode(const serde::Value& value) const override
    {
        return core_serde::JsonCodec{}.encode(value);
    }

    [[nodiscard]] std::expected<serde::Value, std::string>
    decode(std::string_view data) const override
    {
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(data);
        if (!value) {
            return std::unexpected{std::string{"invalid JSON payload"}};
        }
        return *value;
    }
};

/** @brief Registers the built-in JSON format exactly once, before any Ser call. */
inline void install_serde_formats()
{
    static serde::SerdeFormatRegistry registry;
    static const bool installed = [] {
        registry.add_format(std::make_shared<JsonSerdeFormat>());
        serde::SerdeFormatRegistry::set_active(&registry);
        return true;
    }();
    (void)installed;
}

/** @brief Media type extracted from the request (falls back to the ambient value). */
[[nodiscard]] inline std::string_view content_type(const interfaces::io::IRequest& request)
{
    const auto normalized = request.find_header(kContentTypeHeader);
    return normalized.empty() ? MediaTypeRegistry::active() : normalized;
}

/**
 * @brief Rejects requests whose content-type / accept header is missing or unsupported,
 * then normalizes the accepted media type for downstream handlers and steps.
 */
inline void content_type_middleware(
    interfaces::io::IRequest& request,
    interfaces::io::IResponse& response,
    interfaces::NextFn&& next,
    std::function<void()> send
)
{
    const auto type = request.find_header(interfaces::io::types::Token::CONTENT_TYPE);
    if (type.empty() || !type.starts_with(kJsonMediaType)) {
        write_error(
            response,
            interfaces::io::types::Status::UNSUPPORTED_MEDIA_TYPE,
            ErrorEnvelope{std::vector{ErrorItem{
                .message = "Unsupported Media Type",
                .type = "UnsupportedMediaTypeError"
            }}},
            std::move(send)
        );
        return;
    }
    const auto accept = request.find_header(interfaces::io::types::Token::ACCEPT);
    if (accept.empty() || (!accept.starts_with(kJsonMediaType) && accept != "*/*")) {
        write_error(
            response,
            interfaces::io::types::Status::NOT_ACCEPTABLE,
            ErrorEnvelope{std::vector{ErrorItem{
                .message = "Not Acceptable",
                .type = "NotAcceptableError"
            }}},
            std::move(send)
        );
        return;
    }
    MediaTypeRegistry::set_active(type);
    request.set_header(kContentTypeHeader, type);
    next(request, response, std::move(send));
}

/** @brief Serde encode with the request media type; returns wire-ready text. */
template <class T>
[[nodiscard]] std::string encode_with(std::string_view type, const T& value)
{
    const auto bytes = serde::Ser::serialize(type, value);
    return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

/** @brief Serde decode with the request media type. */
template <class T>
[[nodiscard]] std::optional<T> decode_with(std::string_view type, std::string_view text)
{
    auto decoded = serde::Ser::deserialize<T>(type, text);
    if (!decoded) {
        return std::nullopt;
    }
    return std::optional<T>{std::move(*decoded)};
}

/** @brief Media type extracted from the current request by the middleware. */
[[nodiscard]] inline std::string_view media_type() noexcept
{
    return MediaTypeRegistry::active();
}

/** @brief Serde encode using the current request's media type. */
template <class T>
[[nodiscard]] std::string encode(const T& value)
{
    return encode_with(media_type(), value);
}

/** @brief Serde decode using the current request's media type. */
template <class T>
[[nodiscard]] std::optional<T> decode(std::string_view text)
{
    return decode_with<T>(media_type(), text);
}

} // namespace blog::handlers::middleware

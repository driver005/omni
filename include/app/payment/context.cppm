export module app_payment:context;

import std;
import interfaces;
import core_behavior;
import core_serde;
import app_auth;
import app_http;

export namespace app::payment::detail {

/** @brief Materializes the HTTP-free request context shared by every payment runtime. */
[[nodiscard]] inline core::behavior::CallContext make_call_context(interfaces::io::IRequest& req)
{
    core::behavior::CallContext result{};

    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    result.merchant_id = std::string{auth.merchant_id()};
    result.profile_id = std::string{auth.profile_id()};
    result.org_id = std::string{auth.org_id()};
    if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
        result.connected_merchant_id = std::string{*connected};
    }
    if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
        result.client_secret = std::string{*secret};
    }
    for (const auto& [key, value]: auth.claims()) {
        result.claims[key] = value;
    }

    const auto segments = ::app::http::RequestPath{}(req);
    for (std::size_t i = 0; i < segments.count(); ++i) {
        if (const auto segment = segments.at(i)) {
            result.path_segments.emplace_back(*segment);
        }
    }

    const std::string_view path = req.get_path();
    const std::size_t question = path.find('?');
    if (question != std::string_view::npos) {
        std::size_t start = question + 1;
        while (start < path.size()) {
            const std::size_t amp = path.find('&', start);
            const std::size_t end = amp == std::string_view::npos ? path.size() : amp;
            const std::string_view pair = path.substr(start, end - start);
            const std::size_t equals = pair.find('=');
            const std::string_view key =
                equals == std::string_view::npos ? pair : pair.substr(0, equals);
            const std::string_view value =
                equals == std::string_view::npos ? std::string_view{} : pair.substr(equals + 1);
            if (!key.empty()) {
                result.query[std::string{key}] = std::string{value};
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }

    for (const auto& entry: req.get_headers()) {
        std::visit(
            [&](const auto& header)
            {
                if (!header) {
                    return;
                }
                using Header = std::remove_cvref_t<decltype(*header)>;
                if constexpr (std::same_as<Header, ::interfaces::io::HeaderField<true>>) {
                    result.headers[std::string{
                        ::interfaces::io::token_to_string(header->get_name())
                    }] = header->get_value();
                } else {
                    result.headers[std::string{header->get_name()}] = header->get_value();
                }
            },
            entry
        );
    }
    return result;
}

/** @brief Auth-scope view consumed by the domain support helpers. */
[[nodiscard]] inline core::behavior::Scope scope_of(const core::behavior::CallContext& ctx)
{
    return core::behavior::Scope{
        ctx.merchant_id,
        ctx.profile_id,
        ctx.org_id,
        ctx.connected_merchant_id
    };
}

[[nodiscard]] inline std::string
path_at(const core::behavior::CallContext& ctx, std::size_t index)
{
    return index < ctx.path_segments.size() ? ctx.path_segments[index] : std::string{};
}

[[nodiscard]] inline std::string last_path(const core::behavior::CallContext& ctx)
{
    return ctx.path_segments.empty() ? std::string{} : ctx.path_segments.back();
}

[[nodiscard]] inline std::string
query_of(const core::behavior::CallContext& ctx, std::string_view key, std::string_view fallback = {})
{
    const auto it = ctx.query.find(std::string{key});
    return it == ctx.query.end() ? std::string{fallback} : it->second;
}

[[nodiscard]] inline std::string
claim_of(const core::behavior::CallContext& ctx, std::string_view key)
{
    const auto it = ctx.claims.find(std::string{key});
    return it == ctx.claims.end() ? std::string{} : it->second;
}

[[nodiscard]] inline std::string
header_of(const core::behavior::CallContext& ctx, std::string_view name)
{
    const auto it = ctx.headers.find(std::string{name});
    return it == ctx.headers.end() ? std::string{} : it->second;
}

/** @brief Builds a DTO-shaped field from whatever parsed request the handler supplies. */
template<class Field, class Dto>
[[nodiscard]] Field as_dto(const Dto& dto)
{
    if constexpr (std::constructible_from<Field, Dto>) {
        return Field{dto};
    } else {
        return Field{};
    }
}

/** @brief Stable JSON representation of a parsed DTO for raw-payload runtime fields. */
template<class Dto>
[[nodiscard]] std::string body_of(const Dto& dto)
{
    const auto bytes = serde::Ser::serialize("application/json", dto);
    return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

} // namespace app::payment::detail

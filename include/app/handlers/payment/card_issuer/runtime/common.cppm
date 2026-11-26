export module app_payment:card_issuer_runtime_common;

import std;
import core_error;
import app_context;
import app_dto;
import app_store;

export namespace app::handlers::card_issuer {

/**
 * @brief Shared business helpers for the card issuer sub-runtimes.
 *
 * Direct port of the helper block in the original CardIssuerRouteHandlers:
 * name validation, id generation, timestamps, dimension limit resolution,
 * store record construction and the ForeignTryInto response transformer.
 */
class CardIssuerRuntimeSupport
{
public:
    explicit CardIssuerRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_issuer_name(std::string_view name) const
    {
        // CardIssuerName is a nutype validated to 1..=255 chars
        // (common_utils/new_type.rs:21-24, consts.rs:157).
        if (name.empty() || name.size() > 255) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "InvalidDataValue: issuer_name"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string new_id() const
    {
        // Upstream id_type::CardIssuerId::generate() emits a 5-character nanoid
        // (common_utils/id_type/card_issuer.rs:31-35, consts.rs:283). The local id
        // keeps that length and alphanumeric shape using the injected id generator.
        const std::string raw = m_context.ids().uuid_v4();
        std::string out;
        for (const char c: raw) {
            if (c != '-') {
                out.push_back(c);
                if (out.size() == 5) {
                    break;
                }
            }
        }
        return out;
    }

    [[nodiscard]] std::string now_stamp() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            )
                .count()
        );
    }

    [[nodiscard]] std::optional<std::size_t> parse_limit(std::string_view value) const
    {
        std::int64_t parsed{};
        const auto converted = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (converted.ec == std::errc{} && converted.ptr == value.data() + value.size() &&
            parsed > 0) {
            return static_cast<std::size_t>(parsed);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::size_t configured_list_limit() const
    {
        // Upstream dimension_state::CardIssuerListMaxLimit: superposition key
        // "CARD_ISSUER_LIST_MAX_LIMIT", DB config key "card_issuer_list_max_limit",
        // default 18000 (core/configs/dimension_config.rs:1419-1429).
        constexpr std::size_t default_limit = 18'000;
        if (const auto configured =
                m_context.superposition().get_config("card_issuer", "list_max_limit")) {
            return parse_limit(configured->get_value()).value_or(default_limit);
        }
        if (const auto configured =
                m_context.config_store().get_value("configs", "card_issuer.list_max_limit")) {
            return parse_limit(*configured).value_or(default_limit);
        }
        if (const auto configured =
                m_context.config_store().get_value("configs", "card_issuer_list_max_limit")) {
            return parse_limit(*configured).value_or(default_limit);
        }
        return default_limit;
    }

    [[nodiscard]] ::app::store::card_issuers::CardIssuerRecord
    to_store_record(std::string_view id, std::string_view name) const
    {
        ::app::store::card_issuers::CardIssuerRecord record;
        record.set_issuer_id(std::string{id});
        record.set_name(std::string{name});
        record.set_issuer_type("");
        record.set_status("");
        record.set_metadata("{}");
        record.set_created_at(now_stamp());
        return record;
    }

    [[nodiscard]] ::app::dto::card_issuer::CardIssuerResponse
    response_from(const ::app::store::card_issuers::CardIssuerRecord& record) const
    {
        ::app::dto::card_issuer::CardIssuerIdValue id;
        id.set_value(record.get_issuer_id());
        ::app::dto::card_issuer::CardIssuerNameValue name;
        name.set_value(record.get_name());
        ::app::dto::card_issuer::CardIssuerResponse response;
        response.set_id(std::move(id));
        response.set_issuer_name(std::move(name));
        return response;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::card_issuer

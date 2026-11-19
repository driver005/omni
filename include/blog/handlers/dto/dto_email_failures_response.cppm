module;
export module blog_handlers:dto_email_failures_response;

import std;
import core_serde;
import :dto_pagination_envelope;
import :dto_email_failure;

export namespace blog::dto {

/** @brief Ghost API DTO: EmailFailuresResponse. */
class EmailFailuresResponse
{
public:
    void set_failures(std::vector<EmailFailure> value) { failures = std::move(value); }
    [[nodiscard]] const std::vector<EmailFailure>& get_failures() const noexcept { return failures; }
    void set_meta(PaginationEnvelope value) { meta = std::move(value); }
    [[nodiscard]] const PaginationEnvelope& get_meta() const noexcept { return meta; }

    std::vector<EmailFailure> failures{};
    PaginationEnvelope meta{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::EmailFailuresResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"failures", &blog::dto::EmailFailuresResponse::get_failures, &blog::dto::EmailFailuresResponse::set_failures, serde::FieldOptions{}>{},
            serde::FieldDesc<"meta", &blog::dto::EmailFailuresResponse::get_meta, &blog::dto::EmailFailuresResponse::set_meta, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "email_failures_response";
    }
};


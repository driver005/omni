module;
export module blog_dto:email_batches_response;

import std;
import core_serde;
import :pagination_envelope;
import :email_batch;

export namespace blog::dto {

/** @brief Ghost API DTO: EmailBatchesResponse. */
class EmailBatchesResponse
{
public:
    void set_batches(std::vector<EmailBatch> value) { batches = std::move(value); }
    [[nodiscard]] const std::vector<EmailBatch>& get_batches() const noexcept { return batches; }
    void set_meta(PaginationEnvelope value) { meta = std::move(value); }
    [[nodiscard]] const PaginationEnvelope& get_meta() const noexcept { return meta; }

    std::vector<EmailBatch> batches{};
    PaginationEnvelope meta{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::EmailBatchesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"batches", &blog::dto::EmailBatchesResponse::get_batches, &blog::dto::EmailBatchesResponse::set_batches, serde::FieldOptions{}>{},
            serde::FieldDesc<"meta", &blog::dto::EmailBatchesResponse::get_meta, &blog::dto::EmailBatchesResponse::set_meta, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "email_batches_response";
    }
};


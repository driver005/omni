export module app_handlers_disputes:result;

import std;
import core_serde;
import app_dto;
import app_model;

export namespace app::handlers::disputes {

class RetrieveDisputeResult : public app::model::disputes::DisputeResponse
{
public:
    RetrieveDisputeResult() = default;

    explicit RetrieveDisputeResult(app::model::disputes::DisputeResponse value) noexcept :
        DisputeResponse{std::move(value)}
    {
    }
};

/** @brief Top-level JSON array: handler serializes the typed item vector directly. */
class FetchDisputesResult
{
public:
    FetchDisputesResult() = default;

    explicit FetchDisputesResult(std::vector<app::model::disputes::DisputeResponse> items) noexcept
        :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::model::disputes::DisputeResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::model::disputes::DisputeResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::model::disputes::DisputeResponse> m_items{};
};

/** @brief Top-level JSON array: handler serializes the typed item vector directly. */
class RetrieveDisputesListResult
{
public:
    RetrieveDisputesListResult() = default;

    explicit RetrieveDisputesListResult(
        std::vector<app::model::disputes::DisputeResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::model::disputes::DisputeResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::model::disputes::DisputeResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::model::disputes::DisputeResponse> m_items{};
};

/** @brief Top-level JSON array: handler serializes the typed item vector directly. */
class RetrieveDisputesListProfileResult
{
public:
    RetrieveDisputesListProfileResult() = default;

    explicit RetrieveDisputesListProfileResult(
        std::vector<app::model::disputes::DisputeResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::model::disputes::DisputeResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::model::disputes::DisputeResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::model::disputes::DisputeResponse> m_items{};
};

class RetrieveDisputesListForPlatformResult :
    public app::model::disputes::PlatformDisputeListResponse
{
public:
    RetrieveDisputesListForPlatformResult() = default;

    explicit RetrieveDisputesListForPlatformResult(
        app::model::disputes::PlatformDisputeListResponse value
    ) noexcept :
        PlatformDisputeListResponse{std::move(value)}
    {
    }
};

class GetDisputesFiltersResult : public app::model::disputes::DisputeListFilters
{
public:
    GetDisputesFiltersResult() = default;

    explicit GetDisputesFiltersResult(app::model::disputes::DisputeListFilters value) noexcept :
        DisputeListFilters{std::move(value)}
    {
    }
};

class GetDisputesFiltersProfileResult : public app::model::disputes::DisputeListFilters
{
public:
    GetDisputesFiltersProfileResult() = default;

    explicit GetDisputesFiltersProfileResult(
        app::model::disputes::DisputeListFilters value
    ) noexcept :
        DisputeListFilters{std::move(value)}
    {
    }
};

class GetPlatformDisputesFiltersResult : public app::model::disputes::PlatformDisputeListFilters
{
public:
    GetPlatformDisputesFiltersResult() = default;

    explicit GetPlatformDisputesFiltersResult(
        app::model::disputes::PlatformDisputeListFilters value
    ) noexcept :
        PlatformDisputeListFilters{std::move(value)}
    {
    }
};

class AcceptDisputeResult : public app::model::disputes::DisputeResponse
{
public:
    AcceptDisputeResult() = default;

    explicit AcceptDisputeResult(app::model::disputes::DisputeResponse value) noexcept :
        DisputeResponse{std::move(value)}
    {
    }
};

class SubmitDisputeEvidenceResult : public app::model::disputes::DisputeResponse
{
public:
    SubmitDisputeEvidenceResult() = default;

    explicit SubmitDisputeEvidenceResult(app::model::disputes::DisputeResponse value) noexcept :
        DisputeResponse{std::move(value)}
    {
    }
};

class AttachDisputeEvidenceResult : public app::dto::files::CreateFileResponse
{
public:
    AttachDisputeEvidenceResult() = default;

    explicit AttachDisputeEvidenceResult(app::dto::files::CreateFileResponse value) noexcept :
        CreateFileResponse{std::move(value)}
    {
    }
};

/** @brief Top-level JSON array: handler serializes the typed item vector directly. */
class RetrieveDisputeEvidenceResult
{
public:
    RetrieveDisputeEvidenceResult() = default;

    explicit RetrieveDisputeEvidenceResult(
        std::vector<app::model::disputes::DisputeEvidenceBlock> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::model::disputes::DisputeEvidenceBlock>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::model::disputes::DisputeEvidenceBlock> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::model::disputes::DisputeEvidenceBlock> m_items{};
};

class DeleteDisputeEvidenceResult
{
public:
    DeleteDisputeEvidenceResult() = default;

    explicit DeleteDisputeEvidenceResult(std::string status) noexcept :
        m_status{std::move(status)}
    {
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

private:
    std::string m_status{};
};

class GetDisputesAggregateResult : public app::model::disputes::DisputesAggregateResponse
{
public:
    GetDisputesAggregateResult() = default;

    explicit GetDisputesAggregateResult(
        app::model::disputes::DisputesAggregateResponse value
    ) noexcept :
        DisputesAggregateResponse{std::move(value)}
    {
    }
};

class GetDisputesAggregateProfileResult : public app::model::disputes::DisputesAggregateResponse
{
public:
    GetDisputesAggregateProfileResult() = default;

    explicit GetDisputesAggregateProfileResult(
        app::model::disputes::DisputesAggregateResponse value
    ) noexcept :
        DisputesAggregateResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::disputes

template<>
struct serde::Serializable<app::handlers::disputes::RetrieveDisputeResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputeResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "retrieve_dispute_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::RetrieveDisputesListForPlatformResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::PlatformDisputeListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "retrieve_disputes_list_for_platform_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::GetDisputesFiltersResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputeListFilters>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_disputes_filters_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::GetDisputesFiltersProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputeListFilters>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_disputes_filters_profile_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::GetPlatformDisputesFiltersResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::PlatformDisputeListFilters>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_platform_disputes_filters_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::AcceptDisputeResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputeResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "accept_dispute_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::SubmitDisputeEvidenceResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputeResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "submit_dispute_evidence_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::AttachDisputeEvidenceResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::files::CreateFileResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "attach_dispute_evidence_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::DeleteDisputeEvidenceResult>
{
    static constexpr auto fields()
    {
        using T = app::handlers::disputes::DeleteDisputeEvidenceResult;
        return std::tuple{
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "status_ok_wire";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::GetDisputesAggregateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputesAggregateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_disputes_aggregate_result";
    }
};

template<>
struct serde::Serializable<app::handlers::disputes::GetDisputesAggregateProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::disputes::DisputesAggregateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_disputes_aggregate_profile_result";
    }
};

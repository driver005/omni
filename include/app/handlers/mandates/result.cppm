export module app_handlers_mandates:result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::mandates {

// The typed responses inherit the wire fields of the exact DTO the old core built
// and encoded, so field names, order and wrapper types stay byte-identical. The
// Serializable specialization reuses the DTO's FieldDesc tuple; those descriptors
// (including the bare-scalar / RawJson wire accessors) apply to the derived object
// just as they do to the base object.

class GetMandateResult : public app::model::mandates::MandateResponse
{
public:
    GetMandateResult() = default;

    explicit GetMandateResult(app::model::mandates::MandateResponse value) noexcept :
        MandateResponse{std::move(value)}
    {
    }
};

class RevokeMandateResult : public app::model::mandates::MandateRevokedResponse
{
public:
    RevokeMandateResult() = default;

    explicit RevokeMandateResult(app::model::mandates::MandateRevokedResponse value) noexcept :
        MandateRevokedResponse{std::move(value)}
    {
    }
};

// The list route returns a top-level JSON array. Carry the typed vector; the
// handler serializes get_items() so the wire stays a bare array without any
// hand-built JSON text.
class RetrieveMandatesListResult
{
public:
    RetrieveMandatesListResult() = default;

    explicit RetrieveMandatesListResult(
        std::vector<app::model::mandates::MandateResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<app::model::mandates::MandateResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<app::model::mandates::MandateResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<app::model::mandates::MandateResponse> m_items{};
};

} // namespace app::handlers::mandates

template<>
struct serde::Serializable<app::handlers::mandates::GetMandateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::mandates::MandateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_mandate_result";
    }
};

template<>
struct serde::Serializable<app::handlers::mandates::RevokeMandateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::mandates::MandateRevokedResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "revoke_mandate_result";
    }
};

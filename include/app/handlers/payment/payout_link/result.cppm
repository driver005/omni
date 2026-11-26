export module app_payment:payout_link_result;

import std;
import core_serde;

export namespace app::handlers::payout_link {

// The render response is a hand-built GenericLinks document (an externally
// tagged `data` payload that the flat DTOs cannot express). The runtime builds
// it as a typed serde::Value; the handler serializes that document directly.
class PayoutLinkRenderResult
{
public:
    PayoutLinkRenderResult() = default;

    explicit PayoutLinkRenderResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

} // namespace app::handlers::payout_link

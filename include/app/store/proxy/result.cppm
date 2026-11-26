export module app_store:proxy_result;

import std;
import core_serde;

export namespace app::handlers::proxy {

/**
 * @brief Typed ProxyResponse wire result.
 *
 * Upstream shape `{"response":...,"status_code":...,"response_headers":...}` is
 * carried as a serde::Value (rfl::Generic) document assembled from typed field
 * assignments. The handler serializes the document with Ser::serialize; no
 * hand-written JSON text crosses the response boundary.
 */
class ProxyResult
{
public:
    ProxyResult() = default;

    explicit ProxyResult(serde::Value document) noexcept :
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
    serde::Value m_document{serde::Value::Object{}};
};

} // namespace app::handlers::proxy

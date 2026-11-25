export module app_handlers_superposition_proxy:result;

import std;
import core_serde;

export namespace app::handlers::superposition_proxy {

/**
 * Every response of this domain is a typed serde::Value wire document. The
 * runtime builds it with typed field assignments (paginated envelopes,
 * transparent resolve map, single-object DTOs); the handler encodes the
 * document with serde::Ser::serialize. No hand-written JSON text is carried.
 */
class SuperpositionProxyRawResult
{
public:
    SuperpositionProxyRawResult() = default;

    explicit SuperpositionProxyRawResult(serde::Value document) noexcept :
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

class ListContextsResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class CreateContextResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class ListDefaultConfigsResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class GetDefaultConfigResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class ListDimensionsResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class GetDimensionResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class ResolveDetailedConfigResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class ResolveConfigExplanationResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

class ListAuditLogsResult : public SuperpositionProxyRawResult
{
public:
    using SuperpositionProxyRawResult::SuperpositionProxyRawResult;
};

} // namespace app::handlers::superposition_proxy

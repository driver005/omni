export module app_handlers_cache:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_invalidate;

export namespace app::handlers::cache {

class CacheRuntime
{
public:
    explicit CacheRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_invalidate{context}
    {
    }

    [[nodiscard]] std::expected<CacheInvalidateResult, core::error::AppError>
    invalidate(const CacheInvalidateInput& input) const
    {
        return m_invalidate.invalidate(input);
    }

private:
    app::context::AppContext& m_context;
    InvalidateRuntime m_invalidate;
};

} // namespace app::handlers::cache

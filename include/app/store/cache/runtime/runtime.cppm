export module app_store:cache_runtime;

import std;
import core_error;
import app_context;

import :cache_input;
import :cache_runtime_common;
import :cache_runtime_invalidate;

export namespace app::handlers::cache {

class CacheRuntime
{
public:
    explicit CacheRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_invalidate{context}
    {
    }

    [[nodiscard]] std::expected<void, core::error::AppError>
    invalidate(const CacheInvalidateInput& input) const
    {
        return m_invalidate.invalidate(input);
    }

private:
    ::app::context::AppContext& m_context;
    InvalidateRuntime m_invalidate;
};

} // namespace app::handlers::cache

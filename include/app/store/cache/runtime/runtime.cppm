export module app_store:cache_runtime;

import std;
import core_behavior;
import core_error;
import app_context;

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

    [[nodiscard]] core::behavior::Outcome<void>
    invalidate(const core::behavior::CallContext& ctx) const
    {
        return m_invalidate.invalidate(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    InvalidateRuntime m_invalidate;
};

} // namespace app::handlers::cache

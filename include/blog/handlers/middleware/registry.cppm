export module blog_handlers_middleware:registry;

import std;


export namespace blog::handlers::middleware {

/**
 * @brief Process-level active context for stateless middleware functions.
 * `interfaces::MiddlewareFn` is a function pointer, so it cannot capture a
 * context; this mirrors `core::events::EventBusRegistry::set_active()`.
 */
class MiddlewareContextRegistry
{
public:
    static void set_active(void* context) noexcept
    {
        s_active = context;
    }

    [[nodiscard]] static void* active() noexcept
    {
        return s_active;
    }

private:
    static inline void* s_active{nullptr};
};

} // namespace blog::handlers::middleware

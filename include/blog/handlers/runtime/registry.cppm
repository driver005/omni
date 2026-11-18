export module blog_handlers_runtime:registry;

import std;
import blog_context;

export namespace blog::handlers::runtime {

/**
 * @brief Process-level active services for stateless operation handlers.
 * Uses std::any so this low-level module does not depend on blog_handlers.
 */
class ServiceRegistry
{
public:
    static void configure(
        blog::BlogContext& context,
        std::any special,
        std::any stripe
    ) noexcept
    {
        s_context = &context;
        s_special = std::move(special);
        s_stripe = std::move(stripe);
    }

    [[nodiscard]] static blog::BlogContext& context() noexcept { return *s_context; }

    template<typename T>
    [[nodiscard]] static T& special() noexcept
    {
        return std::any_cast<std::reference_wrapper<T>>(s_special).get();
    }

    template<typename T>
    [[nodiscard]] static T& stripe() noexcept
    {
        return std::any_cast<std::reference_wrapper<T>>(s_stripe).get();
    }

private:
    static inline blog::BlogContext* s_context{nullptr};
    static inline std::any s_special{};
    static inline std::any s_stripe{};
};

} // namespace blog::handlers::runtime

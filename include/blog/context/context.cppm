export module blog_context:context;

import std;
import :datastore;
import :resource_store;
import :resource_schema;
import :query;
import :json;

export namespace blog {

/**
 * @brief Shared state assembled once when the blog plugin loads. Handlers
 * receive a reference to this context instead of owning global state.
 */
class BlogContext
{
public:
    BlogContext() = default;

    [[nodiscard]] api::ApiDataStore& store() noexcept
    {
        return m_store;
    }

    [[nodiscard]] const api::ApiDataStore& store() const noexcept
    {
        return m_store;
    }

private:
    api::ApiDataStore m_store{};
};

} // namespace blog

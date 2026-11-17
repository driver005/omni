export module blog_api:api;

import std;
import :request;
import :response;
import :dispatcher;
import :route;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Opaque Ghost API handle. All members inline, no free/static functions. */
class Api
{
public:
    Api() = default;

    Api(const Api&) = delete;
    Api& operator=(const Api&) = delete;
    Api(Api&&) noexcept = default;
    Api& operator=(Api&&) noexcept = default;
    ~Api() = default;

    [[nodiscard]] ApiResponse dispatch(const ApiRequest& request)
    {
        return m_dispatcher.dispatch(request);
    }

    [[nodiscard]] const std::vector<ApiRoute>& routes() const noexcept
    {
        return m_dispatcher.table().routes();
    }

    [[nodiscard]] std::size_t route_count() const noexcept
    {
        return m_dispatcher.table().size();
    }

    [[nodiscard]] std::size_t content_route_count() const noexcept
    {
        return m_dispatcher.table().content_count();
    }

private:
    ApiDispatcher m_dispatcher{};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_api"> blog_api_api_suite = []
{
    "facade serves all routes"_test = []
    {
        Api api;
        expect(api.route_count() == 261);
        expect(api.content_route_count() == 20);
        expect(api.dispatch(ApiRequest{"GET", "/posts"}).ok());
    };
};
} // namespace blog::api::tests
#endif

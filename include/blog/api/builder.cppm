export module blog_api:builder;

import std;
import :api;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Fluent entry point — sole public way to construct an Api handle. */
class ApiBuilder
{
public:
    ApiBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Api build() const
    {
        return Api{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_builder"> blog_api_builder_suite = []
{
    "builds an Api handle"_test = []
    {
        const Api api = ApiBuilder{}.option("version", "v5.0").build();
        expect(api.route_count() == 261);
    };
};
} // namespace blog::api::tests
#endif

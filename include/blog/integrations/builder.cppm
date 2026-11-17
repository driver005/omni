export module blog_integrations:builder;

import std;
import :integrations;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::integrations {
class IntegrationServiceBuilder
{
public:
    [[nodiscard]] IntegrationService build() const { return IntegrationService{}; }
};
} // namespace blog::integrations
#ifdef CONGELADO_TEST
namespace blog::integrations::tests {
using namespace boost::ut;
suite<"blog_integrations_builder"> blog_integrations_builder_suite = []
{
    "builds integration service"_test = [] { expect(IntegrationServiceBuilder{}.build().targets("x").empty()); };
};
} // namespace blog::integrations::tests
#endif

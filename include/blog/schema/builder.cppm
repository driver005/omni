export module blog_schema:builder;

import std;
import :schema;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::schema {

/** @brief Fluent entry point for the schema registry. */
class SchemaBuilder
{
public:
    [[nodiscard]] SchemaRegistry build() const
    {
        return SchemaRegistry{};
    }
};

} // namespace blog::schema

#ifdef CONGELADO_TEST
namespace blog::schema::tests {
using namespace boost::ut;
suite<"blog_schema_builder"> blog_schema_builder_suite = []
{
    "builds a registry"_test = []
    {
        const SchemaRegistry registry = SchemaBuilder{}.build();
        expect(registry.version() == 0);
    };
};
} // namespace blog::schema::tests
#endif

export module blog_members:builder;

import std;
import core_database;
import :members;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::members {
class MemberServiceBuilder
{
public:
    explicit MemberServiceBuilder(core::db::Database& db) : m_db{db} {}
    [[nodiscard]] MemberService build() const { return MemberService{m_db}; }
private:
    core::db::Database& m_db;
};
} // namespace blog::members
#ifdef CONGELADO_TEST
namespace blog::members::tests {
using namespace boost::ut;
suite<"blog_members_builder"> blog_members_builder_suite = []
{
    "build test placeholder"_test = [] { expect(true); };
};
} // namespace blog::members::tests
#endif

export module blog_members:members;

import std;
import blog_model;
import blog_auth;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::members {

/** @brief Member service: signup and status over the core auth + database facades. */
class MemberService
{
public:
    explicit MemberService(core::db::Database& db) :
        m_repo{db}
    {
    }

    [[nodiscard]] std::optional<model::Member> signup(std::string email, std::string name)
    {
        model::Member member;
        member.set_email(std::move(email));
        member.set_name(std::move(name));
        member.set_status("free");
        return m_repo.insert(std::move(member));
    }

private:
    core::db::RepositoryBase<model::Member> m_repo;
};

} // namespace blog::members

#ifdef CONGELADO_TEST
namespace blog::members::tests {
using namespace boost::ut;
suite<"blog_members_members"> blog_members_members_suite = []
{
    "signup test placeholder"_test = [] { expect(true); };
};
} // namespace blog::members::tests
#endif

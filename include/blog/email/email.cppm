export module blog_email:email;

import std;
import core_email;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::email {

/** @brief Newsletter/transactional email service over the core email facade. */
class EmailService
{
public:
    EmailService()
    {
        m_email = core::email::EmailBuilder{}.build();
    }

    bool send(std::string to, std::string subject, std::string body)
    {
        const core::email::EmailMessage message{
            core::email::EmailAddress{"noreply@example.com"},
            core::email::EmailAddress{std::move(to)},
            std::move(subject)
        };
        auto configured = message;
        configured.set_text(std::move(body));
        return m_email.send(configured);
    }

    [[nodiscard]] std::size_t sent() const noexcept
    {
        return m_email.sent_count();
    }

private:
    core::email::Email m_email{};
};

} // namespace blog::email
#ifdef CONGELADO_TEST
namespace blog::email::tests {
using namespace boost::ut;
suite<"blog_email_email"> blog_email_email_suite = []
{
    "email service placeholder"_test = [] { expect(true); };
};
} // namespace blog::email::tests
#endif

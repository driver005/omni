export module core_email:message;

import std;
import :address;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {

/** @brief An outbound email message with plain-text and HTML bodies. */
class EmailMessage
{
public:
    EmailMessage() = default;

    EmailMessage(EmailAddress from, EmailAddress to, std::string subject) :
        m_from{std::move(from)},
        m_to{std::move(to)},
        m_subject{std::move(subject)}
    {
    }

    [[nodiscard]] const EmailAddress& from() const noexcept
    {
        return m_from;
    }

    [[nodiscard]] const EmailAddress& to() const noexcept
    {
        return m_to;
    }

    [[nodiscard]] const std::string& subject() const noexcept
    {
        return m_subject;
    }

    [[nodiscard]] const std::string& text() const noexcept
    {
        return m_text;
    }

    [[nodiscard]] const std::string& html() const noexcept
    {
        return m_html;
    }

    EmailMessage& set_text(std::string body)
    {
        m_text = std::move(body);
        return *this;
    }

    EmailMessage& set_html(std::string body)
    {
        m_html = std::move(body);
        return *this;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return m_from.valid() && m_to.valid() && !m_subject.empty();
    }

private:
    EmailAddress m_from{};
    EmailAddress m_to{};
    std::string m_subject{};
    std::string m_text{};
    std::string m_html{};
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_email_message"> core_email_email_message_suite = []
{
    "carries sender, recipient, subject and bodies"_test = []
    {
        EmailMessage message{
            EmailAddress{"from@example.com"}, EmailAddress{"to@example.com"}, "Hello"
        };
        message.set_text("plain").set_html("<b>html</b>");
        expect(message.valid());
        expect(message.subject() == "Hello");
        expect(message.text() == "plain");
        expect(message.html() == "<b>html</b>");
    };
};

} // namespace core::email::tests
#endif

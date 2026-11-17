export module memory_backends:email;

import std;
import core_email;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::email {

/** @brief Reference provider: records sent messages in memory (for tests/dev). */
class LocalEmailProvider final : public EmailProvider
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "local";
    }

    bool send(const EmailMessage& message) override
    {
        if (!message.valid()) {
            return false;
        }
        m_sent.push_back(message);
        return true;
    }

    [[nodiscard]] std::size_t sent_count() const noexcept override
    {
        return m_sent.size();
    }

    [[nodiscard]] const std::vector<EmailMessage>& sent() const noexcept
    {
        return m_sent;
    }

private:
    std::vector<EmailMessage> m_sent{};
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_local_email_provider"> core_email_local_email_provider_suite = []
{
    "records valid messages only"_test = []
    {
        LocalEmailProvider provider;
        const EmailMessage message{
            EmailAddress{"from@example.com"}, EmailAddress{"to@example.com"}, "Subject"
        };
        expect(provider.send(message));
        expect(provider.sent_count() == 1);
        expect(!provider.send(EmailMessage{}));
        expect(provider.sent_count() == 1);
    };
};

} // namespace core::email::tests
#endif

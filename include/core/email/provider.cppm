export module core_email:provider;

import std;
import :message;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {

/** @brief Internal provider contract; concrete SMTP transport is a plugin. */
class EmailProvider
{
public:
    virtual ~EmailProvider() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    virtual bool send(const EmailMessage& message) = 0;
    [[nodiscard]] virtual std::size_t sent_count() const noexcept = 0;
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_email_provider"> core_email_email_provider_suite = []
{
    "provider contract is abstract"_test = []
    {
        expect(std::is_abstract_v<EmailProvider>);
    };
};

} // namespace core::email::tests
#endif

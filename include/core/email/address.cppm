export module core_email:address;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {

/** @brief A validated email address with an optional display name. */
class EmailAddress
{
public:
    EmailAddress() = default;

    EmailAddress(std::string address, std::string name = {}) :
        m_address{std::move(address)},
        m_name{std::move(name)}
    {
    }

    [[nodiscard]] const std::string& address() const noexcept
    {
        return m_address;
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        const auto at = m_address.find('@');
        return at != std::string::npos && at > 0 && at + 1 < m_address.size() &&
               m_address.find('.', at + 1) != std::string::npos;
    }

private:
    std::string m_address{};
    std::string m_name{};
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_email_address"> core_email_email_address_suite = []
{
    "validates addresses"_test = []
    {
        expect(EmailAddress{"a@example.com"}.valid());
        expect(!EmailAddress{"bad"}.valid());
        expect(!EmailAddress{}.valid());
    };
};

} // namespace core::email::tests
#endif

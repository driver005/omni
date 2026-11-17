export module core_email:builder;

import std;
import :email;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {

/** @brief Fluent entry point — sole public way to construct an Email handle. */
class EmailBuilder
{
public:
    EmailBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Email build() const
    {
        return Email{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_email_builder"> core_email_email_builder_suite = []
{
    "builds an Email handle"_test = []
    {
        const Email email = EmailBuilder{}.option("provider", "local").build();
        expect(email.backend_name() == "local");
    };
};

} // namespace core::email::tests
#endif

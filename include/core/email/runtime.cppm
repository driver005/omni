export module core_email:runtime;

import std;
import :provider;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {
class EmailRuntime;
}

namespace core::email {

/** @brief Internal state owned by the facade. */
class EmailRuntime
{
public:
    explicit EmailRuntime(std::unique_ptr<EmailProvider> provider) :
        m_provider{std::move(provider)}
    {
    }

    EmailProvider& provider() noexcept
    {
        return *m_provider;
    }

    [[nodiscard]] const EmailProvider& provider() const noexcept
    {
        return *m_provider;
    }

private:
    std::unique_ptr<EmailProvider> m_provider{};
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_runtime"> core_email_runtime_suite = []
{
    "owns a local provider"_test = []
    {
        EmailRuntime runtime;
        expect(runtime.provider().name() == "local");
    };
};

} // namespace core::email::tests
#endif

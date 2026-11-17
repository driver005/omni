export module core_email:email;

import std;
import :provider;
import :address;
import :message;
import :template_engine;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {

/** @brief Opaque email handle. All members inline, shared_ptr pImpl. */
class Email
{
public:
    Email() :
        m_runtime{std::make_shared<EmailRuntime>(nullptr)}
    {
    }

    void use_backend(std::unique_ptr<EmailProvider> backend)
    {
        m_runtime = std::make_shared<EmailRuntime>(std::move(backend));
    }

    Email(const Email&) = delete;
    Email& operator=(const Email&) = delete;
    Email(Email&&) noexcept = default;
    Email& operator=(Email&&) noexcept = default;
    ~Email() = default;

    bool send(const EmailMessage& message)
    {
        return m_runtime->provider().send(message);
    }

    [[nodiscard]] std::string render(
        std::string_view source,
        const std::map<std::string, std::string>& values
    ) const
    {
        return m_template_engine.render(source, values);
    }

    [[nodiscard]] std::size_t sent_count() const noexcept
    {
        return m_runtime->provider().sent_count();
    }

    [[nodiscard]] std::string_view backend_name() const noexcept
    {
        return m_runtime->provider().name();
    }

private:
    std::shared_ptr<EmailRuntime> m_runtime{};
    TemplateEngine m_template_engine{};
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_email"> core_email_email_suite = []
{
    "renders and sends through the facade"_test = []
    {
        Email email;
        const auto rendered = email.render("Hi {{name}}", {{"name", "Ada"}});
        expect(rendered == "Hi Ada");
        const EmailMessage message{
            EmailAddress{"from@example.com"}, EmailAddress{"to@example.com"}, rendered
        };
        expect(email.send(message));
        expect(email.sent_count() == 1);
        expect(email.backend_name() == "local");
    };
};

} // namespace core::email::tests
#endif

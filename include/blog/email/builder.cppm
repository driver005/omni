export module blog_email:builder;
import std;
import :email;
export namespace blog::email {
class EmailServiceBuilder
{
public:
    [[nodiscard]] EmailService build() const { return EmailService{}; }
};
} // namespace blog::email

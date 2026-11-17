export module blog_redirects:builder;
import std;
import :redirects;
export namespace blog::redirects {
class RedirectRegistryBuilder
{
public:
    [[nodiscard]] RedirectRegistry build() const { return RedirectRegistry{}; }
};
} // namespace blog::redirects

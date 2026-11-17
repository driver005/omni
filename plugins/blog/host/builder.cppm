export module blog_host:builder;
import std;
import :host;
export namespace blog::host {
class HostServiceBuilder
{
public:
    [[nodiscard]] HostService build() const { return HostService{}; }
};
} // namespace blog::host

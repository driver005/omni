export module blog_storage:builder;
import std;
import :storage;
export namespace blog::storage {
class MediaServiceBuilder
{
public:
    [[nodiscard]] MediaService build() const { return MediaService{}; }
};
} // namespace blog::storage

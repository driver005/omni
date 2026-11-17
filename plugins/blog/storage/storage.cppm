export module blog_storage:storage;

import std;
import core_storage;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::storage {

/** @brief Ghost media service: stores blobs through the core storage facade. */
class MediaService
{
public:
    MediaService() :
        m_storage{core::storage::StorageBuilder{}.build()}
    {
    }

    [[nodiscard]] core::storage::StorageRef store(std::string_view payload)
    {
        return m_storage.write(payload).ref();
    }

    [[nodiscard]] std::string load(std::string_view uri) const
    {
        return m_storage.read(uri).data();
    }

private:
    core::storage::Storage m_storage{};
};

} // namespace blog::storage
#ifdef CONGELADO_TEST
namespace blog::storage::tests {
using namespace boost::ut;
suite<"blog_storage_storage"> blog_storage_storage_suite = []
{
    "stores and loads a blob"_test = []
    {
        MediaService media;
        const auto ref = media.store("blob");
        expect(ref.valid());
        expect(media.load(ref.uri()) == "blob");
    };
};
} // namespace blog::storage::tests
#endif

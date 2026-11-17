export module blog_importexport:importexport;

import std;
import core_storage;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::importexport {

/** @brief JSON export/import helper using core_storage for large payloads. */
class ImportExportService
{
public:
    ImportExportService() :
        m_storage{core::storage::StorageBuilder{}.build()}
    {
    }

    [[nodiscard]] std::string serialize(std::string_view payload)
    {
        return std::format("{{\"data\":\"{}\"}}", payload);
    }

    [[nodiscard]] std::optional<std::string> load(std::string_view uri) const
    {
        const auto result = m_storage.read(uri);
        return result.ok() ? std::optional<std::string>{result.data()} : std::nullopt;
    }

private:
    core::storage::Storage m_storage{};
};

} // namespace blog::importexport
#ifdef CONGELADO_TEST
namespace blog::importexport::tests {
using namespace boost::ut;
suite<"blog_importexport_importexport"> blog_importexport_importexport_suite = []
{
    "serializes a payload"_test = []
    {
        ImportExportService service;
        expect(service.serialize("x").contains("x"));
    };
};
} // namespace blog::importexport::tests
#endif

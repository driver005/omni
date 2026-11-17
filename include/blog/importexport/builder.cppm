export module blog_importexport:builder;
import std;
import :importexport;
export namespace blog::importexport {
class ImportExportServiceBuilder
{
public:
    [[nodiscard]] ImportExportService build() const { return ImportExportService{}; }
};
} // namespace blog::importexport

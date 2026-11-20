module;
export module app_dto:files;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::files {

class CreateFileResponse
{
public:
    void set_file_id(std::string value) { file_id = std::move(value); }
    [[nodiscard]] const std::string& get_file_id() const noexcept { return file_id; }
    std::string file_id{};
};

class FileMetadataResponse
{
public:
    void set_file_id(std::string value) { file_id = std::move(value); }
    [[nodiscard]] const std::string& get_file_id() const noexcept { return file_id; }
    void set_file_name(std::optional<std::string> value) { file_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept { return file_name; }
    void set_file_size(std::int32_t value) { file_size = std::move(value); }
    [[nodiscard]] const std::int32_t& get_file_size() const noexcept { return file_size; }
    void set_file_type(std::string value) { file_type = std::move(value); }
    [[nodiscard]] const std::string& get_file_type() const noexcept { return file_type; }
    void set_available(bool value) { available = std::move(value); }
    [[nodiscard]] const bool& get_available() const noexcept { return available; }
    std::string file_id{};
    std::optional<std::string> file_name{};
    std::int32_t file_size{};
    std::string file_type{};
    bool available{};
};

class FileRetrieveQuery
{
public:
    void set_dispute_id(std::optional<std::string> value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept { return dispute_id; }
    std::optional<std::string> dispute_id{};
};
} // namespace app::dto::files

template<>
struct serde::Serializable<app::dto::files::CreateFileResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"file_id", &app::dto::files::CreateFileResponse::get_file_id, &app::dto::files::CreateFileResponse::set_file_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_file_response"; }
};

template<>
struct serde::Serializable<app::dto::files::FileMetadataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"file_id", &app::dto::files::FileMetadataResponse::get_file_id, &app::dto::files::FileMetadataResponse::set_file_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_name", &app::dto::files::FileMetadataResponse::get_file_name, &app::dto::files::FileMetadataResponse::set_file_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_size", &app::dto::files::FileMetadataResponse::get_file_size, &app::dto::files::FileMetadataResponse::set_file_size, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_type", &app::dto::files::FileMetadataResponse::get_file_type, &app::dto::files::FileMetadataResponse::set_file_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"available", &app::dto::files::FileMetadataResponse::get_available, &app::dto::files::FileMetadataResponse::set_available, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "file_metadata_response"; }
};

template<>
struct serde::Serializable<app::dto::files::FileRetrieveQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::dto::files::FileRetrieveQuery::get_dispute_id, &app::dto::files::FileRetrieveQuery::set_dispute_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "file_retrieve_query"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::files::tests {
using namespace boost::ut;
suite<"app_dto_files"> app_dto_files_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::files::CreateFileResponse>::fields())> > 0);
    };
};
}
#endif

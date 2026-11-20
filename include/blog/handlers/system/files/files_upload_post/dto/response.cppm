export module blog_handlers:files_files_upload_post_dto;

import std;
import core_serde;

export namespace blog::dto::files {

/**
 * @brief Metadata stand-in for the Ghost storage adapter's `save({name, path, type})`.
 * No storage-adapter abstraction is registered in the allowed modules, so these
 * fields are written through the generic `files` store resource. The HTTP wire
 * response is built by the handler (`{files:[{url,ref}]}`), not by this DTO.
 */
class FileMetaDto
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_path(std::string value) { path = std::move(value); }
    [[nodiscard]] const std::string& get_path() const noexcept { return path; }
    void set_type(std::string value) { type = std::move(value); }
    [[nodiscard]] const std::string& get_type() const noexcept { return type; }

    std::string name{};
    std::string path{};
    std::string type{};
};

} // namespace blog::dto::files

template<> struct serde::Serializable<blog::dto::files::FileMetaDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &blog::dto::files::FileMetaDto::get_name,
                             &blog::dto::files::FileMetaDto::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"path", &blog::dto::files::FileMetaDto::get_path,
                             &blog::dto::files::FileMetaDto::set_path, serde::FieldOptions{}>{},
            serde::FieldDesc<"type", &blog::dto::files::FileMetaDto::get_type,
                             &blog::dto::files::FileMetaDto::set_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "files_upload_meta"; }
};

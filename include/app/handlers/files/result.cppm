export module app_handlers_files:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::files {

/**
 * @brief Create response. Inherits the wire fields of the exact DTO the old core
 * built (`app::dto::files::CreateFileResponse` -> `{"file_id":...}`).
 */
class FilesCreateResult : public app::dto::files::CreateFileResponse
{
public:
    FilesCreateResult() = default;

    explicit FilesCreateResult(app::dto::files::CreateFileResponse value) noexcept :
        CreateFileResponse{std::move(value)}
    {
    }
};

/** @brief Upstream `ApplicationResponse::StatusOk` wire shape (`core/files.rs:96,108`). */
class FilesDeleteResult
{
public:
    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    std::string status{};
};

/**
 * @brief `ApplicationResponse::FileData((bytes, mime))` (`core/files.rs:139-145`).
 * Typed DTO: file bytes and mime are reflected fields, so the handler encodes
 * the whole result via `Ser::serialize`. The mime is still emitted as the
 * response's Content-Type header by the handler.
 */
class FilesRetrieveResult
{
public:
    void set_file_data(std::string value)
    {
        file_data = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_data() const noexcept
    {
        return file_data;
    }

    void set_content_type(std::string value)
    {
        content_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_content_type() const noexcept
    {
        return content_type;
    }

private:
    std::string file_data{};
    std::string content_type{};
};

} // namespace app::handlers::files

template<>
struct serde::Serializable<app::handlers::files::FilesCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::files::CreateFileResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "files_create_result";
    }
};

template<>
struct serde::Serializable<app::handlers::files::FilesDeleteResult>
{
    static constexpr auto fields()
    {
        using T = app::handlers::files::FilesDeleteResult;
        return std::tuple{
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "files_delete_result";
    }
};

template<>
struct serde::Serializable<app::handlers::files::FilesRetrieveResult>
{
    static constexpr auto fields()
    {
        using T = app::handlers::files::FilesRetrieveResult;
        return std::tuple{
            serde::FieldDesc<
                "file_data",
                &T::get_file_data,
                &T::set_file_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "content_type",
                &T::get_content_type,
                &T::set_content_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "files_retrieve_result";
    }
};

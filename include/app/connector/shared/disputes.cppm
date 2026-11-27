module;
export module app_connector:model_shared_disputes;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::disputes {

/**
 * @brief Upstream `api_models::files::FileMetadataResponse` wire shape
 * (api_models/files.rs:10-16): file_id, file_name, file_size, file_type,
 * available. Formerly a single opaque string, which did not match upstream
 * `DisputeEvidenceBlock.file_metadata_response`.
 */
class FileMetadataResponseValue : public serde::Serializer<FileMetadataResponseValue>
{
public:
    void set_file_id(std::string value)
    {
        file_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_id() const noexcept
    {
        return file_id;
    }

    void set_file_name(std::optional<std::string> value)
    {
        file_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept
    {
        return file_name;
    }

    void set_file_size(std::int32_t value)
    {
        file_size = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_file_size() const noexcept
    {
        return file_size;
    }

    void set_file_type(std::string value)
    {
        file_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_type() const noexcept
    {
        return file_type;
    }

    void set_available(bool value)
    {
        available = std::move(value);
    }

    [[nodiscard]] const bool& get_available() const noexcept
    {
        return available;
    }

    std::string file_id{};
    std::optional<std::string> file_name{};
    std::int32_t file_size{};
    std::string file_type{};
    bool available{};
};

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorInfoValue : public serde::Serializer<MerchantConnectorInfoValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

} // namespace app::model::disputes

template<>
struct serde::Serializable<app::model::disputes::FileMetadataResponseValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "file_id",
                &app::model::disputes::FileMetadataResponseValue::get_file_id,
                &app::model::disputes::FileMetadataResponseValue::set_file_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_name",
                &app::model::disputes::FileMetadataResponseValue::get_file_name,
                &app::model::disputes::FileMetadataResponseValue::set_file_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_size",
                &app::model::disputes::FileMetadataResponseValue::get_file_size,
                &app::model::disputes::FileMetadataResponseValue::set_file_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_type",
                &app::model::disputes::FileMetadataResponseValue::get_file_type,
                &app::model::disputes::FileMetadataResponseValue::set_file_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "available",
                &app::model::disputes::FileMetadataResponseValue::get_available,
                &app::model::disputes::FileMetadataResponseValue::set_available,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "file_metadata_response_value";
    }
};

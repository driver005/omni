module;
export module app_connector:model_shared_blocklist;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::blocklist {

class BatchBlocklistJobStatusValue : public serde::Serializer<BatchBlocklistJobStatusValue>
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

class ProfileIdValue : public serde::Serializer<ProfileIdValue>
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

class ProfileCloneTargetMetadata : public serde::Serializer<ProfileCloneTargetMetadata>
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_status(BatchBlocklistJobStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const BatchBlocklistJobStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_processed_rows(std::uint32_t value)
    {
        processed_rows = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_processed_rows() const noexcept
    {
        return processed_rows;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    ProfileIdValue profile_id{};
    BatchBlocklistJobStatusValue status{};
    std::uint32_t processed_rows{};
    std::optional<std::string> error_message{};
};

class ProfileCloneJobMetadata : public serde::Serializer<ProfileCloneJobMetadata>
{
public:
    void set_targets(std::vector<ProfileCloneTargetMetadata> value)
    {
        targets = std::move(value);
    }

    [[nodiscard]] const std::vector<ProfileCloneTargetMetadata>& get_targets() const noexcept
    {
        return targets;
    }

    std::vector<ProfileCloneTargetMetadata> targets{};
};

} // namespace app::model::blocklist

template<>
struct serde::Serializable<app::model::blocklist::ProfileCloneJobMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "targets",
            &app::model::blocklist::ProfileCloneJobMetadata::get_targets,
            &app::model::blocklist::ProfileCloneJobMetadata::set_targets,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "profile_clone_job_metadata";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ProfileCloneTargetMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::blocklist::ProfileCloneTargetMetadata::get_profile_id,
                &app::model::blocklist::ProfileCloneTargetMetadata::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::blocklist::ProfileCloneTargetMetadata::get_status,
                &app::model::blocklist::ProfileCloneTargetMetadata::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processed_rows",
                &app::model::blocklist::ProfileCloneTargetMetadata::get_processed_rows,
                &app::model::blocklist::ProfileCloneTargetMetadata::set_processed_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::blocklist::ProfileCloneTargetMetadata::get_error_message,
                &app::model::blocklist::ProfileCloneTargetMetadata::set_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_clone_target_metadata";
    }
};

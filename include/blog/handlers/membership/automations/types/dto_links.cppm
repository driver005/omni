export module blog_handlers:automations_dto_links;

import std;
import core_serde;

export namespace blog::dto::automations {

class LinkRecord
{
public:
    void set_url(std::string value)
    {
        url = std::move(value);
    }

    [[nodiscard]] const std::string& get_url() const noexcept
    {
        return url;
    }

    void set_clicked_count(std::int64_t value)
    {
        clicked_count = value;
    }

    [[nodiscard]] std::int64_t get_clicked_count() const noexcept
    {
        return clicked_count;
    }

    std::string url{};
    std::int64_t clicked_count{0};
};

class GiftLinksResponse
{
public:
    void set_gift_links(std::vector<LinkRecord> value)
    {
        gift_links = std::move(value);
    }

    [[nodiscard]] const std::vector<LinkRecord>& get_gift_links() const noexcept
    {
        return gift_links;
    }

    std::vector<LinkRecord> gift_links{};
};

class AutomationLinksResponse
{
public:
    void set_data(std::vector<LinkRecord> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<LinkRecord>& get_data() const noexcept
    {
        return data;
    }

    std::vector<LinkRecord> data{};
};

} // namespace blog::dto::automations

template<>
struct serde::Serializable<blog::dto::automations::LinkRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "url",
                &blog::dto::automations::LinkRecord::get_url,
                &blog::dto::automations::LinkRecord::set_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "clicked_count",
                &blog::dto::automations::LinkRecord::get_clicked_count,
                &blog::dto::automations::LinkRecord::set_clicked_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "link_record";
    }
};

template<>
struct serde::Serializable<blog::dto::automations::GiftLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "gift_links",
            &blog::dto::automations::GiftLinksResponse::get_gift_links,
            &blog::dto::automations::GiftLinksResponse::set_gift_links,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "gift_links";
    }
};

template<>
struct serde::Serializable<blog::dto::automations::AutomationLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "data",
            &blog::dto::automations::AutomationLinksResponse::get_data,
            &blog::dto::automations::AutomationLinksResponse::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "automation_links";
    }
};

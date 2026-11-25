export module blog_handlers:gift_links_dto_links;

import std;
import core_serde;

export namespace blog::dto::gift_links {

class LinkRecord
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_url(std::string value)
    {
        url = std::move(value);
    }

    [[nodiscard]] const std::string& get_url() const noexcept
    {
        return url;
    }

    std::string id{"1"};
    std::string url{"https://ghost.local/link"};
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
    void set_links(std::vector<LinkRecord> value)
    {
        links = std::move(value);
    }

    [[nodiscard]] const std::vector<LinkRecord>& get_links() const noexcept
    {
        return links;
    }

    std::vector<LinkRecord> links{};
};

} // namespace blog::dto::gift_links

template<>
struct serde::Serializable<blog::dto::gift_links::LinkRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::dto::gift_links::LinkRecord::get_id,
                &blog::dto::gift_links::LinkRecord::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "url",
                &blog::dto::gift_links::LinkRecord::get_url,
                &blog::dto::gift_links::LinkRecord::set_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "link_record";
    }
};

template<>
struct serde::Serializable<blog::dto::gift_links::GiftLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "gift_links",
            &blog::dto::gift_links::GiftLinksResponse::get_gift_links,
            &blog::dto::gift_links::GiftLinksResponse::set_gift_links,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "gift_links";
    }
};

template<>
struct serde::Serializable<blog::dto::gift_links::AutomationLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "links",
            &blog::dto::gift_links::AutomationLinksResponse::get_links,
            &blog::dto::gift_links::AutomationLinksResponse::set_links,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "links";
    }
};

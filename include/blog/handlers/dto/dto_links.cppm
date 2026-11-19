export module blog_handlers:dto_links;

import std;
import core_serde;

export namespace blog::dto {

class LinkRecord
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_url(std::string value) { url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return url; }
    std::string id{"1"};
    std::string url{"https://ghost.local/link"};
};

class GiftLinksResponse
{
public:
    void set_gift_links(std::vector<LinkRecord> value) { gift_links = std::move(value); }
    [[nodiscard]] const std::vector<LinkRecord>& get_gift_links() const noexcept { return gift_links; }
    std::vector<LinkRecord> gift_links{};
};

class AutomationLinksResponse
{
public:
    void set_links(std::vector<LinkRecord> value) { links = std::move(value); }
    [[nodiscard]] const std::vector<LinkRecord>& get_links() const noexcept { return links; }
    std::vector<LinkRecord> links{};
};

} // namespace blog::dto

template<> struct serde::Serializable<blog::dto::LinkRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::LinkRecord::get_id, &blog::dto::LinkRecord::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::LinkRecord::get_url, &blog::dto::LinkRecord::set_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "link_record"; }
};


template<> struct serde::Serializable<blog::dto::GiftLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"gift_links", &blog::dto::GiftLinksResponse::get_gift_links,
                                           &blog::dto::GiftLinksResponse::set_gift_links,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "gift_links"; }
};
template<> struct serde::Serializable<blog::dto::AutomationLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"links", &blog::dto::AutomationLinksResponse::get_links,
                                           &blog::dto::AutomationLinksResponse::set_links,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "links"; }
};

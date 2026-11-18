export module blog_dto:links;

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

#define LINK_SERDE(Type, JsonName, Field)                                                                  \
    template<> struct serde::Serializable<blog::dto::Type>                                     \
    {                                                                                          \
        static constexpr auto fields()                                                         \
        { return std::tuple{serde::FieldDesc<JsonName, &blog::dto::Type::get_##Field,                   \
                                             &blog::dto::Type::set_##Field,                      \
                                             serde::FieldOptions{}>{}}; }                      \
        static constexpr std::string_view table_name() { return JsonName; }                         \
    };

LINK_SERDE(GiftLinksResponse, "gift_links", gift_links)
LINK_SERDE(AutomationLinksResponse, "links", links)

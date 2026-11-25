module;
export module app_http:header;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::http {

/** @brief Instance reader turning a request header value into an owned string. */
class HeaderReader
{
public:
    [[nodiscard]] std::string get(interfaces::io::IRequest& request, std::string_view name) const
    {
        return std::string{request.find_header(name)};
    }
};

/** @brief Instance functor reading the Hyperswitch merchant id header. */
class merchant_id_from_header
{
public:
    [[nodiscard]] std::string operator()(interfaces::io::IRequest& request) const
    {
        return HeaderReader{}.get(request, "x-merchant-id");
    }
};

/** @brief Instance functor reading the Hyperswitch organization id header. */
class org_id_from_header
{
public:
    [[nodiscard]] std::string operator()(interfaces::io::IRequest& request) const
    {
        return HeaderReader{}.get(request, "x-org-id");
    }
};

} // namespace app::http

#ifdef CONGELADO_TEST
namespace app::http::tests {
using namespace boost::ut;

suite<"app_http_header"> header_suite = []
{
    struct FakeRequest : interfaces::io::IRequest
    {
        explicit FakeRequest(std::string merchant, std::string org) :
            interfaces::io::IRequest{0},
            m_merchant{std::move(merchant)},
            m_org{std::move(org)}
        {
        }

        [[nodiscard]] std::string_view find_header(
            std::variant<std::string_view, interfaces::io::types::Token> name_or_token
        ) const noexcept override
        {
            const auto* name{std::get_if<std::string_view>(&name_or_token)};
            if (name == nullptr) {
                return {};
            }
            if (*name == "x-merchant-id") {
                return m_merchant;
            }
            if (*name == "x-org-id") {
                return m_org;
            }
            return {};
        }

        std::string m_merchant{};
        std::string m_org{};
    };

    "reader returns owned header values, empty when absent"_test = []
    {
        FakeRequest request{"merchant_123", "org_456"};
        const HeaderReader read;

        expect(read.get(request, "x-merchant-id") == "merchant_123");
        expect(read.get(request, "x-org-id") == "org_456");
        expect(read.get(request, "x-unknown").empty());
    };

    "hyperswitch helpers read their specific headers"_test = []
    {
        FakeRequest request{"merchant_123", "org_456"};

        expect(merchant_id_from_header{}(request) == "merchant_123");
        expect(org_id_from_header{}(request) == "org_456");
    };
};

} // namespace app::http::tests
#endif

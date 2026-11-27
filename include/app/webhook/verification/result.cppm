export module app_webhook:verification_result;

import std;
import interfaces;
import core_behavior;
import core_serde;
import core_error;
import app_dto;

export namespace app::handlers::verification {

// Wire field lists captured from the retired Result serializers, in declaration
// order. Serializer<T>::serialize(fields) emits the same upstream bytes.
[[nodiscard]] inline constexpr auto apple_pay_merchant_registration_wire_fields()
{
    return std::array<std::string_view, 1>{"status_message"};
}

[[nodiscard]] inline constexpr auto retrieve_apple_pay_verified_domains_wire_fields()
{
    return std::array<std::string_view, 1>{"verified_domains"};
}

} // namespace app::handlers::verification

namespace {

void publish_verification_value(
    interfaces::io::ResponseSink& sink,
    const std::expected<std::string, core::error::AppError>& encoded
) {
    if (!encoded) {
        encoded.error().send(sink);
        return;
    }
    sink.set_status(interfaces::io::types::Status::OK);
    sink.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
    sink.send();
}

} // namespace

export template<>
class core::behavior::Outcome<::app::dto::verifications::ApplepayMerchantResponse> :
    public std::expected<::app::dto::verifications::ApplepayMerchantResponse, core::error::AppError>
{
public:
    using std::expected<
        ::app::dto::verifications::ApplepayMerchantResponse,
        core::error::AppError>::expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        publish_verification_value(sink, value().serialize(
            app::handlers::verification::apple_pay_merchant_registration_wire_fields()
        ));
    }
};

export template<>
class core::behavior::Outcome<::app::dto::verifications::ApplepayVerifiedDomainsResponse> :
    public std::expected<
        ::app::dto::verifications::ApplepayVerifiedDomainsResponse,
        core::error::AppError>
{
public:
    using std::expected<
        ::app::dto::verifications::ApplepayVerifiedDomainsResponse,
        core::error::AppError>::expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        publish_verification_value(sink, value().serialize(
            app::handlers::verification::retrieve_apple_pay_verified_domains_wire_fields()
        ));
    }
};

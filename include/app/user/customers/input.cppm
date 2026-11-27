export module app_user:customers_input;

import std;
import interfaces;
import core_behavior;
import core_serde;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_model;

export namespace app::handlers::customers {

// Local equivalents of the upstream Cargo features that select the customer
// surface (routes/app.rs:1536-1636): v1 `/customers` and v2 `/{prefix}/customers`.
inline constexpr bool kV1 = true;
inline constexpr bool kV2 = true;

inline constexpr std::string_view kRedacted = "Redacted";
inline constexpr std::size_t kDefaultListApiLimit = 10;
inline constexpr std::size_t kDefaultCustomerListLimit = 10;
inline constexpr std::size_t kMigrationMaxFileSize = 1'024U * 1'024U;
inline constexpr std::size_t kMigrationMaxRecords = 500;
inline constexpr std::size_t kMigrationParallelism = 10;

enum class ApiVersion : std::uint8_t
{
    V1,
    V2
};

struct CustomerAddressDto
{
    std::optional<std::string> city{};
    std::optional<std::string> country{};
    std::optional<std::string> line1{};
    std::optional<std::string> line2{};
    std::optional<std::string> line3{};
    std::optional<std::string> zip{};
    std::optional<std::string> state{};
    std::optional<std::string> first_name{};
    std::optional<std::string> last_name{};
    std::optional<std::string> origin_zip{};
};

struct CustomerDocumentDto
{
    std::optional<std::string> document_type{};
    std::optional<std::string> document_number{};
};

struct CustomerCreateDto
{
    std::optional<std::string> customer_id{};
    std::optional<std::string> merchant_reference_id{};
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<CustomerAddressDto> address{};
    std::optional<CustomerAddressDto> billing_address{};
    std::optional<CustomerAddressDto> shipping_address{};
    std::optional<CustomerDocumentDto> document_details{};
    std::optional<std::string> metadata_raw{};
    std::optional<std::string> tax_registration_id{};
    std::optional<std::string> default_payment_method_id{};
};

struct CustomerUpdateDto
{
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<CustomerAddressDto> address{};
    std::optional<CustomerAddressDto> billing_address{};
    std::optional<CustomerAddressDto> shipping_address{};
    std::optional<CustomerDocumentDto> document_details{};
    std::optional<std::string> metadata_raw{};
    std::optional<std::string> tax_registration_id{};
    std::optional<std::string> default_payment_method_id{};
};

/** @brief Read-only navigation of a parsed JSON value (serde::Value = rfl::Generic). */
class JsonValueView
{
public:
    JsonValueView() = default;

    explicit JsonValueView(serde::Value value) :
        m_value{std::move(value)}
    {
    }

    [[nodiscard]] std::optional<std::string> string(std::string_view key) const
    {
        const auto field = find(key);
        if (!field) {
            return std::nullopt;
        }
        const auto* value = std::get_if<std::string>(&field->get());
        if (!value) {
            return std::nullopt;
        }
        return *value;
    }

    [[nodiscard]] std::optional<std::string> raw(std::string_view key) const
    {
        const auto field = find(key);
        if (!field) {
            return std::nullopt;
        }
        return serde::Ser::encode_json(*field);
    }

    [[nodiscard]] std::optional<JsonValueView> object(std::string_view key) const
    {
        const auto field = find(key);
        if (!field) {
            return std::nullopt;
        }
        if (!std::holds_alternative<serde::Value::Object>(field->get())) {
            return std::nullopt;
        }
        return JsonValueView{*field};
    }

private:
    [[nodiscard]] std::optional<serde::Value> find(std::string_view key) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&m_value.get());
        if (!object) {
            return std::nullopt;
        }
        auto value = object->get(std::string{key});
        if (!value) {
            return std::nullopt;
        }
        return std::optional<serde::Value>{*value};
    }

    serde::Value m_value{};
};

/** @brief Mirrors the upstream `MigrationRow<State>` typestate machine (migrate.rs:111-306). */
struct MigrationRow
{
    enum class State : std::uint8_t
    {
        Parsed,
        Fetched,
        V1Verified,
        NeedsUpdate,
        Generated
    };
    std::size_t row_number{};
    std::string merchant_id{};
    std::string customer_id{};
    std::optional<std::string> old_id{};
    std::optional<std::string> version{};
    std::optional<std::string> new_id{};
    std::optional<std::string> update_status{};
    State state{State::Parsed};
};

using MigrationOutcome =
    std::variant<MigrationRow, ::app::model::customers::CustomerGlobalIdMigrationRowResult>;

/** @brief Request snapshot shared by every customers runtime method. */
[[nodiscard]] inline ::core::behavior::CallContext make_call_context(interfaces::io::IRequest& req)
{
    ::core::behavior::CallContext ctx{};
    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ctx.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
    if (ctx.merchant_id.empty()) ctx.merchant_id = auth.merchant_id();
    ctx.org_id = std::string{::app::http::org_id_from_header{}(req)};
    if (ctx.org_id.empty()) ctx.org_id = auth.org_id();
    ctx.profile_id = auth.profile_id();
    if (const auto c = auth.claim(::app::auth::kClaimConnectedMerchantId)) ctx.connected_merchant_id = std::string{*c};
    if (const auto s = auth.claim(::app::auth::kClaimClientSecret)) ctx.client_secret = std::string{*s};
    const auto segments = ::app::http::RequestPath{}(req);
    for (std::size_t i{0}; i < segments.count(); ++i) ctx.path_segments.push_back(std::string{segments.at(i).value_or("")});
    const auto raw = req.get_path();
    if (const auto q = raw.find('?'); q != std::string_view::npos) {
        const std::string_view tail = raw.substr(q + 1);
        std::size_t start{0};
        while (start <= tail.size()) {
            const auto amp = tail.find('&', start);
            const auto end = amp == std::string_view::npos ? tail.size() : amp;
            const auto pair = tail.substr(start, end - start);
            const auto eq = pair.find('=');
            if (!pair.empty()) {
                ctx.query.insert_or_assign(
                    std::string{eq == std::string_view::npos ? pair : pair.substr(0, eq)},
                    std::string{eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)}
                );
            }
            if (amp == std::string_view::npos) break;
            start = amp + 1;
        }
    }
    constexpr std::string_view kHeaders[] = {
        "x-merchant-id", "x-org-id", "x-organization-id", "x-profile-id", "authorization",
        "x-client-secret", "x-api-key", "x-connected-merchant-id", "x-forwarded-for"
    };
    for (const auto name: kHeaders) {
        const auto value = req.find_header(name);
        if (!value.empty()) ctx.headers.insert_or_assign(std::string{name}, std::string{value});
    }
    for (const auto& [key, value]: auth.claims()) ctx.claims.insert_or_assign(key, value);
    ctx.claims.insert_or_assign("merchant_id", ctx.merchant_id);
    ctx.claims.insert_or_assign("org_id", ctx.org_id);
    ctx.claims.insert_or_assign("profile_id", ctx.profile_id);
    return ctx;
}

[[nodiscard]] inline std::string ctx_string(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto it = ctx.claims.find(std::string{key});
    return it == ctx.claims.end() ? std::string{} : it->second;
}

[[nodiscard]] inline ApiVersion ctx_version(const ::core::behavior::CallContext& ctx)
{
    return ctx_string(ctx, "version") == "v2" ? ApiVersion::V2 : ApiVersion::V1;
}

[[nodiscard]] inline std::optional<std::string>
ctx_optional_string(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto raw = ctx_string(ctx, key);
    return raw.empty() ? std::nullopt : std::optional<std::string>{raw};
}

template<class T>
[[nodiscard]] std::optional<T> ctx_optional_number(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto raw = ctx_string(ctx, key);
    if (raw.empty()) return std::nullopt;
    T value{};
    const auto result = std::from_chars(raw.data(), raw.data() + raw.size(), value);
    return result.ec == std::errc{} && result.ptr == raw.data() + raw.size()
               ? std::optional<T>{value}
               : std::nullopt;
}

[[nodiscard]] inline std::size_t ctx_size(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    return ctx_optional_number<std::size_t>(ctx, key).value_or(0U);
}

[[nodiscard]] inline std::optional<::app::model::customers::CustomerTimeRange>
ctx_time_range(const ::core::behavior::CallContext& ctx)
{
    const auto start = ctx_optional_string(ctx, "start_time");
    const auto end = ctx_optional_string(ctx, "end_time");
    if (!start && !end) return std::nullopt;
    ::app::model::customers::CustomerTimeRange range{};
    if (start) range.set_start_time(*start);
    if (end) range.set_end_time(*end);
    return range;
}

template<class T>
[[nodiscard]] std::string encode_optional(const std::optional<T>& value)
{
    return value ? std::to_string(*value) : std::string{};
}

[[nodiscard]] inline std::string
encode_time_range(const std::optional<::app::model::customers::CustomerTimeRange>& range)
{
    return range && range->get_start_time().has_value() ? *range->get_start_time() : std::string{};
}

[[nodiscard]] inline std::string
encode_time_range_end(const std::optional<::app::model::customers::CustomerTimeRange>& range)
{
    return range && range->get_end_time().has_value() ? *range->get_end_time() : std::string{};
}

[[nodiscard]] inline std::string encode_optional_string(const std::optional<std::string>& value)
{
    return value.value_or("");
}

/**
 * @brief Parse/auth/path/query support for the customers handlers.
 *
 * Free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. All helpers return errors instead
 * of writing them, so the handler keeps one response-writing path.
 */
class CustomersInputSupport
{
public:
    explicit CustomersInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] ::core::error::AppError
    error(::core::error::AppErrorKind kind, std::string message) const
    {
        return ::core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] ::core::error::AppError
    upstream_error(std::string_view wire_name, std::string message) const
    {
        using K = ::core::error::AppErrorKind;
        const K kind = wire_name == "CustomerAlreadyExists" ? K::ALREADY_EXISTS
                       : wire_name == "CustomerNotFound"    ? K::NOT_FOUND
                       : wire_name == "MandateActive"       ? K::PRECONDITION_FAILED
                       : wire_name == "AccessForbidden"     ? K::FORBIDDEN
                       : wire_name == "InternalServerError" ? K::INTERNAL_ERROR
                                                            : K::VALIDATION_FAILED;
        // Upstream code lives in `kind_name` (serialized as the `error` field);
        // the real human-readable text lives in `message`.
        std::string code{wire_name};
        std::string canonical{message};
        if (const auto wire = upstream_wire(wire_name, message)) {
            code = std::string{std::get<0>(*wire)};
            canonical = std::get<2>(*wire);
        }
        auto result = error(kind, std::move(canonical));
        result.set_kind_name(std::move(code));
        return result;
    }

    /** @brief Upstream `ErrorResponse` wire name → (code, error_type, canonical message). */
    [[nodiscard]] std::optional<std::tuple<std::string_view, std::string_view, std::string>>
    upstream_wire(std::string_view name, const std::string& message) const
    {
        if (name == "InvalidRequestData") {
            return std::tuple<std::string_view, std::string_view, std::string>{
                std::string_view{"IR_06"},
                std::string_view{"invalid_request"},
                message
            };
        }
        if (name == "CustomerAlreadyExists") {
            return std::tuple<std::string_view, std::string_view, std::string>{
                std::string_view{"IR_12"},
                std::string_view{"invalid_request"},
                std::string{"Customer with the given `customer_id` already exists"}
            };
        }
        if (name == "CustomerNotFound") {
            return std::tuple<std::string_view, std::string_view, std::string>{
                std::string_view{"HE_02"},
                std::string_view{"invalid_request"},
                std::string{"Customer does not exist in our records"}
            };
        }
        if (name == "MandateActive") {
            return std::tuple<std::string_view, std::string_view, std::string>{
                std::string_view{"IR_10"},
                std::string_view{"invalid_request"},
                std::string{"Customer has active mandate/subsciption"}
            };
        }
        if (name == "AccessForbidden") {
            return std::tuple<std::string_view, std::string_view, std::string>{
                std::string_view{"IR_22"},
                std::string_view{"invalid_request"},
                "Access forbidden. Not authorized to access this resource " + message
            };
        }
        if (name == "InternalServerError") {
            return std::tuple<std::string_view, std::string_view, std::string>{
                std::string_view{"HE_00"},
                std::string_view{"api"},
                std::string{"Something went wrong"}
            };
        }
        return std::nullopt;
    }

    [[nodiscard]] bool is_jwt_auth(interfaces::io::IRequest& req) const
    {
        const std::string_view header = req.find_header("authorization");
        constexpr std::string_view prefix{"Bearer "};
        return header.size() > prefix.size() &&
               std::equal(
                   prefix.begin(),
                   prefix.end(),
                   header.begin(),
                   [](char lhs, char rhs)
                   {
                       return std::tolower(static_cast<unsigned char>(lhs)) ==
                              std::tolower(static_cast<unsigned char>(rhs));
                   }
               );
    }

    [[nodiscard]] bool has_header(interfaces::io::IRequest& req, std::string_view name) const
    {
        return !req.find_header(name).empty();
    }

    /** @brief Upstream auth extractor selection (routes/customers.rs:39-50,82-93, et al). */
    [[nodiscard]] std::optional<::core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        ::app::auth::AppPermission permission,
        bool jwt,
        bool v2_api_key,
        bool sdk,
        bool client,
        bool header_api_key,
        bool allow_connected,
        bool allow_platform
    ) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        if (jwt && is_jwt_auth(req)) {
            return m_context.authorizer().authorize_route(
                context,
                ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::JWT,
                    permission,
                    allow_connected,
                    allow_platform
                }
            );
        }
        if (sdk && (has_header(req, "x-sdk-key") || has_header(req, "x-publishable-key"))) {
            return m_context.authorizer().authorize_route(
                context,
                ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::SDK,
                    permission,
                    allow_connected,
                    allow_platform
                }
            );
        }
        if (client && (has_header(req, "x-client-secret") || has_header(req, "client-secret"))) {
            return m_context.authorizer().authorize_route(
                context,
                ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::CLIENT_SECRET,
                    permission,
                    allow_connected,
                    allow_platform
                }
            );
        }
        if (v2_api_key && has_header(req, "x-api-key")) {
            return m_context.authorizer().authorize_route(
                context,
                ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::V2_API_KEY,
                    permission,
                    allow_connected,
                    allow_platform
                }
            );
        }
        if (header_api_key) {
            return m_context.authorizer().authorize_route(
                context,
                ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::API_KEY,
                    permission,
                    allow_connected,
                    allow_platform
                }
            );
        }
        const auto scheme =
            v2_api_key ? ::app::auth::AuthScheme::V2_API_KEY : ::app::auth::AuthScheme::API_KEY;
        return m_context.authorizer().authorize_route(
            context,
            ::app::auth::RouteAuthRule{scheme, permission, allow_connected, allow_platform}
        );
    }

    /** @brief core::utils::validate_legacy_endpoint_access (core/utils.rs:167-193). */
    [[nodiscard]] std::optional<::core::error::AppError>
    validate_legacy_endpoint_access(interfaces::io::IRequest& req) const
    {
        const bool dashboard = is_jwt_auth(req);
        if (dashboard) {
            return std::nullopt;
        }
        const auto enabled = m_context.config_store().get_value(
            "feature_config",
            "is_payment_method_modular_allowed"
        );
        if (enabled && (*enabled == "true" || *enabled == "1")) {
            return upstream_error("AccessForbidden", "Deprecated route");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string request_merchant_id(interfaces::io::IRequest& req) const
    {
        return ::app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::optional<std::uint32_t> parse_uint32(std::string_view text) const
    {
        std::uint32_t out{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
            return std::nullopt;
        }
        return out;
    }

    [[nodiscard]] std::optional<std::uint16_t> parse_uint16(std::string_view text) const
    {
        std::uint16_t out{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
            return std::nullopt;
        }
        return out;
    }

    [[nodiscard]] std::optional<std::size_t> parse_size(std::string_view text) const
    {
        std::size_t out{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
            return std::nullopt;
        }
        return out;
    }

    [[nodiscard]] CustomerCreateDto
    parse_create_input(const JsonValueView& view, ApiVersion version) const
    {
        CustomerCreateDto input{};
        input.customer_id =
            view.string(version == ApiVersion::V1 ? "customer_id" : "merchant_reference_id");
        input.merchant_reference_id = view.string("merchant_reference_id");
        input.name = view.string("name");
        input.email = view.string("email");
        input.phone = view.string("phone");
        input.description = view.string("description");
        input.phone_country_code = view.string("phone_country_code");
        input.tax_registration_id = view.string("tax_registration_id");
        input.default_payment_method_id = view.string("default_payment_method_id");
        input.metadata_raw = view.raw("metadata");
        if (const auto address = view.object("address")) {
            input.address = parse_address(*address);
        }
        if (const auto address = view.object("default_billing_address")) {
            input.billing_address = parse_address(*address);
        }
        if (const auto address = view.object("default_shipping_address")) {
            input.shipping_address = parse_address(*address);
        }
        if (const auto document = view.object("document_details")) {
            CustomerDocumentDto parsed{};
            parsed.document_type = document->string("document_type");
            parsed.document_number = document->string("document_number");
            input.document_details = std::move(parsed);
        }
        return input;
    }

    [[nodiscard]] CustomerUpdateDto parse_update_input(const JsonValueView& view) const
    {
        CustomerUpdateDto input{};
        input.name = view.string("name");
        input.email = view.string("email");
        input.phone = view.string("phone");
        input.description = view.string("description");
        input.phone_country_code = view.string("phone_country_code");
        input.tax_registration_id = view.string("tax_registration_id");
        input.default_payment_method_id = view.string("default_payment_method_id");
        input.metadata_raw = view.raw("metadata");
        if (const auto address = view.object("address")) {
            input.address = parse_address(*address);
        }
        if (const auto address = view.object("default_billing_address")) {
            input.billing_address = parse_address(*address);
        }
        if (const auto address = view.object("default_shipping_address")) {
            input.shipping_address = parse_address(*address);
        }
        if (const auto document = view.object("document_details")) {
            CustomerDocumentDto parsed{};
            parsed.document_type = document->string("document_type");
            parsed.document_number = document->string("document_number");
            input.document_details = std::move(parsed);
        }
        return input;
    }

    [[nodiscard]] CustomerAddressDto parse_address(const JsonValueView& view) const
    {
        CustomerAddressDto address{};
        address.city = view.string("city");
        address.country = view.string("country");
        address.line1 = view.string("line1");
        address.line2 = view.string("line2");
        address.line3 = view.string("line3");
        address.zip = view.string("zip");
        address.state = view.string("state");
        address.first_name = view.string("first_name");
        address.last_name = view.string("last_name");
        address.origin_zip = view.string("origin_zip");
        return address;
    }

    // DocumentKind::validate (common_types/customers.rs:65-122).
    [[nodiscard]] std::optional<::core::error::AppError>
    validate_document_details(const std::optional<CustomerDocumentDto>& document) const
    {
        if (!document) {
            return std::nullopt;
        }
        const std::string& type = document->document_type.value_or("");
        const std::string& number = document->document_number.value_or("");
        if (type.empty() || number.empty() || number.size() > 255U) {
            return upstream_error("InvalidRequestData", "invalid document_details");
        }
        bool valid{true};
        if (type == "psn") {
            valid = number.size() == 12U && std::all_of(
                                                number.begin(),
                                                number.end(),
                                                [](char c)
                                                {
                                                    return c >= '0' && c <= '9';
                                                }
                                            );
        } else if (type == "cpf") {
            valid = valid_cpf(number);
        } else if (type == "cnpj") {
            valid = valid_cnpj(number);
        }
        if (!valid) {
            return upstream_error("InvalidRequestData", "invalid document_details");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool valid_cpf(std::string_view value) const noexcept
    {
        if (value.size() != 11U) {
            return false;
        }
        std::array<int, 11> digits{};
        for (std::size_t i{0}; i < 11U; ++i) {
            if (value[i] < '0' || value[i] > '9') {
                return false;
            }
            digits[i] = value[i] - '0';
        }
        const auto check = [&digits](std::size_t length)
        {
            int sum{0};
            for (std::size_t i{0}; i < length; ++i) {
                sum += digits[i] * static_cast<int>(length + 1 - i);
            }
            int result = (sum * 10) % 11;
            return result == 10 ? 0 : result;
        };
        return check(9) == digits[9] && check(10) == digits[10];
    }

    [[nodiscard]] bool valid_cnpj(std::string_view value) const noexcept
    {
        if (value.size() != 14U) {
            return false;
        }
        std::array<int, 14> digits{};
        for (std::size_t i{0}; i < 14U; ++i) {
            if (value[i] < '0' || value[i] > '9') {
                return false;
            }
            digits[i] = value[i] - '0';
        }
        const auto check = [&digits](std::size_t length)
        {
            int sum{0};
            std::size_t weight{length - 7};
            for (std::size_t i{0}; i < length; ++i) {
                sum += digits[i] * static_cast<int>(weight);
                weight = weight == 2 ? 9 : weight - 1;
            }
            const int result = sum % 11;
            return result < 2 ? 0 : 11 - result;
        };
        return check(12) == digits[12] && check(13) == digits[13];
    }

    [[nodiscard]] std::optional<::core::error::AppError>
    validate_phone_country_code(const std::optional<std::string>& code) const
    {
        if (!code) {
            return std::nullopt;
        }
        constexpr std::size_t max_length{8U};
        const bool valid = !code->empty() && code->size() <= max_length &&
                           std::all_of(
                               code->begin() + (code->front() == '+' ? 1 : 0),
                               code->end(),
                               [](char c)
                               {
                                   return c >= '0' && c <= '9';
                               }
                           );
        if (!valid) {
            return upstream_error(
                "InvalidRequestData",
                std::format(
                    "phone_country_code must be a valid country calling code, got \"{}\"",
                    *code
                )
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin{0};
        std::size_t end{value.size()};
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) {
            ++begin;
        }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
            --end;
        }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult
    invalid_csv_row_result(
        std::size_t row_number,
        std::string merchant_id,
        std::string customer_id,
        std::string message
    ) const
    {
        ::app::model::customers::CustomerGlobalIdMigrationRowResult result{};
        result.set_row_number(row_number);
        result.set_merchant_id(
            merchant_id.empty() ? std::nullopt : std::optional<std::string>{merchant_id}
        );
        result.set_customer_id(
            customer_id.empty() ? std::nullopt : std::optional<std::string>{customer_id}
        );
        result.set_status("InvalidCsvRow");
        result.set_message(std::move(message));
        return result;
    }

    [[nodiscard]] core::behavior::Outcome<std::vector<MigrationOutcome>>
    parse_customer_global_id_migration_csv(std::string_view body) const
    {
        if (body.size() > kMigrationMaxFileSize) {
            return std::unexpected{upstream_error(
                "InvalidRequestData",
                std::format("CSV file exceeds maximum size of {} bytes", kMigrationMaxFileSize)
            )};
        }
        std::vector<std::string> lines{};
        std::string current{};
        const auto finish = [&]()
        {
            if (!current.empty()) {
                lines.push_back(current);
            }
            current.clear();
        };
        for (const char ch: body) {
            if (ch == '\n') {
                finish();
            } else if (ch != '\r') {
                current.push_back(ch);
            }
        }
        finish();
        if (lines.empty() || trim(lines.front()) != "merchant_id,customer_id") {
            return std::unexpected{upstream_error(
                "InvalidRequestData",
                "CSV must include header: merchant_id,customer_id"
            )};
        }
        std::vector<MigrationOutcome> rows{};
        for (std::size_t i{1}; i < lines.size(); ++i) {
            if (i - 1 >= kMigrationMaxRecords) {
                return std::unexpected{upstream_error(
                    "InvalidRequestData",
                    std::format("CSV file exceeds maximum record count of {}", kMigrationMaxRecords)
                )};
            }
            const std::size_t comma = lines[i].find(',');
            if (comma == std::string::npos) {
                rows.push_back(invalid_csv_row_result(
                    i + 1,
                    "",
                    "",
                    "CSV row must contain valid merchant_id and customer_id values"
                ));
                continue;
            }
            MigrationRow row{};
            row.row_number = i + 1;
            row.merchant_id = trim(lines[i].substr(0, comma));
            row.customer_id = trim(lines[i].substr(comma + 1));
            if (row.merchant_id.empty() || row.customer_id.empty() ||
                lines[i].find(',', comma + 1) != std::string::npos) {
                rows.push_back(invalid_csv_row_result(
                    row.row_number,
                    row.merchant_id,
                    row.customer_id,
                    "CSV row must contain valid merchant_id and customer_id values"
                ));
                continue;
            }
            rows.push_back(std::move(row));
        }
        return rows;
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain authorization policy for customers.
 *
 * Owns the extractor selection for the v1/v2 customer surfaces. The permission
 * and scope flags come from the caller's AuthSpec; the operation string picks
 * which extractors (JWT / SDK / client / API key) are consulted, exactly as
 * the original route declarations did.
 */
class CustomersPolicy
{
public:
    explicit CustomersPolicy(CustomersInputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec& spec
    ) const
    {
        bool jwt_detect{true};
        bool v2_api_key{false};
        bool sdk{false};
        bool client{false};
        bool header_api_key{false};
        if (operation == "customers_create" || operation == "customers_retrieve" ||
            operation == "customers_update" || operation == "customers_delete" ||
            operation == "get_customer_mandates") {
            header_api_key = true;
        } else if (
            operation == "customers_create_v2" || operation == "customers_retrieve_v2" ||
            operation == "customers_retrieve_by_merchant_reference_id" ||
            operation == "customers_list_v2" || operation == "customers_list_with_count_v2" ||
            operation == "customers_update_v2" || operation == "customers_delete_v2"
        ) {
            v2_api_key = true;
            if (operation == "customers_retrieve_v2") {
                sdk = true;
                client = true;
            }
            if (operation == "customers_retrieve_by_merchant_reference_id") {
                jwt_detect = false;
            }
        }
        const bool jwt = jwt_detect && m_input.is_jwt_auth(req);
        return m_input.authorize(
            req,
            permission(spec.permission),
            jwt,
            v2_api_key,
            sdk,
            client,
            header_api_key,
            spec.allow_connected,
            spec.allow_platform
        );
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ::core::behavior::Scope out{};
        out.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        out.org_id = std::string{::app::http::org_id_from_header{}(req)};
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value()) {
            out.connected_merchant_id = std::string{*connected};
        }
        return out;
    }

    [[nodiscard]] ::core::behavior::CallContext
    context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }

    /** @brief From-end segment index, matching the legacy path_segment helper. */
    [[nodiscard]] static std::size_t path_index(interfaces::io::IRequest& req, std::size_t from_end)
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        if (from_end >= segments.count()) {
            return std::numeric_limits<std::size_t>::max();
        }
        return segments.count() - 1 - from_end;
    }

private:
    [[nodiscard]] static ::app::auth::AppPermission permission(std::string_view name) noexcept
    {
        if (name == "USER_WRITE") {
            return ::app::auth::AppPermission::USER_WRITE;
        }
        if (name == "MANDATE_READ") {
            return ::app::auth::AppPermission::MANDATE_READ;
        }
        return ::app::auth::AppPermission::USER_READ;
    }

    CustomersInputSupport& m_input;
};

using CustomersRequestSupport = ::core::behavior::RequestSupport<CustomersPolicy>;

} // namespace app::handlers::customers

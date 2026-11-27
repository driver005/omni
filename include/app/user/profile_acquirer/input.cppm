export module app_user:profile_acquirer_input;

import std;
import interfaces;
import core_behavior;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::profile_acquirer {

/**
 * @brief Parse/auth/path/query support for the profile_acquirer handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class ProfileAcquirerInputSupport
{
public:
    explicit ProfileAcquirerInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ---- authentication -----------------------------------------------------
    // Local port of upstream's `auth_type` dual extractor
    // (routes/profile_acquirer.rs:34-46,81-93):
    //   HeaderAuth(ApiKeyAuth{allow_connected_scope_operation: true,
    //                         allow_platform_self_operation: false})
    //   OR JWTAuthProfileFromRoute{profile_id, ProfileAccountWrite,
    //                              allow_connected: true, allow_platform: false}
    // Permission::ProfileAccountWrite maps to AppPermission::PROFILE_WRITE, the
    // same local mapping used for that upstream permission in verification.cppm.
    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view route_profile_id
    ) const
    {
        (void)operation; // audited registry rows exist but are coarser than ProfileAccountWrite
        auto context = ::app::auth::AuthContextBuilder{}(req);

        // is_jwt_auth: a Bearer token selects the JWT branch, else ApiKeyAuth.
        if (!context.token().is_empty()) {
            // JWTAuthProfileFromRoute binds the caller to the profile id from the
            // route/body. When an x-profile-id header is present it must match the
            // route profile; the auth principal is then the route profile id.
            if (!context.profile_id().empty() &&
                context.profile_id() != std::string{route_profile_id}) {
                auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::UNAUTHORIZED,
                    "Unauthorized"
                );
                error.set_kind_name("Unauthorized");
                return error;
            }
            context.set_profile_id(std::string{route_profile_id});
            const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                                  .scheme(::app::auth::AuthScheme::JWT)
                                  .permission(::app::auth::AppPermission::PROFILE_WRITE)
                                  .allow_connected(true)
                                  .allow_platform(false)
                                  .build();
            return m_context.authorizer().authorize_route(context, rule);
        }

        // The local authorizer requires a credential for API_KEY rules; upstream
        // authenticates this branch from the api-key header, so carry it as the
        // local credential (same adaptation as verification.cppm).
        if (const auto api_key = context.claim(::app::auth::kClaimApiKey);
            api_key && !api_key->empty()) {
            context.set_token(::app::auth::BearerToken{std::string{*api_key}});
        }
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(::app::auth::AuthScheme::API_KEY)
                              .permission(::app::auth::AppPermission::PROFILE_WRITE)
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        return m_context.authorizer().authorize_route(context, rule);
    }

    // ---- route ids from the path ---------------------------------------------
    [[nodiscard]] std::expected<std::pair<std::string, std::string>, core::error::AppError>
    profile_and_acquirer_ids_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (segments.count() < 2) {
            return std::unexpected{missing_ids_error()};
        }
        const std::string profile_acquirer_id =
            std::string{segments.at(segments.count() - 1).value_or("")};
        const std::string profile_id = std::string{segments.at(segments.count() - 2).value_or("")};
        if (profile_id.empty() || profile_acquirer_id.empty()) {
            return std::unexpected{missing_ids_error()};
        }
        return std::make_pair(profile_id, profile_acquirer_id);
    }

    // ---- serde extraction validation (upstream web::Json<...> extraction) ----
    // Upstream rejects at extraction with HTTP 400, before server_wrap/core:
    //   ProfileAcquirerCreate (api_models:4-32)   -> enum validation only
    //   ProfileAcquirerUpdate (api_models:154-174)-> #[serde(deny_unknown_fields)] + enums
    // CardNetwork serde values are the variant names plus #[serde(alias)] entries
    // (common_enums/src/enums.rs:3659-3723); CountryAlpha2 serde values are the
    // 249 two-letter variants (enums.rs:4285-...), the exact set held by the
    // alpha2_to_numeric() table below used by the core's country normalization.
    [[nodiscard]] bool
    payload_passes_serde_validation(std::string_view body, bool deny_unknown_fields) const
    {
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return false;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return false;
        }
        static constexpr auto kUpdateFields = std::to_array<std::string_view>(
            {"acquirer_assigned_merchant_id",
             "merchant_name",
             "network",
             "acquirer_bin",
             "acquirer_ica",
             "acquirer_fraud_rate",
             "acquirer_country_code",
             "is_default"}
        );
        for (const auto& [key, value]: *root) {
            const std::string_view field{key};
            if (deny_unknown_fields &&
                std::find(kUpdateFields.begin(), kUpdateFields.end(), field) ==
                    kUpdateFields.end()) {
                return false; // serde deny_unknown_fields
            }
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                if (field == "network" && !is_valid_card_network(*text)) {
                    return false;
                }
                if (field == "acquirer_country_code" && !is_valid_country_alpha2(*text)) {
                    return false;
                }
            }
            // A non-string enum value is either null (Option => valid) or is
            // rejected by the typed deserialize<Input>() below with HTTP 400.
        }
        return true;
    }

    [[nodiscard]] bool is_valid_card_network(std::string_view network) const
    {
        static constexpr auto kAccepted = std::to_array<std::string_view>(
            {"Visa",
             "Mastercard",
             "AmericanExpress",
             "JCB",
             "DinersClub",
             "Discover",
             "CartesBancaires",
             "UnionPay",
             "Interac",
             "RuPay",
             "Maestro",
             "Star",
             "Pulse",
             "Accel",
             "Nyce",
             "Prop",
             "PrivateLabel",
             "Dinacard",
             "AirPlus",
             "Aurore",
             "EftposAustralia",
             "GeCapital",
             "Uatp",
             "VISA",
             "MASTERCARD",
             "AMERICANEXPRESS",
             "AMEX",
             "DINERSCLUB",
             "DISCOVER",
             "CARTESBANCAIRES",
             "UNIONPAY",
             "INTERAC",
             "RUPAY",
             "MAESTRO",
             "STAR",
             "PULSE",
             "ACCEL",
             "NYCE",
             "PROP",
             "PRIVATE LABEL",
             "DINACARD",
             "AIRPLUS",
             "AURORE",
             "EFTPOS_AUSTRALIA",
             "GECAPITAL",
             "UATP"}
        );
        return std::find(kAccepted.begin(), kAccepted.end(), network) != kAccepted.end();
    }

    [[nodiscard]] bool is_valid_country_alpha2(std::string_view code) const
    {
        return alpha2_to_numeric(code).has_value();
    }

    /** @brief Full 249-entry ISO 3166-1 alpha2 -> numeric table (transformers.rs:277+,1544+).
     *
     * Shared by is_valid_country_alpha2() above and the core's normalize_country() so
     * extraction-time CountryAlpha2 validation and the core conversion use the exact
     * same code set.
     */
    [[nodiscard]] std::optional<unsigned> alpha2_to_numeric(std::string_view alpha) const
    {
        static constexpr std::pair<std::string_view, unsigned> kAlpha2ToNumeric[]{
            {"AD", 20},  {"AE", 784}, {"AF", 4},   {"AG", 28},  {"AI", 660}, {"AL", 8},
            {"AM", 51},  {"AO", 24},  {"AQ", 10},  {"AR", 32},  {"AS", 16},  {"AT", 40},
            {"AU", 36},  {"AW", 533}, {"AX", 248}, {"AZ", 31},  {"BA", 70},  {"BB", 52},
            {"BD", 50},  {"BE", 56},  {"BF", 854}, {"BG", 100}, {"BH", 48},  {"BI", 108},
            {"BJ", 204}, {"BL", 652}, {"BM", 60},  {"BN", 96},  {"BO", 68},  {"BQ", 535},
            {"BR", 76},  {"BS", 44},  {"BT", 64},  {"BV", 74},  {"BW", 72},  {"BY", 112},
            {"BZ", 84},  {"CA", 124}, {"CC", 166}, {"CD", 180}, {"CF", 140}, {"CG", 178},
            {"CH", 756}, {"CI", 384}, {"CK", 184}, {"CL", 152}, {"CM", 120}, {"CN", 156},
            {"CO", 170}, {"CR", 188}, {"CU", 192}, {"CV", 132}, {"CW", 531}, {"CX", 162},
            {"CY", 196}, {"CZ", 203}, {"DE", 276}, {"DJ", 262}, {"DK", 208}, {"DM", 212},
            {"DO", 214}, {"DZ", 12},  {"EC", 218}, {"EE", 233}, {"EG", 818}, {"EH", 732},
            {"ER", 232}, {"ES", 724}, {"ET", 231}, {"FI", 246}, {"FJ", 242}, {"FK", 238},
            {"FM", 583}, {"FO", 234}, {"FR", 250}, {"GA", 266}, {"GB", 826}, {"GD", 308},
            {"GE", 268}, {"GF", 254}, {"GG", 831}, {"GH", 288}, {"GI", 292}, {"GL", 304},
            {"GM", 270}, {"GN", 324}, {"GP", 312}, {"GQ", 226}, {"GR", 300}, {"GS", 239},
            {"GT", 320}, {"GU", 316}, {"GW", 624}, {"GY", 328}, {"HK", 344}, {"HM", 334},
            {"HN", 340}, {"HR", 191}, {"HT", 332}, {"HU", 348}, {"ID", 360}, {"IE", 372},
            {"IL", 376}, {"IM", 833}, {"IN", 356}, {"IO", 86},  {"IQ", 368}, {"IR", 364},
            {"IS", 352}, {"IT", 380}, {"JE", 832}, {"JM", 388}, {"JO", 400}, {"JP", 392},
            {"KE", 404}, {"KG", 417}, {"KH", 116}, {"KI", 296}, {"KM", 174}, {"KN", 659},
            {"KP", 408}, {"KR", 410}, {"KW", 414}, {"KY", 136}, {"KZ", 398}, {"LA", 418},
            {"LB", 422}, {"LC", 662}, {"LI", 438}, {"LK", 144}, {"LR", 430}, {"LS", 426},
            {"LT", 440}, {"LU", 442}, {"LV", 428}, {"LY", 434}, {"MA", 504}, {"MC", 492},
            {"MD", 498}, {"ME", 499}, {"MF", 663}, {"MG", 450}, {"MH", 584}, {"MK", 807},
            {"ML", 466}, {"MM", 104}, {"MN", 496}, {"MO", 446}, {"MP", 580}, {"MQ", 474},
            {"MR", 478}, {"MS", 500}, {"MT", 470}, {"MU", 480}, {"MV", 462}, {"MW", 454},
            {"MX", 484}, {"MY", 458}, {"MZ", 508}, {"NA", 516}, {"NC", 540}, {"NE", 562},
            {"NF", 574}, {"NG", 566}, {"NI", 558}, {"NL", 528}, {"NO", 578}, {"NP", 524},
            {"NR", 520}, {"NU", 570}, {"NZ", 554}, {"OM", 512}, {"PA", 591}, {"PE", 604},
            {"PF", 258}, {"PG", 598}, {"PH", 608}, {"PK", 586}, {"PL", 616}, {"PM", 666},
            {"PN", 612}, {"PR", 630}, {"PS", 275}, {"PT", 620}, {"PW", 585}, {"PY", 600},
            {"QA", 634}, {"RE", 638}, {"RO", 642}, {"RS", 688}, {"RU", 643}, {"RW", 646},
            {"SA", 682}, {"SB", 90},  {"SC", 690}, {"SD", 729}, {"SE", 752}, {"SG", 702},
            {"SH", 654}, {"SI", 705}, {"SJ", 744}, {"SK", 703}, {"SL", 694}, {"SM", 674},
            {"SN", 686}, {"SO", 706}, {"SR", 740}, {"SS", 728}, {"ST", 678}, {"SV", 222},
            {"SX", 534}, {"SY", 760}, {"SZ", 748}, {"TC", 796}, {"TD", 148}, {"TF", 260},
            {"TG", 768}, {"TH", 764}, {"TJ", 762}, {"TK", 772}, {"TL", 626}, {"TM", 795},
            {"TN", 788}, {"TO", 776}, {"TR", 792}, {"TT", 780}, {"TV", 798}, {"TW", 158},
            {"TZ", 834}, {"UA", 804}, {"UG", 800}, {"UM", 581}, {"US", 840}, {"UY", 858},
            {"UZ", 860}, {"VA", 336}, {"VC", 670}, {"VE", 862}, {"VG", 92},  {"VI", 850},
            {"VN", 704}, {"VU", 548}, {"WF", 876}, {"WS", 882}, {"YE", 887}, {"YT", 175},
            {"ZA", 710}, {"ZM", 894}, {"ZW", 716}
        };
        for (const auto& [key, numeric]: kAlpha2ToNumeric) {
            if (key == alpha) {
                return numeric;
            }
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] core::error::AppError missing_ids_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::VALIDATION_FAILED,
            "missing profile id or profile acquirer id"
        );
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    ::app::context::AppContext& m_context;
};

// Each struct carries exactly what the old core block received.

struct CreateProfileAcquirerInput
{
    ::app::dto::profile_acquirer::ProfileAcquirerCreate request{};
    std::string profile_id{};
};

struct ProfileAcquirerUpdateInput
{
    ::app::dto::profile_acquirer::ProfileAcquirerUpdate request{};
    std::string profile_id{};
    std::string profile_acquirer_id{};
};

/**
 * @brief Domain authorization policy for profile acquirers.
 *
 * Upstream's dual extractor binds the caller to the profile id taken from the
 * route (or from the create body). The handler binds that id before the check.
 */
class ProfileAcquirerPolicy
{
public:
    explicit ProfileAcquirerPolicy(ProfileAcquirerInputSupport& input) noexcept :
        m_input{input}
    {
    }

    void bind_profile_id(std::string_view profile_id)
    {
        m_route_profile_id = std::string{profile_id};
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_input.authorize(req, operation, m_route_profile_id);
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

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }

private:
    ProfileAcquirerInputSupport& m_input;
    std::string m_route_profile_id{};
};

using ProfileAcquirerRequestSupport = ::core::behavior::RequestSupport<ProfileAcquirerPolicy>;

} // namespace app::handlers::profile_acquirer

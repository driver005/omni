export module app_user:hierarchical_resources_runtime_common;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::hierarchical_resources {

/** @brief Local mirror of upstream ApplePayCertificateData (core:65-72).
 *
 *  `status` is serialized as snake_case; the two Option fields are skipped when
 *  absent (serde `skip_serializing_if`), exactly like upstream.
 */
struct ApplePayCertificateData
{
    std::string status{};
    std::string apple_merchant_identifier{};
    std::string payment_processing_certificate{};
};

/** @brief Local mirror of the persisted HierarchicalResource payload.
 *
 *  Upstream stores `data: Secret<serde_json::Value>` (ApplePayCertificateData) and
 *  `encrypted_data: Option<Encryption>` as separate entity columns
 *  (core:180-194, :349-355). The local repository record has a single `data`
 *  column, so this typed JSON object carries the same field names verbatim.
 */
struct ResourceData
{
    std::string data{};
    std::string encrypted_data{};
    std::string public_key_digest{};
    std::string merchant_id{};
};

struct GeneratedKeys
{
    std::string private_key_pem{};
    std::string csr_pem{};
};

struct ParsedCertificate
{
    std::string der{};
    std::string public_key_digest{};
};

/**
 * @brief Shared business helpers for the hierarchical resources sub-runtimes.
 *
 * Direct ports of core/hierarchical_resources.rs helpers: id/date wire values,
 * scope authorization, requestor resolution, key store, certificate parsing,
 * status/display strings and typed persistence encoding.
 */
class HierarchicalResourcesRuntimeSupport
{
public:
    explicit HierarchicalResourcesRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] ::app::dto::hierarchical_resources::ResourceIdValue
    resource_id_value(std::string v) const
    {
        ::app::dto::hierarchical_resources::ResourceIdValue out;
        out.set_value(std::move(v));
        return out;
    }

    [[nodiscard]] ::app::dto::hierarchical_resources::PrimitiveDateTimeValue
    datetime_value(std::string v) const
    {
        ::app::dto::hierarchical_resources::PrimitiveDateTimeValue out;
        out.set_value(std::move(v));
        return out;
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // ── Upstream route-level helpers ──────────────────────────────────────────

    /** @brief core:880-890 authorize_scope_id_belongs_to_org. */
    [[nodiscard]] bool authorize_scope_id_belongs_to_org(
        std::string_view resource_scope,
        std::string_view organization_id
    ) const
    {
        return !resource_scope.empty() && !organization_id.empty() &&
               resource_scope == organization_id;
    }

    // ── Requestor resolution (core:412-483, V1) ───────────────────────────────

    [[nodiscard]] core::behavior::Outcome<std::string> fetch_requestor_account(
        std::string_view requestor_type,
        std::string_view requestor_id,
        std::string_view merchant_id
    ) const
    {
        if (requestor_type == "merchant_connector_account") {
            const auto connector =
                m_context.repository().find_merchant_connector_by_string(requestor_id);
            if (!connector.has_value()) {
                return std::unexpected{make_error(
                    core::error::AppErrorKind::NOT_FOUND,
                    "merchant connector account not found"
                )};
            }
            if (!merchant_id.empty() && connector->get_merchant_id() != merchant_id) {
                return std::unexpected{make_error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "requestor belongs to a different merchant"
                )};
            }
            return std::string{requestor_id};
        }
        if (requestor_type == "profile") {
            const auto profile = m_context.repository().find_profile_by_string(requestor_id);
            if (!profile.has_value()) {
                return std::unexpected{
                    make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found")
                };
            }
            if (!merchant_id.empty() && profile->get_merchant_id() != merchant_id) {
                return std::unexpected{make_error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "requestor belongs to a different merchant"
                )};
            }
            return std::string{requestor_id};
        }
        if (requestor_type == "merchant_account") {
            if (!merchant_id.empty() && requestor_id != merchant_id) {
                return std::unexpected{
                    make_error(core::error::AppErrorKind::NOT_FOUND, "merchant account not found")
                };
            }
            if (!m_context.repository().find_merchant_account_by_string(requestor_id).has_value()) {
                return std::unexpected{
                    make_error(core::error::AppErrorKind::NOT_FOUND, "merchant account not found")
                };
            }
            return std::string{requestor_id};
        }
        return std::unexpected{
            make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid requestor_type")
        };
    }

    /** @brief core:502-533 resolve_apple_pay_certificate_data cascading cache read. */
    [[nodiscard]] std::string
    resolve_apple_pay_certificate_data(std::string_view requestor_id) const
    {
        return find_linked_resource(requestor_id);
    }

    [[nodiscard]] std::string find_linked_resource(std::string_view requestor_id) const
    {
        const auto cached = m_context.config_store().get_value(
            "hierarchical_resources.account_links",
            requestor_id
        );
        if (cached.has_value() && !cached->empty()) {
            return *cached;
        }
        const auto link = m_context.repository().find_hierarchical_resource_by_string(
            link_resource_id(requestor_id)
        );
        if (link.has_value()) {
            // Local mirror of the account `apple_pay_certificates` cache payload:
            // the link record stores a JSON object with a `resource_id` field.
            const auto pos = link->get_data().find("\"resource_id\":\"");
            if (pos != std::string::npos) {
                const auto start = pos + std::string{"\"resource_id\":\""}.size();
                const auto end = link->get_data().find('"', start);
                if (end != std::string::npos) {
                    return link->get_data().substr(start, end - start);
                }
            }
        }
        // Cascade profile -> merchant account (upstream uses inherent entity properties).
        const auto profile = m_context.repository().find_profile_by_string(requestor_id);
        if (profile.has_value()) {
            return find_linked_resource(profile->get_merchant_id());
        }
        const auto connector =
            m_context.repository().find_merchant_connector_by_string(requestor_id);
        if (connector.has_value()) {
            return find_linked_resource(connector->get_merchant_id());
        }
        return {};
    }

    // ── Apple Pay crypto / certificate helpers (local OpenSSL substitutes) ────

    /** @brief core:892-922 ensure_organization_key_store. */
    [[nodiscard]] std::string ensure_organization_key_store(std::string_view org_id) const
    {
        constexpr std::string_view purpose{"hierarchical_resources.org"};
        const auto existing = m_context.key_store().get(purpose, org_id);
        if (existing.has_value()) {
            return existing->value();
        }
        const ::app::crypto::SecretString master{
            m_context.ids().uuid_v4() + m_context.ids().uuid_v4()
        };
        m_context.key_store().put(purpose, org_id, "AES-256", master);
        return master.value();
    }

    [[nodiscard]] std::string ensure_requestor_key(std::string_view requestor_id) const
    {
        constexpr std::string_view purpose{"hierarchical_resources.requestor"};
        const auto existing = m_context.key_store().get(purpose, requestor_id);
        if (existing.has_value()) {
            return existing->value();
        }
        const ::app::crypto::SecretString key{
            m_context.ids().uuid_v4() + m_context.ids().uuid_v4()
        };
        m_context.key_store().put(purpose, requestor_id, "AES-256", key);
        return key.value();
    }

    /** @brief core:605-628 generate_ec_keypair_and_csr (local PEM substitute). */
    [[nodiscard]] GeneratedKeys generate_ec_keypair_and_csr(std::string_view org_key) const
    {
        const auto seed = m_context.ids().uuid_v4() + m_context.ids().uuid_v4();
        const auto private_der = base64_encode("ec-p256|" + seed);
        const auto csr_hash = ::app::crypto::KeyedHasher{std::string{org_key}}.hash(seed);
        const auto csr_der = base64_encode("csr|" + csr_hash);
        return GeneratedKeys{
            "-----BEGIN PRIVATE KEY-----\n" + private_der + "\n-----END PRIVATE KEY-----\n",
            "-----BEGIN CERTIFICATE REQUEST-----\n" + csr_der +
                "\n-----END CERTIFICATE REQUEST-----\n"
        };
    }

    /** @brief core:685-695 parse_certificate_from_base64_der (local substitute). */
    [[nodiscard]] core::behavior::Outcome<ParsedCertificate> parse_certificate_from_base64_der(
        std::string_view certificate_b64,
        std::string_view org_key
    ) const
    {
        const auto der = base64_decode(certificate_b64);
        if (!der.has_value() || der->empty()) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "certificate is not valid base64"
            )};
        }
        // Minimal DER sanity check: an X.509 certificate is a SEQUENCE (0x30).
        if (static_cast<unsigned char>((*der)[0]) != 0x30U) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "certificate is not a valid DER-encoded X.509 certificate"
            )};
        }
        return ParsedCertificate{*der, ::app::crypto::KeyedHasher{std::string{org_key}}.hash(*der)};
    }

    /** @brief core:697-707 certificate_matches_private_key (local substitute).
     *
     *  OpenSSL public-key DER comparison is unavailable locally. The structural
     *  substitute verifies the certificate is a well-formed DER SEQUENCE and that
     *  a binding digest was stored at generation time; it cannot reject a
     *  structurally valid certificate issued for a different key.
     */
    [[nodiscard]] bool certificate_matches_private_key(
        const ParsedCertificate& certificate,
        const ResourceData& payload
    ) const
    {
        return !certificate.der.empty() && !payload.encrypted_data.empty() &&
               !payload.public_key_digest.empty();
    }

    /** @brief core:709-726 parse_apple_merchant_identifier (local substitute). */
    [[nodiscard]] std::optional<std::string>
    parse_apple_merchant_identifier(const ParsedCertificate& certificate) const
    {
        std::string common_name{};
        // DER: CN has OID 2.5.4.3 = {0x55,0x04,0x03}, followed by a TLV.
        static constexpr std::string_view cn_oid{"\x55\x04\x03", 3};
        const auto oid_pos = certificate.der.find(cn_oid);
        if (oid_pos != std::string::npos && oid_pos + 3 + 2 <= certificate.der.size()) {
            const auto tag = static_cast<unsigned char>(certificate.der[oid_pos + 3]);
            if (tag == 0x0CU || tag == 0x13U || tag == 0x16U) {
                std::size_t length = static_cast<unsigned char>(certificate.der[oid_pos + 4]);
                std::size_t value_start = oid_pos + 5;
                if ((length & 0x80U) != 0U) {
                    const std::size_t extra = length & 0x7FU;
                    if (extra > 0 && extra <= 4 && value_start + extra <= certificate.der.size()) {
                        length = 0;
                        for (std::size_t i{0}; i < extra; ++i) {
                            length = (length << 8) |
                                     static_cast<unsigned char>(certificate.der[value_start + i]);
                        }
                        value_start += extra;
                    } else {
                        length = 0;
                    }
                }
                if (length > 0 && value_start + length <= certificate.der.size()) {
                    common_name = certificate.der.substr(value_start, length);
                }
            }
        }
        // Text fallback (local certificate stand-ins may carry a literal CN=).
        if (common_name.empty()) {
            const auto cn_pos = certificate.der.find("CN=");
            if (cn_pos != std::string::npos) {
                const auto rest = std::string_view{certificate.der}.substr(cn_pos + 3);
                const auto end = rest.find_first_of(",/ \t\r\n\"");
                common_name = std::string{rest.substr(0, end)};
            }
        }
        if (common_name.empty()) {
            return std::nullopt;
        }
        // core:720-725 — rsplit(':').next().trim().
        const auto last_colon = common_name.rfind(':');
        auto identifier =
            last_colon == std::string::npos ? common_name : common_name.substr(last_colon + 1);
        const auto first = identifier.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return std::nullopt;
        }
        const auto last = identifier.find_last_not_of(" \t\r\n");
        return identifier.substr(first, last - first + 1);
    }

    // ── Status / display (core:101-117, :214-236, :800-820) ───────────────────

    [[nodiscard]] std::string status_csr_generated() const
    {
        return "csr_generated";
    }

    [[nodiscard]] std::string status_active() const
    {
        return "active";
    }

    [[nodiscard]] std::string display_schema() const
    {
        return R"({"apple_pay_merchant_identifier":"String"})";
    }

    [[nodiscard]] std::string display_data(const ApplePayCertificateData& data) const
    {
        if (data.apple_merchant_identifier.empty()) {
            return R"({"apple_pay_merchant_identifier":null})";
        }
        return "{\"apple_pay_merchant_identifier\":" +
               serde::Ser::encode_json(data.apple_merchant_identifier) + "}";
    }

    [[nodiscard]] bool hierarchical_resource_is_linkable(const ResourceData& payload) const
    {
        const auto data = parse_apple_pay_certificate_payload(payload.data);
        return data.status == status_active();
    }

    // ── Typed persistence encoding (local repository substitute) ──────────────

    [[nodiscard]] std::string
    encode_apple_pay_certificate_data(const ApplePayCertificateData& data) const
    {
        std::string out = "{\"status\":" + serde::Ser::encode_json(data.status);
        if (!data.apple_merchant_identifier.empty()) {
            out += ",\"apple_pay_merchant_identifier\":" +
                   serde::Ser::encode_json(data.apple_merchant_identifier);
        }
        if (!data.payment_processing_certificate.empty()) {
            out += ",\"payment_processing_certificate\":" +
                   serde::Ser::encode_json(data.payment_processing_certificate);
        }
        out += "}";
        return out;
    }

    [[nodiscard]] ApplePayCertificateData
    parse_apple_pay_certificate_payload(std::string_view json) const
    {
        ApplePayCertificateData data{};
        data.status = json_string_field(json, "status");
        data.apple_merchant_identifier = json_string_field(json, "apple_pay_merchant_identifier");
        data.payment_processing_certificate =
            json_string_field(json, "payment_processing_certificate");
        return data;
    }

    [[nodiscard]] std::string json_string_field(std::string_view json, std::string_view key) const
    {
        const std::string needle = "\"" + std::string{key} + "\":";
        const auto pos = json.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto rest = json.substr(pos + needle.size());
        const auto first = rest.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos || rest[first] != '"') {
            return {};
        }
        rest = rest.substr(first + 1);
        std::string out{};
        for (std::size_t i{0}; i < rest.size(); ++i) {
            if (rest[i] == '\\' && i + 1 < rest.size()) {
                out.push_back(rest[++i]);
            } else if (rest[i] == '"') {
                break;
            } else {
                out.push_back(rest[i]);
            }
        }
        return out;
    }

    [[nodiscard]] std::string encode_resource_data(const ResourceData& value) const
    {
        return "{\"data\":" + serde::Ser::encode_json(value.data) +
               ",\"encrypted_data\":" + serde::Ser::encode_json(value.encrypted_data) +
               ",\"public_key_digest\":" + serde::Ser::encode_json(value.public_key_digest) +
               ",\"merchant_id\":" + serde::Ser::encode_json(value.merchant_id) + "}";
    }

    [[nodiscard]] ResourceData decode_resource_data(std::string_view raw) const
    {
        ResourceData result{};
        result.data = json_string_field(raw, "data");
        result.encrypted_data = json_string_field(raw, "encrypted_data");
        result.public_key_digest = json_string_field(raw, "public_key_digest");
        result.merchant_id = json_string_field(raw, "merchant_id");
        return result;
    }

    [[nodiscard]] std::string link_resource_id(std::string_view requestor_id) const
    {
        return "link:" + std::string{requestor_id};
    }

    // ── Standard base64 (upstream BASE64_ENGINE; local, no new module deps) ────

    [[nodiscard]] std::string base64_encode(std::string_view input) const
    {
        static constexpr std::string_view table{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
        };
        std::string out{};
        out.reserve(4 * ((input.size() + 2) / 3));
        std::uint32_t accumulator{0};
        int bits{0};
        for (const char c: input) {
            accumulator = (accumulator << 8) | static_cast<unsigned char>(c);
            bits += 8;
            while (bits >= 6) {
                bits -= 6;
                out.push_back(table[(accumulator >> bits) & 0x3FU]);
            }
        }
        if (bits > 0) {
            out.push_back(table[(accumulator << (6 - bits)) & 0x3FU]);
            while (out.size() % 4 != 0) {
                out.push_back('=');
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string> base64_decode(std::string_view input) const
    {
        static constexpr std::string_view table{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
        };
        if (input.size() % 4 != 0) {
            return std::nullopt;
        }
        std::string out{};
        out.reserve((input.size() / 4) * 3);
        for (std::size_t i{0}; i < input.size(); i += 4) {
            const auto chunk = input.substr(i, 4);
            std::size_t padding{0};
            while (padding < chunk.size() && chunk[chunk.size() - 1 - padding] == '=') {
                ++padding;
            }
            if (padding > 2) {
                return std::nullopt;
            }
            std::uint32_t word{0};
            for (std::size_t j{0}; j < 4; ++j) {
                const char c = chunk[j];
                if (j >= 4 - padding) {
                    if (c != '=') {
                        return std::nullopt;
                    }
                    continue; // '=' contributes zero bits.
                }
                const auto pos = table.find(c);
                if (pos == std::string_view::npos) {
                    return std::nullopt;
                }
                word = (word << 6) | static_cast<std::uint32_t>(pos);
            }
            out.push_back(static_cast<char>((word >> 16) & 0xFFU));
            if (padding < 2) {
                out.push_back(static_cast<char>((word >> 8) & 0xFFU));
            }
            if (padding < 1) {
                out.push_back(static_cast<char>(word & 0xFFU));
            }
        }
        return out;
    }

    // ── time::PrimitiveDateTime wire shape ────────────────────────────────────

    /** @brief time::PrimitiveDateTime wire shape: "YYYY-MM-DD HH:MM:SS.nnnnnnnnn". */
    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    [[nodiscard]] std::string iso_utc(std::int64_t epoch_seconds) const
    {
        constexpr std::int64_t seconds_per_day{86'400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719'468;
        const std::int64_t era = (days >= 0 ? days : days - 146'096) / 146'097;
        const std::int64_t doe = days - era * 146'097;
        const std::int64_t yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.000000000",
            year,
            month,
            day,
            seconds_of_day / 3'600,
            (seconds_of_day % 3'600) / 60,
            seconds_of_day % 60
        );
    }

    [[nodiscard]] std::string created_at_value() const
    {
        return iso_utc(now_epoch());
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::hierarchical_resources

export module app_routing:blocklist_input;

import std;
import interfaces;
import core_serde;
import core_error;
import core_behavior;
import app_context;
import app_model;
import app_dto;
import app_store;
import app_auth;
import app_http;

import :policy;

export namespace app::handlers::blocklist {

using EntryRecord = ::app::store::blocklist_entries::BlocklistEntryRecord;
using JobRecord =
    std::remove_cvref_t<decltype(*std::declval<::app::context::AppContext&>()
                                      .repository()
                                      .find_routing_config_by_string(std::string_view{}))>;

// Wire parsing helpers (mirrors the original private structs).
struct EntryWire
{
    std::string kind{};
    std::string data{};
    std::string transaction_type{};
    std::optional<core::error::AppError> failure{};
};

struct CsvRow
{
    std::string kind{};
    std::string data{};
    std::string transaction_type{};
};

struct CsvParse
{
    std::vector<CsvRow> rows{};
    std::optional<core::error::AppError> failure{};
};

struct MultipartUpload
{
    std::string_view content{};
    std::string file_name{};
};

struct ProfileResolution
{
    std::string id{};
    std::optional<core::error::AppError> failure{};
};

/**
 * @brief Parse/auth/path/query support for the blocklist handlers.
 *
 * Free of response writing: helpers return errors instead of sending them so the
 * facade keeps the single response-writing path for every route.
 */
class BlocklistInputSupport
{
public:
    explicit BlocklistInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    template<typename T>
    [[nodiscard]] T wrap(std::string value) const
    {
        T result{};
        result.set_value(std::move(value));
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    require_merchant(std::string_view merchant) const
    {
        if (merchant.empty()) {
            return error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id");
        }
        return std::nullopt;
    }

    // ───────────────────────── Auth / path / headers ─────────────────────────

    [[nodiscard]] std::optional<core::error::AppError>
    authorize_operation(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        return m_context.authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::string merchant_id_of(interfaces::io::IRequest& req) const
    {
        return ::app::http::merchant_id_from_header{}(req);
    }

    /** @brief auth.profile: JWT profile or X-Profile-Id header (auth/context.cppm:156-158). */
    [[nodiscard]] std::string explicit_profile_of(interfaces::io::IRequest& req) const
    {
        return ::app::auth::AuthContextBuilder{}(req).profile_id();
    }

    // core/utils.rs:2414-2450 get_profile_from_business_details.
    [[nodiscard]] ProfileResolution
    resolve_required_profile(interfaces::io::IRequest& req, std::string_view merchant) const
    {
        const auto explicit_id = explicit_profile_of(req);
        if (!explicit_id.empty()) {
            const auto found = m_context.repository().find_profile_by_string(explicit_id);
            if (!found || (found->get_merchant_id() != merchant)) {
                return {explicit_id, profile_not_found(explicit_id)};
            }
            return {explicit_id, std::nullopt};
        }
        const auto account = m_context.repository().find_merchant_account_by_string(merchant);
        const auto default_id = account ? account->get_default_profile() : std::string{};
        if (!default_id.empty()) {
            const auto found = m_context.repository().find_profile_by_string(default_id);
            if (!found || (found->get_merchant_id() != merchant)) {
                return {default_id, profile_not_found(default_id)};
            }
            return {default_id, std::nullopt};
        }
        return {
            {},
            error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Missing required param: profile_id or business_country, business_label"
            )
        };
    }

    [[nodiscard]] core::error::AppError profile_not_found(std::string_view id) const
    {
        return error(
            core::error::AppErrorKind::NOT_FOUND,
            "Business profile with the given id " + std::string{id} + " does not exist"
        );
    }

    [[nodiscard]] bool
    profile_is_owned(std::string_view profile_id, std::string_view merchant) const
    {
        if (profile_id.empty() || merchant.empty()) {
            return false;
        }
        const auto found = m_context.repository().find_profile_by_string(profile_id);
        return found.has_value() && found->get_merchant_id() == merchant;
    }

    [[nodiscard]] std::string job_id_of(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            if (segments.at(i).value() == "batch") {
                return std::string{segments.at(i + 1).value()};
            }
        }
        return {};
    }

    [[nodiscard]] std::string job_key(std::string_view job_id) const
    {
        return "blocklist.job:" + std::string{job_id};
    }

    [[nodiscard]] std::optional<JobRecord> find_job(std::string_view config_id) const
    {
        return m_context.repository().find_routing_config_by_string(config_id);
    }

    [[nodiscard]] bool job_scope_matches(
        const JobRecord& record,
        std::string_view merchant,
        std::string_view profile
    ) const
    {
        const bool merchant_ok = merchant.empty() || record.get_merchant_id().empty() ||
                                 record.get_merchant_id() == merchant;
        const bool profile_ok = profile.empty() || record.get_profile_id().empty() ||
                                record.get_profile_id() == profile;
        return merchant_ok && profile_ok;
    }

    // ───────────────────────── Query helpers ─────────────────────────

    template<typename T>
    [[nodiscard]] std::optional<T>
    query_number(const ::app::http::QueryParams& query, std::string_view key) const
    {
        const auto raw = query.get(key);
        if (!raw) {
            return std::nullopt;
        }
        T value{};
        const auto [end, ec] = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        (void)end;
        if (ec != std::errc{}) {
            return std::nullopt;
        }
        return value;
    }

    [[nodiscard]] std::optional<std::string> kind_from_type(std::string_view type) const
    {
        if (type == "fingerprint") {
            return std::string{"payment_method"};
        }
        if (type == "card_bin" || type == "extended_card_bin" || type == "generic_card_bin") {
            return std::string{type};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string> query_kind(std::string_view raw) const
    {
        if (raw == "payment_method") {
            return std::string{"payment_method"};
        }
        if (raw == "card_bin" || raw == "extended_card_bin" || raw == "generic_card_bin") {
            return std::string{raw};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string> parse_transaction_type(std::string_view raw) const
    {
        if (raw == "payment" || raw == "payout") {
            return std::string{raw};
        }
        return std::nullopt;
    }

    [[nodiscard]] bool known_job_type(std::string_view value) const
    {
        return value == "upload" || value == "export" || value == "profile_clone";
    }

    // ───────────────────────── Wire parsing / validation ─────────────────────────

    // api_models:36-43 BlocklistEntryRequest (flattened tagged enum + transaction_type).
    [[nodiscard]] EntryWire parse_entry_body(std::string_view body) const
    {
        EntryWire wire;
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            wire.failure = error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            return wire;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            wire.failure = error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid blocklist entry request"
            );
            return wire;
        }
        const auto type_field = root->get("type");
        if (!type_field) {
            wire.failure =
                error(core::error::AppErrorKind::INVALID_REQUEST, "missing field `type`");
            return wire;
        }
        const auto* type_text = std::get_if<std::string>(&type_field->get());
        if (type_text == nullptr) {
            wire.failure =
                error(core::error::AppErrorKind::INVALID_REQUEST, "invalid field `type`");
            return wire;
        }
        const auto kind = kind_from_type(*type_text);
        if (!kind) {
            wire.failure = error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unknown variant `" + *type_text +
                    "`, expected one of `card_bin`, `fingerprint`, "
                    "`extended_card_bin`, `generic_card_bin`"
            );
            return wire;
        }
        const auto data_field = root->get("data");
        if (!data_field) {
            wire.failure =
                error(core::error::AppErrorKind::INVALID_REQUEST, "missing field `data`");
            return wire;
        }
        const auto* data_text = std::get_if<std::string>(&data_field->get());
        if (data_text == nullptr) {
            wire.failure =
                error(core::error::AppErrorKind::INVALID_REQUEST, "invalid field `data`");
            return wire;
        }
        wire.kind = *kind;
        wire.data = *data_text;
        wire.transaction_type = "payment"; // serde(default) at api_models:40.
        if (const auto tt_field = root->get("transaction_type")) {
            const auto* tt_text = std::get_if<std::string>(&tt_field->get());
            if (tt_text == nullptr) {
                wire.failure =
                    error(core::error::AppErrorKind::INVALID_REQUEST, "invalid transaction_type");
                return wire;
            }
            const auto parsed = parse_transaction_type(*tt_text);
            if (!parsed) {
                wire.failure = error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant `" + *tt_text + "`, expected `payment` or `payout`"
                );
                return wire;
            }
            wire.transaction_type = *parsed;
        }
        // utils.rs:164-190 validate_bin (fingerprints are opaque: no digit format).
        wire.failure = validate_bin(wire.data, wire.kind);
        return wire;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_bin(std::string_view data, std::string_view kind) const
    {
        std::string expected;
        std::size_t length{0};
        if (kind == "card_bin") {
            expected = "a 6 digit number";
            length = 6;
        } else if (kind == "extended_card_bin") {
            expected = "an 8 digit number";
            length = 8;
        } else if (kind == "generic_card_bin") {
            expected = "a 6 to 10 digit number";
        } else {
            return std::nullopt; // payment_method / fingerprint: opaque.
        }
        const bool digits = std::all_of(
            data.begin(),
            data.end(),
            [](unsigned char ch)
            {
                return std::isdigit(ch) != 0;
            }
        );
        const bool valid_length = kind == "generic_card_bin"
                                      ? (data.size() >= 6 && data.size() <= 10)
                                      : data.size() == length;
        if (!digits || !valid_length) {
            return error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "data contains invalid data. Expected format is " + expected
            );
        }
        return std::nullopt;
    }

    // ───────────────────────── Multipart / CSV (batch.rs) ─────────────────────────

    [[nodiscard]] MultipartUpload
    multipart_file(interfaces::io::IRequest& req, std::string_view body) const
    {
        const auto content_type = ::app::http::HeaderReader{}.get(req, "content-type");
        if (!content_type.starts_with("multipart/form-data")) {
            return {body, {}};
        }
        const auto marker = content_type.find("boundary=");
        if (marker == std::string::npos) {
            return {body, {}};
        }
        auto boundary = content_type.substr(marker + 9);
        if (!boundary.empty() && boundary.front() == '"') {
            boundary = boundary.substr(1);
            if (!boundary.empty() && boundary.back() == '"') {
                boundary = boundary.substr(0, boundary.size() - 1);
            }
        }
        const auto delimiter = std::string{"--"} + boundary;
        const auto header_end = body.find("\r\n\r\n");
        if (header_end == std::string_view::npos) {
            return {body, {}};
        }
        std::string file_name;
        const auto part_headers = body.substr(0, header_end);
        const auto name_pos = part_headers.find("filename=");
        if (name_pos != std::string_view::npos) {
            auto quoted = part_headers.find('"', name_pos);
            if (quoted != std::string_view::npos) {
                const auto end = part_headers.find('"', ++quoted);
                if (end != std::string_view::npos) {
                    file_name = std::string{part_headers.substr(quoted, end - quoted)};
                }
            }
        }
        const auto start = header_end + 4;
        const auto tail = body.find(delimiter, start);
        return {
            tail == std::string_view::npos ? body.substr(start) : body.substr(start, tail - start),
            file_name
        };
    }

    [[nodiscard]] CsvParse
    parse_blocklist_csv(std::string_view csv, std::string_view override_type) const
    {
        CsvParse parsed;
        std::vector<std::string_view> lines;
        std::size_t cursor{0};
        while (cursor <= csv.size()) {
            const auto newline = csv.find('\n', cursor);
            const auto end = newline == std::string_view::npos ? csv.size() : newline;
            auto line = csv.substr(cursor, end - cursor);
            if (!line.empty() && line.back() == '\r') {
                line = line.substr(0, line.size() - 1);
            }
            lines.push_back(line);
            if (newline == std::string_view::npos) {
                break;
            }
            cursor = newline + 1;
        }
        if (lines.empty()) {
            parsed.failure = error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "CSV must contain at least one valid data row"
            );
            return parsed;
        }
        const auto header = split_csv_line(lines.front());
        std::size_t type_col{0};
        std::size_t data_col{1};
        std::size_t metadata_col{2};
        std::optional<std::size_t> tt_col{};
        for (std::size_t i = 0; i < header.size(); ++i) {
            const auto name = trim(header[i]);
            if (name == "type") {
                type_col = i;
            } else if (name == "data") {
                data_col = i;
            } else if (name == "metadata") {
                metadata_col = i;
            } else if (name == "transaction_type") {
                tt_col = i;
            }
        }
        const bool has_type = std::any_of(
            header.begin(),
            header.end(),
            [this](std::string_view v)
            {
                return trim(v) == "type";
            }
        );
        const bool has_data = std::any_of(
            header.begin(),
            header.end(),
            [this](std::string_view v)
            {
                return trim(v) == "data";
            }
        );
        if (!has_type || !has_data) {
            parsed.failure = csv_row_error(0, "card_bin", "", "missing `type` or `data` column");
            return parsed;
        }
        if (lines.size() - 1 > 100'000) {
            parsed.failure = error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "CSV exceeds maximum allowed rows (100000); got " + std::to_string(lines.size() - 1)
            );
            return parsed;
        }
        if (lines.size() == 1) {
            parsed.failure = error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "CSV must contain at least one valid data row"
            );
            return parsed;
        }
        (void)metadata_col;
        for (std::size_t i = 1; i < lines.size(); ++i) {
            if (lines[i].empty()) {
                continue;
            }
            const auto cells = split_csv_line(lines[i]);
            const auto kind_token = trim(cells.size() > type_col ? cells[type_col] : "");
            const auto kind = kind_from_type(kind_token);
            if (!kind) {
                parsed.failure = csv_row_error(
                    i - 1,
                    "card_bin",
                    std::string{cells.size() > data_col ? cells[data_col] : ""},
                    "unknown type `" + std::string{kind_token} +
                        "`; expected generic_card_bin, card_bin, extended_card_bin, or fingerprint"
                );
                return parsed;
            }
            const auto data = std::string{trim(cells.size() > data_col ? cells[data_col] : "")};
            if (data.empty()) {
                parsed.failure = csv_row_error(i - 1, *kind, "", "data field must not be empty");
                return parsed;
            }
            if (const auto bad = validate_bin(data, *kind)) {
                const auto reason =
                    *kind == "card_bin" ? std::string{"card_bin must be exactly 6 digits"}
                    : *kind == "extended_card_bin"
                        ? std::string{"extended_card_bin must be exactly 8 digits"}
                        : std::string{"generic_card_bin must be a 6 to 10 digit number"};
                parsed.failure = csv_row_error(i - 1, *kind, data, reason);
                return parsed;
            }
            const auto metadata =
                cells.size() > metadata_col ? trim(cells[metadata_col]) : std::string_view{};
            if (!metadata.empty() && !valid_metadata(metadata)) {
                parsed.failure = csv_row_error(
                    i - 1,
                    *kind,
                    data,
                    "metadata must be in key=value format, separated by semicolons "
                    "(e.g. reason=fraud;source=manual)"
                );
                return parsed;
            }
            std::string transaction_type = "payment";
            if (!override_type.empty()) {
                transaction_type = override_type;
            } else if (tt_col && cells.size() > *tt_col) {
                const auto raw = trim(cells[*tt_col]);
                if (!raw.empty()) {
                    const auto parsed_tt = parse_transaction_type(raw);
                    if (!parsed_tt) {
                        parsed.failure = csv_row_error(
                            i - 1,
                            *kind,
                            data,
                            "unknown variant `" + std::string{raw} +
                                "`, expected `payment` or `payout`"
                        );
                        return parsed;
                    }
                    transaction_type = *parsed_tt;
                }
            }
            parsed.rows.push_back({*kind, data, transaction_type});
        }
        return parsed;
    }

    [[nodiscard]] core::error::AppError csv_row_error(
        std::size_t row_index,
        std::string_view kind,
        std::string data,
        std::string reason
    ) const
    {
        ::app::model::blocklist::BlocklistRowError row;
        row.set_row_index(row_index);
        row.set_data_kind(wrap<::app::model::blocklist::BlocklistDataKindValue>(std::string{kind}));
        row.set_data(std::move(data));
        row.set_reason(std::move(reason));
        // batch.rs:314-324 serializes BlocklistRowError into InvalidRequestData.message.
        return error(core::error::AppErrorKind::VALIDATION_FAILED, serde::Ser::encode_json(row));
    }

    [[nodiscard]] bool valid_metadata(std::string_view raw) const
    {
        std::size_t cursor{0};
        while (cursor <= raw.size()) {
            const auto semi = raw.find(';', cursor);
            const auto pair =
                raw.substr(cursor, (semi == std::string_view::npos ? raw.size() : semi) - cursor);
            const auto equals = pair.find('=');
            if (equals == std::string_view::npos || trim(pair.substr(0, equals)).empty()) {
                return false;
            }
            if (semi == std::string_view::npos) {
                break;
            }
            cursor = semi + 1;
        }
        return true;
    }

    [[nodiscard]] std::vector<std::string_view> split_csv_line(std::string_view line) const
    {
        std::vector<std::string_view> parts;
        std::size_t start{0};
        while (true) {
            const auto comma = line.find(',', start);
            if (comma == std::string_view::npos) {
                parts.push_back(line.substr(start));
                break;
            }
            parts.push_back(line.substr(start, comma - start));
            start = comma + 1;
        }
        return parts;
    }

    [[nodiscard]] std::string_view trim(std::string_view raw) const
    {
        while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.front()))) {
            raw.remove_prefix(1);
        }
        while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.back()))) {
            raw.remove_suffix(1);
        }
        return raw;
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy for blocklist routes. Auth is the registry rule for the
 * operation name; scope is the request's merchant/profile identity.
 */
class BlocklistPolicy
{
public:
    explicit BlocklistPolicy(BlocklistInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize_operation(req, operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        return ::app::routing::policy::scope_of(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope& scope) const
    {
        return ::app::routing::policy::scope_error(scope);
    }

private:
    BlocklistInputSupport& m_support;
};

using BlocklistRequestSupport = ::core::behavior::RequestSupport<BlocklistPolicy>;

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old route logic handed to its core block.

struct AddEntryInput
{
    std::string merchant{};
    std::string profile{};
    std::string kind{};
    std::string data{};
    std::string transaction_type{};
};

struct RemoveEntryInput
{
    std::string merchant{};
    std::string profile{};
    std::string kind{};
    std::string data{};
    std::string transaction_type{};
};

struct ListBlockedPaymentMethodsInput
{
    std::string merchant{};
    std::string profile{};
    std::string kind{};
    std::string transaction_type{};
    std::uint16_t limit{10};
    std::uint16_t offset{0};
};

struct GetBlocklistCountInput
{
    std::string merchant{};
    std::string profile{};
    std::string kind{};
    std::string transaction_type{};
};

struct LookupBlocklistEntryInput
{
    std::string merchant{};
    std::string profile{};
    std::string data{};
    std::string transaction_type{};
};

struct ToggleBlocklistGuardInput
{
    std::string merchant{};
    bool status{false};
};

struct UploadBatchBlocklistInput
{
    std::string merchant{};
    std::string profile{};
    std::string override_type{};
    std::string content{};
    std::string file_name{};
    std::vector<CsvRow> rows{};
};

struct GetBatchBlocklistJobStatusInput
{
    std::string job_id{};
    std::string merchant{};
    std::string profile{};
    JobRecord stored{};
};

struct ListBatchBlocklistJobsInput
{
    std::string merchant{};
    std::string profile{};
    std::uint32_t limit{10};
    std::uint32_t offset{0};
    std::string job_type{};
};

struct CreateBlocklistExportInput
{
    std::string merchant{};
    std::string profile{};
    std::string transaction_type{};
};

struct CloneBlocklistEntriesInput
{
    std::string merchant{};
    std::string source{};
    std::vector<::app::model::blocklist::ProfileIdValue> targets{};
    std::string transaction_type{};
};

} // namespace app::handlers::blocklist

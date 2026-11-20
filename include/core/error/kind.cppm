module;
export module core_error:kind;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::error {

enum class AppErrorKind : std::uint8_t
{
    EMPTY_REQUEST,
    INVALID_JSON,
    NOT_FOUND,
    UNAUTHORIZED,
    FORBIDDEN,
    INTERNAL,

    ALREADY_EXISTS,
    PRECONDITION_FAILED,
    INVALID_REQUEST,
    INTERNAL_ERROR,
    DUPLICATE_REQUEST,
    CONNECTOR_ERROR,
    VALIDATION_FAILED,
    RATE_LIMITED,
    TIMEOUT,
};

/** @brief Wire name of an error kind. Instance functor, no static members. */
class AppErrorKindName
{
public:
    [[nodiscard]] std::string_view operator()(AppErrorKind kind) const noexcept
    {
        switch (kind) {
        case AppErrorKind::EMPTY_REQUEST: return "EMPTY_REQUEST";
        case AppErrorKind::INVALID_JSON: return "INVALID_JSON";
        case AppErrorKind::NOT_FOUND: return "NOT_FOUND";
        case AppErrorKind::UNAUTHORIZED: return "UNAUTHORIZED";
        case AppErrorKind::FORBIDDEN: return "FORBIDDEN";
        case AppErrorKind::INTERNAL: return "INTERNAL";
        case AppErrorKind::ALREADY_EXISTS: return "ALREADY_EXISTS";
        case AppErrorKind::PRECONDITION_FAILED: return "PRECONDITION_FAILED";
        case AppErrorKind::INVALID_REQUEST: return "INVALID_REQUEST";
        case AppErrorKind::INTERNAL_ERROR: return "INTERNAL_ERROR";
        case AppErrorKind::DUPLICATE_REQUEST: return "DUPLICATE_REQUEST";
        case AppErrorKind::CONNECTOR_ERROR: return "CONNECTOR_ERROR";
        case AppErrorKind::VALIDATION_FAILED: return "VALIDATION_FAILED";
        case AppErrorKind::RATE_LIMITED: return "RATE_LIMITED";
        case AppErrorKind::TIMEOUT: return "TIMEOUT";
        }
        return "INTERNAL";
    }
};

/** @brief Maps an error kind to its HTTP status. Instance functor, no static members. */
class AppErrorStatus
{
public:
    [[nodiscard]] interfaces::io::types::Status operator()(AppErrorKind kind) const noexcept
    {
        using interfaces::io::types::Status;

        switch (kind) {
        case AppErrorKind::EMPTY_REQUEST: return Status::BAD_REQUEST;
        case AppErrorKind::INVALID_JSON: return Status::BAD_REQUEST;
        case AppErrorKind::NOT_FOUND: return Status::NOT_FOUND;
        case AppErrorKind::UNAUTHORIZED: return Status::UNAUTHORIZED;
        case AppErrorKind::FORBIDDEN: return Status::FORBIDDEN;
        case AppErrorKind::INTERNAL: return Status::INTERNAL_SERVER_ERROR;
        case AppErrorKind::ALREADY_EXISTS: return Status::CONFLICT;
        case AppErrorKind::PRECONDITION_FAILED: return Status::CONFLICT;
        case AppErrorKind::INVALID_REQUEST: return Status::BAD_REQUEST;
        case AppErrorKind::INTERNAL_ERROR: return Status::INTERNAL_SERVER_ERROR;
        case AppErrorKind::DUPLICATE_REQUEST: return Status::CONFLICT;
        case AppErrorKind::CONNECTOR_ERROR: return Status::BAD_GATEWAY;
        case AppErrorKind::VALIDATION_FAILED: return Status::UNPROCESSABLE_CONTENT;
        case AppErrorKind::RATE_LIMITED: return Status::TOO_MANY_REQUESTS;
        case AppErrorKind::TIMEOUT: return Status::GATEWAY_TIMEOUT;
        }
        return Status::INTERNAL_SERVER_ERROR;
    }
};

} // namespace core::error

#ifdef CONGELADO_TEST
namespace core::error::tests {
using namespace boost::ut;

suite<"core_error_kind"> kind_suite = []
{
    "kind names are stable wire values"_test = []
    {
        expect(AppErrorKindName{}(AppErrorKind::INVALID_JSON) == "INVALID_JSON");
        expect(AppErrorKindName{}(AppErrorKind::EMPTY_REQUEST) == "EMPTY_REQUEST");
        expect(AppErrorKindName{}(AppErrorKind::RATE_LIMITED) == "RATE_LIMITED");
        expect(AppErrorKindName{}(AppErrorKind::TIMEOUT) == "TIMEOUT");
    };
};

suite<"core_error_status"> status_suite = []
{
    "kinds map to their HTTP status"_test = []
    {
        using interfaces::io::types::Status;
        expect(AppErrorStatus{}(AppErrorKind::NOT_FOUND) == Status::NOT_FOUND);
        expect(AppErrorStatus{}(AppErrorKind::FORBIDDEN) == Status::FORBIDDEN);
        expect(AppErrorStatus{}(AppErrorKind::UNAUTHORIZED) == Status::UNAUTHORIZED);
        expect(AppErrorStatus{}(AppErrorKind::ALREADY_EXISTS) == Status::CONFLICT);
        expect(AppErrorStatus{}(AppErrorKind::INVALID_REQUEST) == Status::BAD_REQUEST);
        expect(AppErrorStatus{}(AppErrorKind::INTERNAL_ERROR) == Status::INTERNAL_SERVER_ERROR);
        expect(AppErrorStatus{}(AppErrorKind::RATE_LIMITED) == Status::TOO_MANY_REQUESTS);
        expect(AppErrorStatus{}(AppErrorKind::TIMEOUT) == Status::GATEWAY_TIMEOUT);
    };
};

} // namespace core::error::tests
#endif

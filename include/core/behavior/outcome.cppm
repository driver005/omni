export module core_behavior:outcome;

import std;
import core_serde;
import interfaces;

export namespace core::behavior {

/**
 * @brief Expected-shaped runtime result that knows how to publish itself to a ResponseSink.
 *
 * Error payloads go through AppError::send(sink); success values must expose
 * `to_response(sink)` (the standard serde::Serializer<T> surface).
 */
template<class T>
class Outcome : public std::expected<T, core::error::AppError>
{
public:
    using std::expected<T, core::error::AppError>::expected;

    Outcome(std::expected<T, core::error::AppError>&& value) :
        std::expected<T, core::error::AppError>{std::move(value)}
    {
    }

    Outcome(const std::expected<T, core::error::AppError>& value) :
        std::expected<T, core::error::AppError>{value}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!*this) {
            this->error().send(sink);
            return;
        }
        this->value().to_response(sink);
    }
};

/** Plain serialized string body. */
template<>
class Outcome<std::string> : public std::expected<std::string, core::error::AppError>
{
public:
    using std::expected<std::string, core::error::AppError>::expected;

    Outcome(std::expected<std::string, core::error::AppError>&& value) :
        std::expected<std::string, core::error::AppError>{std::move(value)}
    {
    }

    Outcome(const std::expected<std::string, core::error::AppError>& value) :
        std::expected<std::string, core::error::AppError>{value}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!this->has_value()) {
            this->error().send(sink);
            return;
        }
        std::vector<std::byte> bytes{};
        bytes.reserve(this->value().size());
        for (const char ch: this->value()) {
            bytes.push_back(static_cast<std::byte>(ch));
        }
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(std::move(bytes));
        sink.send();
    }
};

/** Vector of reflected DTOs — serialize as a top-level JSON array. */
template<class T>
class Outcome<std::vector<T>> : public std::expected<std::vector<T>, core::error::AppError>
{
public:
    using std::expected<std::vector<T>, core::error::AppError>::expected;

    Outcome(std::expected<std::vector<T>, core::error::AppError>&& value) :
        std::expected<std::vector<T>, core::error::AppError>{std::move(value)}
    {
    }

    Outcome(const std::expected<std::vector<T>, core::error::AppError>& value) :
        std::expected<std::vector<T>, core::error::AppError>{value}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!this->has_value()) {
            this->error().send(sink);
            return;
        }
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(serde::Ser::serialize(serde::json_content_type, this->value()));
        sink.send();
    }
};

/** Dynamic JSON document — no DTO serializer wrapper, encode the Value itself. */
template<>
class Outcome<serde::Value> : public std::expected<serde::Value, core::error::AppError>
{
public:
    using std::expected<serde::Value, core::error::AppError>::expected;

    Outcome(std::expected<serde::Value, core::error::AppError>&& value) :
        std::expected<serde::Value, core::error::AppError>{std::move(value)}
    {
    }

    Outcome(const std::expected<serde::Value, core::error::AppError>& value) :
        std::expected<serde::Value, core::error::AppError>{value}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(serde::Ser::serialize(serde::json_content_type, value()));
        sink.send();
    }
};

/** Empty success (204-shaped handlers used to emit 200 with an empty body). */
template<>
class Outcome<void> : public std::expected<void, core::error::AppError>
{
public:
    using std::expected<void, core::error::AppError>::expected;

    Outcome(std::expected<void, core::error::AppError>&& value) :
        std::expected<void, core::error::AppError>{std::move(value)}
    {
    }

    Outcome(const std::expected<void, core::error::AppError>& value) :
        std::expected<void, core::error::AppError>{value}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(std::vector<std::byte>{});
        sink.send();
    }
};

/** Dynamic document plus an optional Set-Cookie header value. */
template<class T>
class Outcome<std::pair<T, std::string>> :
    public std::expected<std::pair<T, std::string>, core::error::AppError>
{
public:
    using std::expected<std::pair<T, std::string>, core::error::AppError>::expected;

    Outcome(std::expected<std::pair<T, std::string>, core::error::AppError>&& value) :
        std::expected<std::pair<T, std::string>, core::error::AppError>{std::move(value)}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!this->has_value()) {
            this->error().send(sink);
            return;
        }
        if (!this->value().second.empty()) {
            sink.set_header(interfaces::io::types::Token::SET_COOKIE, this->value().second);
        }
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(serde::Ser::serialize(serde::json_content_type, this->value().first));
        sink.send();
    }
};

template<>
class Outcome<std::pair<serde::Value, std::string>> :
    public std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
{
public:
    using std::expected<std::pair<serde::Value, std::string>, core::error::AppError>::expected;

    Outcome(std::expected<std::pair<serde::Value, std::string>, core::error::AppError>&& value) :
        std::expected<std::pair<serde::Value, std::string>, core::error::AppError>{std::move(value)}
    {
    }

    Outcome(
        const std::expected<std::pair<serde::Value, std::string>, core::error::AppError>& value
    ) :
        std::expected<std::pair<serde::Value, std::string>, core::error::AppError>{value}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        const auto& [document, cookie] = value();
        if (!cookie.empty()) {
            sink.set_header(interfaces::io::types::Token::SET_COOKIE, cookie);
        }
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(serde::Ser::serialize(serde::json_content_type, document));
        sink.send();
    }
};

} // namespace core::behavior

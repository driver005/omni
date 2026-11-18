export module blog_handlers:emails_recipient_failures_get_dto;
import std;
export namespace blog::handlers::emails::recipient_failures_get::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

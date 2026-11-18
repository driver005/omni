export module blog_handlers:emails_analytics_get_dto;
import std;
export namespace blog::handlers::emails::analytics_get::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

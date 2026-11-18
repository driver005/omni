export module blog_handlers:config_browse_dto;
import std;
export namespace blog::handlers::config::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

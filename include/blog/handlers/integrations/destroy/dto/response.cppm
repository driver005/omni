export module blog_handlers:integrations_destroy_dto;
import std;
export namespace blog::handlers::integrations::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

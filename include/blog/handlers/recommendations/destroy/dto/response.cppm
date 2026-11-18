export module blog_handlers:recommendations_destroy_dto;
import std;
export namespace blog::handlers::recommendations::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

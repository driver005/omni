export module blog_handlers:pages_bulk_destroy_dto;
import std;
export namespace blog::handlers::pages::bulk_destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

export module blog_handlers:themes_destroy_dto;
import std;
export namespace blog::handlers::themes::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

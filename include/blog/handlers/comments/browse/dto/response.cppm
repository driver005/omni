export module blog_handlers:comments_browse_dto;
import std;
export namespace blog::handlers::comments::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

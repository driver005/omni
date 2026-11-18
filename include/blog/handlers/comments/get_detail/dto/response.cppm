export module blog_handlers:comments_get_detail_dto;
import std;
export namespace blog::handlers::comments::get_detail::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

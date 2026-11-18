export module blog_handlers:email_previews_get_detail_dto;
import std;
export namespace blog::handlers::email_previews::get_detail::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

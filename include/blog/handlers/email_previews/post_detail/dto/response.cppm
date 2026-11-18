export module blog_handlers:email_previews_post_detail_dto;
import std;
export namespace blog::handlers::email_previews::post_detail::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

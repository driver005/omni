export module blog_handlers:email_previews_post_detail_model;
import std;
export namespace blog::handlers::email_previews::post_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

export module blog_handlers:email_previews_get_detail_model;
import std;
export namespace blog::handlers::email_previews::get_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

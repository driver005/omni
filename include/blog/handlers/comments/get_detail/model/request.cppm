export module blog_handlers:comments_get_detail_model;
import std;
export namespace blog::handlers::comments::get_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

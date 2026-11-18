export module blog_handlers:slugs_get_detail_model;
import std;
export namespace blog::handlers::slugs::get_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

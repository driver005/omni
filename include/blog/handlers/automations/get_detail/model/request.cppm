export module blog_handlers:automations_get_detail_model;
import std;
export namespace blog::handlers::automations::get_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

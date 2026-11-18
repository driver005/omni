export module blog_handlers:pages_gift_links_get_model;
import std;
export namespace blog::handlers::pages::gift_links_get::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

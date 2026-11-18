export module blog_handlers:apps_all_collection_model;
import std;
export namespace blog::handlers::apps::all_collection::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

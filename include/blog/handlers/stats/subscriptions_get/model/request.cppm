export module blog_handlers:stats_subscriptions_get_model;
import std;
export namespace blog::handlers::stats::subscriptions_get::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}

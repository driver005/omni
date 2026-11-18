export module blog_handlers:incoming_recommendations_browse_dto;
import std;
export namespace blog::handlers::incoming_recommendations::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

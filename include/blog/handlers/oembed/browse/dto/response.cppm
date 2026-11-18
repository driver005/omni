export module blog_handlers:oembed_browse_dto;
import std;
export namespace blog::handlers::oembed::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}

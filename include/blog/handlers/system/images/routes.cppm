export module blog_handlers_routes:images_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

namespace blog::api {

namespace {

    /** @brief `upload.single('file')` + `upload.validation({type:'images'})` presence guard. */
    bool has_upload_file(std::string_view content_type, std::string_view body)
    {
        if (!content_type.starts_with("multipart/form-data")) {
            return false;
        }
        constexpr std::string_view name_marker{"name=\"file\""};
        constexpr std::string_view filename_marker{"filename=\""};
        auto pos = body.find(name_marker);
        while (pos != std::string_view::npos) {
            auto block_end = body.find("\r\n\r\n", pos);
            if (block_end == std::string_view::npos) {
                block_end = body.size();
            }
            const auto fn = body.find(filename_marker, pos);
            if (fn != std::string_view::npos && fn < block_end) {
                const auto begin = fn + filename_marker.size();
                const auto end = body.find('"', begin);
                if (end != std::string_view::npos && end < block_end && end > begin) {
                    return true;
                }
            }
            pos = body.find(name_marker, pos + name_marker.size());
        }
        return false;
    }

    /** @brief Local stand-in for Ghost's missing-file ValidationError (upload.js:285-292). */
    void require_image_upload(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        interfaces::NextFn&& next,
        std::function<void()> send
    )
    {
        if (!has_upload_file(request.get_content_type(), request.body_as_string())) {
            blog::handlers::middleware::ErrorEnvelope envelope;
            envelope.errors.push_back(
                blog::handlers::middleware::ErrorItem{
                    .message = "Please select an image.",
                    .property = "",
                    .type = "ValidationError"
                }
            );
            blog::handlers::middleware::write_error(
                response,
                interfaces::io::types::Status::UNPROCESSABLE_CONTENT,
                envelope,
                std::move(send)
            );
            return;
        }
        next(request, response, std::move(send));
    }

} // namespace

export class ImagesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost route chain: mw.authAdminApi -> upload.single('file') ->
        // upload.validation({type:'images'}). No content_type_middleware: it would
        // 415 the multipart/form-data body Ghost requires (images.js:11-18).
        Router{router, "/images"}
            .use(blog::handlers::middleware::bearer_middleware)
            .use(blog::handlers::middleware::require_auth)
            .use(require_image_upload)
            .add_route(
                Route{"/upload"}
                    .post(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.images_images_upload_post().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api

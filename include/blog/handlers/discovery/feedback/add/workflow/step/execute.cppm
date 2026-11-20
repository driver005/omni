export module blog_handlers:feedback_add_step;
import :domain_all;
import :feedback_read_by_id_step;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::feedback::add::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"feedback_add_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(
            in.body_as_string().empty() ? "{}" : in.body_as_string());
        if (!parsed) return invalid("Invalid feedback body", "body");
        const auto* root = std::get_if<serde::Value::Object>(&parsed->get());
        if (root == nullptr) return invalid("Invalid feedback body", "body");

        const auto* node = lookup(*root, "feedback");
        if (node == nullptr) {
            if (const auto* data = lookup(*root, "data")) {
                if (const auto* data_object = std::get_if<serde::Value::Object>(&data->get())) {
                    node = lookup(*data_object, "feedback");
                }
            }
        }
        const auto* array = node != nullptr
                                ? std::get_if<serde::Value::Array>(&node->get())
                                : nullptr;
        if (array == nullptr || array->empty()) {
            return invalid("feedback[0] is required", "feedback");
        }
        const auto* item = std::get_if<serde::Value::Object>(&(*array)[0].get());
        if (item == nullptr) return invalid("feedback[0] must be an object", "feedback");

        const auto* post_node = lookup(*item, "post_id");
        const auto* score_node = lookup(*item, "score");
        if (post_node == nullptr || scalar(*post_node).empty()) {
            return invalid("A post_id is required", "post_id");
        }
        if (score_node == nullptr) return invalid("A score is required", "score");
        const auto score_text = scalar(*score_node);
        if (score_text != "0" && score_text != "1") {
            // Ghost: "Invalid feedback score. Only 1 or 0 is currently allowed."
            return core::workflow::StepResponse<Output>{{{"status", "422"},
                {"body", error("Invalid feedback score. Only 1 or 0 is currently allowed.",
                               "Invalid feedback score. Only 1 or 0 is currently allowed.",
                               "score")}}};
        }
        const std::string post_id = scalar(*post_node);

        // Member identity comes from the members-app session (modelled by the
        // ambient member-id header, the same primitive used by the other
        // members-facing handlers). Ghost throws InternalServerError when the
        // authenticated member is absent.
        std::string member_id{in.find_header(kMemberIdHeader)};
        if (member_id.empty()) member_id = std::string{in.find_header("x-member-id")};
        if (member_id.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "500"},
                {"body", error("Member not found.", "Member not found.", "",
                               "InternalServerError")}}};
        }

        // Ghost: getPostById(postId) uses status:'all'; a missing post is 404.
        const auto post = ctx.store().find_by_resource("posts", post_id);
        if (!post) {
            return core::workflow::StepResponse<Output>{{{"status", "404"},
                {"body", error("Post not found.", "Post not found.", "", "NotFoundError")}}};
        }
        const auto canonical_post = text_field(*post, "id");
        const std::string stored_post_id =
            canonical_post.empty() ? post_id : canonical_post;

        const auto existing = blog::api::parse_array(ctx.store().json_for(
            kFeedBackResource, {{"filter[post_id]", stored_post_id},
                                {"filter[member_id]", member_id}}));

        std::string record;
        if (!existing.empty()) {
            record = blog::api::record_to_json(existing.front());
            // Ghost skips the save when the score is unchanged, preserving
            // updated_at, and still answers 201.
            if (text_field(record, "score") == score_text) {
                return core::workflow::StepResponse<Output>{{{"status", "201"},
                    {"body", wire(record)}}};
            }
            auto edited = replace_field(record, "score", score_text);
            edited = replace_field(edited, "updated_at", quote(now()));
            const auto id = text_field(record, "id");
            (void)ctx.store().edit(kFeedBackResource, id, edited);
            record = edited;
        } else {
            record = "{\"id\":" + quote(make_object_id()) +
                     ",\"score\":" + score_text +
                     ",\"member_id\":" + quote(member_id) +
                     ",\"post_id\":" + quote(stored_post_id) +
                     ",\"created_at\":" + quote(now()) +
                     ",\"updated_at\":" + quote(now()) + "}";
            (void)ctx.store().create(kFeedBackResource, record);
        }

        // Ghost's add op always answers 201, on create and on edit.
        return core::workflow::StepResponse<Output>{{{"status", "201"},
            {"body", wire(record)}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    static const serde::Value* lookup(const Object& object, std::string_view key)
    {
        for (const auto& [name, value] : object) if (name == key) return &value;
        return nullptr;
    }

    static std::string scalar(const serde::Value& value)
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) return *text;
        return support::unquote(core_serde::JsonCodec{}.encode(value));
    }

    static core::workflow::StepResponse<Output> invalid(std::string_view reason,
                                                        std::string_view property)
    {
        return core::workflow::StepResponse<Output>{{{"status", "422"},
            {"body", support::error("Validation error", reason, property)}}};
    }

    /** @brief Feedback model wire shape: {id, memberId, postId, score}. */
    static std::string wire(std::string_view record)
    {
        const auto id = support::field(record, "id");
        const auto member_id = support::field(record, "member_id");
        const auto post_id = support::field(record, "post_id");
        const auto score = support::field(record, "score");
        return "{\"id\":" + (id.empty() ? std::string{"null"} : id) +
               ",\"memberId\":" + (member_id.empty() ? std::string{"null"} : member_id) +
               ",\"postId\":" + (post_id.empty() ? std::string{"null"} : post_id) +
               ",\"score\":" + (score.empty() ? std::string{"0"} : score) + "}";
    }

    static std::string replace_field(std::string_view object, std::string_view key,
                                     std::string_view raw_value)
    {
        const auto pattern = std::string{"\""} + std::string{key} + "\":";
        const auto at = object.find(pattern);
        if (at == std::string_view::npos) return std::string{object};
        const auto value_start = at + pattern.size();
        const auto value_end = support::value_end(object, value_start);
        std::string out{object.substr(0, value_start)};
        out += raw_value;
        out += object.substr(value_end);
        return out;
    }

    /** @brief Ghost ObjectID: 24 lowercase hex characters. */
    static std::string make_object_id()
    {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> digit{0, 15};
        std::string out;
        out.reserve(24);
        for (int i = 0; i < 24; ++i) out.push_back("0123456789abcdef"[digit(rng)]);
        return out;
    }

    static std::string now()
    {
        const auto tp = std::chrono::system_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            tp.time_since_epoch()).count();
        const auto stamp = std::format("{:%Y-%m-%dT%H:%M:%S}",
                                       std::chrono::floor<std::chrono::seconds>(tp));
        return stamp + std::format(".{:03d}Z", ms % 1000);
    }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}

export module blog_handlers:mutation;

import std;
import blog_model;
import blog_context;

#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Ghost-compatible error envelope from the e2e-api snapshots. */
class ApiError
{
public:
    [[nodiscard]] std::string json(
        std::string_view message,
        std::string_view context,
        std::string_view help = ""
    ) const
    {
        return envelope(message, context, help, "", "");
    }

    /** @brief Validation error with Ghost's `property` and `type` fields. */
    [[nodiscard]] std::string validation(
        std::string_view message,
        std::string_view context,
        std::string_view property,
        std::string_view type = "ValidationError"
    ) const
    {
        return envelope(message, context, "", property, type);
    }

private:
    [[nodiscard]] std::string envelope(
        std::string_view message,
        std::string_view context,
        std::string_view help,
        std::string_view property,
        std::string_view type
    ) const
    {
        const std::string id = std::format("{:032x}", ++m_counter);
        return std::format(
            R"({{"errors":[{{"id":"{}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":{},"message":{},"property":{},"type":{}}}]}})",
            id,
            context.empty() ? "null" : JsonBody{}.quote(context),
            help.empty() ? "null" : JsonBody{}.quote(help),
            JsonBody{}.quote(message),
            property.empty() ? "null" : JsonBody{}.quote(property),
            type.empty() ? "null" : JsonBody{}.quote(type)
        );
    }

    mutable std::uint64_t m_counter{0};
};

/** @brief Ghost-shaped write responses for admin create/edit/delete endpoints. */
class MutationResult
{
public:
    MutationResult() = default;

    MutationResult(int status, std::string body, bool error = false) :
        m_status{status},
        m_body{std::move(body)},
        m_error{error}
    {
    }

    [[nodiscard]] int status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::string& body() const noexcept
    {
        return m_body;
    }

    [[nodiscard]] bool ok() const noexcept
    {
        return !m_error;
    }

private:
    int m_status{200};
    std::string m_body{"{}"};
    bool m_error{false};
};

/** @brief Real mutating operations for Admin API resources. */
class MutationStore
{
public:
    explicit MutationStore(ApiDataStore& store) :
        m_store{store}
    {
    }

    [[nodiscard]] MutationResult create(std::string_view resource, std::string_view body)
    {
        JsonBody parser;
        const auto fields = parser.parse(body);
        if ((resource == "posts" || resource == "pages" || resource == "tags") &&
            !fields.contains("title") && !fields.contains("name")) {
            return MutationResult{422,
                ApiError{}.validation(
                    "Validation error, cannot save post.",
                    "title is required",
                    "title"
                ), true};
        }
        if (resource == "posts") {
            return wrap("posts", m_store.create_entity<model::Post>(body));
        }
        if (resource == "pages") {
            return wrap("pages", m_store.create_entity<model::Page>(body));
        }
        if (resource == "tags") {
            return wrap("tags", m_store.create_entity<model::Tag>(body));
        }
        if (resource == "users") {
            return wrap("users", m_store.create_entity<model::User>(body));
        }
        if (resource == "members") {
            return wrap("members", m_store.create_entity<model::Member>(body));
        }
        if (resource == "newsletters") {
            return wrap("newsletters", m_store.create_entity<model::Newsletter>(body));
        }
        if (resource == "tiers") {
            return wrap("tiers", m_store.create_entity<model::Tier>(body));
        }
        if (resource == "comments") {
            return wrap("comments", m_store.create_entity<model::Comment>(body));
        }
        const std::string id = m_store.resources().create(resource, body);
        return MutationResult{
            201,
            std::format(R"({{"{}":[{{"id":"{}"}}]}})", resource, id)
        };
    }

    [[nodiscard]] MutationResult edit(
        std::string_view resource,
        std::string_view id,
        std::string_view body
    )
    {
        if (resource == "posts") {
            return wrap("posts", m_store.edit_entity<model::Post>(id, body));
        }
        if (resource == "pages") {
            return wrap("pages", m_store.edit_entity<model::Page>(id, body));
        }
        if (resource == "tags") {
            return wrap("tags", m_store.edit_entity<model::Tag>(id, body));
        }
        if (resource == "users") {
            return wrap("users", m_store.edit_entity<model::User>(id, body));
        }
        if (resource == "members") {
            return wrap("members", m_store.edit_entity<model::Member>(id, body));
        }
        if (m_store.resources().edit(resource, id, body)) {
            return MutationResult{200, std::format(R"({{"{}":[{{"id":"{}"}}]}})", resource, id)};
        }
        const std::string created = m_store.resources().create(resource, body);
        return MutationResult{200, std::format(R"({{"{}":[{{"id":"{}"}}]}})", resource, created)};
    }

    [[nodiscard]] MutationResult destroy(std::string_view resource, std::string_view id)
    {
        bool removed = false;
        if (resource == "posts") {
            removed = m_store.destroy_entity<model::Post>(id);
        } else if (resource == "pages") {
            removed = m_store.destroy_entity<model::Page>(id);
        } else if (resource == "tags") {
            removed = m_store.destroy_entity<model::Tag>(id);
        } else if (resource == "users") {
            removed = m_store.destroy_entity<model::User>(id);
        } else if (resource == "members") {
            removed = m_store.destroy_entity<model::Member>(id);
        }
        if (resource != "posts" && resource != "pages" && resource != "tags" &&
            resource != "users" && resource != "members" &&
            removed == false) {
            removed = m_store.resources().destroy(resource, id);
        }
        if (!removed) {
            return MutationResult{404, ApiError{}.json("Resource not found", "Not Found"), true};
        }
        return MutationResult{200, "{}"};
    }


    [[nodiscard]] MutationResult bulk_destroy(std::string_view resource, std::string_view body)
    {
        const auto count = m_store.resources().bulk_destroy(resource, body);
        return MutationResult{200, std::format(R"({{"meta":{{"deleted":{}}}}})", count)};
    }

    [[nodiscard]] MutationResult bulk_edit(std::string_view resource, std::string_view body)
    {
        const auto count = m_store.resources().bulk_edit(resource, body);
        return MutationResult{200, std::format(R"({{"meta":{{"updated":{}}}}})", count)};
    }

private:
    template<typename E>
    [[nodiscard]] MutationResult wrap(std::string_view resource, std::optional<E> value) const
    {
        if (!value) {
            return MutationResult{404, ApiError{}.json("Resource not found", "Not Found"), true};
        }
        const std::string singular{resource.substr(0, resource.size() - 1)};
        return MutationResult{
            201,
            std::format(R"({{"{}":[{}]}})", resource, EntityJson<E>{}.to_json(*value))
        };
    }

    template<typename E>
    [[nodiscard]] MutationResult wrap(std::string_view resource, E value) const
    {
        return wrap<E>(resource, std::optional<E>{std::move(value)});
    }

    ApiDataStore& m_store;
};

} // namespace blog::api

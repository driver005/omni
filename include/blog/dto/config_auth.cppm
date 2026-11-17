module;
export module blog_dto:config_auth;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: ConfigAuth. */
class ConfigAuth
{
public:
    void set_engine(std::string value) { engine = std::move(value); }
    [[nodiscard]] const std::string& get_engine() const noexcept { return engine; }

    std::string engine{"legacy"};
};

} // namespace blog::dto

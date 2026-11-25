module;
#include <rfl/to_generic.hpp>

export module app_handlers_routing:result;

import std;
import core_serde;

export namespace app::handlers::routing {

/** @brief Builds typed dynamic response documents; no hand-written JSON strings. */
class ResponseDocument
{
public:
    [[nodiscard]] static serde::Value empty() noexcept
    {
        return serde::Value{serde::Value::Object{}};
    }

    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

// Each Result carries a typed serde::Value document (rfl::Generic) built from
// reflected response values via ResponseDocument. The handler writes it with
// serde::Ser::serialize, so no pre-encoded JSON text crosses the boundary.

class RoutingEntryResult
{
public:
    RoutingEntryResult() = default;

    explicit RoutingEntryResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingCreateConfigResult
{
public:
    RoutingCreateConfigResult() = default;

    explicit RoutingCreateConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingLinkConfigResult
{
public:
    RoutingLinkConfigResult() = default;

    explicit RoutingLinkConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingRetrieveConfigResult
{
public:
    RoutingRetrieveConfigResult() = default;

    explicit RoutingRetrieveConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class ListRoutingConfigsResult
{
public:
    ListRoutingConfigsResult() = default;

    explicit ListRoutingConfigsResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class ListRoutingConfigsForProfileResult
{
public:
    ListRoutingConfigsForProfileResult() = default;

    explicit ListRoutingConfigsForProfileResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingUnlinkConfigResult
{
public:
    RoutingUnlinkConfigResult() = default;

    explicit RoutingUnlinkConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingUpdateDefaultConfigResult
{
public:
    RoutingUpdateDefaultConfigResult() = default;

    explicit RoutingUpdateDefaultConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingRetrieveDefaultConfigResult
{
public:
    RoutingRetrieveDefaultConfigResult() = default;

    explicit RoutingRetrieveDefaultConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class UpsertSurchargeDecisionManagerConfigResult
{
public:
    UpsertSurchargeDecisionManagerConfigResult() = default;

    explicit UpsertSurchargeDecisionManagerConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class DeleteSurchargeDecisionManagerConfigResult
{
public:
    DeleteSurchargeDecisionManagerConfigResult() = default;

    explicit DeleteSurchargeDecisionManagerConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RetrieveSurchargeDecisionManagerConfigResult
{
public:
    RetrieveSurchargeDecisionManagerConfigResult() = default;

    explicit RetrieveSurchargeDecisionManagerConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class UpsertDecisionManagerConfigResult
{
public:
    UpsertDecisionManagerConfigResult() = default;

    explicit UpsertDecisionManagerConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class DeleteDecisionManagerConfigResult
{
public:
    DeleteDecisionManagerConfigResult() = default;

    explicit DeleteDecisionManagerConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RetrieveDecisionManagerConfigResult
{
public:
    RetrieveDecisionManagerConfigResult() = default;

    explicit RetrieveDecisionManagerConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingRetrieveLinkedConfigResult
{
public:
    RoutingRetrieveLinkedConfigResult() = default;

    explicit RoutingRetrieveLinkedConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingRetrieveDefaultConfigForProfilesResult
{
public:
    RoutingRetrieveDefaultConfigForProfilesResult() = default;

    explicit RoutingRetrieveDefaultConfigForProfilesResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingUpdateDefaultConfigForProfileResult
{
public:
    RoutingUpdateDefaultConfigForProfileResult() = default;

    explicit RoutingUpdateDefaultConfigForProfileResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class CreateSuccessBasedRoutingResult
{
public:
    CreateSuccessBasedRoutingResult() = default;

    explicit CreateSuccessBasedRoutingResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class CreateEliminationRoutingResult
{
public:
    CreateEliminationRoutingResult() = default;

    explicit CreateEliminationRoutingResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class ContractBasedRoutingSetupConfigResult
{
public:
    ContractBasedRoutingSetupConfigResult() = default;

    explicit ContractBasedRoutingSetupConfigResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class SetDynamicRoutingVolumeSplitResult
{
public:
    SetDynamicRoutingVolumeSplitResult() = default;

    explicit SetDynamicRoutingVolumeSplitResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class GetDynamicRoutingVolumeSplitResult
{
public:
    GetDynamicRoutingVolumeSplitResult() = default;

    explicit GetDynamicRoutingVolumeSplitResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class RoutingMigrationStatusResult
{
public:
    RoutingMigrationStatusResult() = default;

    explicit RoutingMigrationStatusResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class MigrateRoutingRulesResult
{
public:
    MigrateRoutingRulesResult() = default;

    explicit MigrateRoutingRulesResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

} // namespace app::handlers::routing

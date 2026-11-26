export module app_user:customers_result;

import std;
import core_serde;

export namespace app::handlers::customers {

/**
 * @brief Typed result base for customers responses.
 *
 * Each runtime builds a `serde::Value` (rfl::Generic) document from typed field
 * assignments; the handler serializes that document with `Ser::serialize`. No
 * response carries hand-written JSON text.
 */
class CustomersDocumentResult
{
public:
    CustomersDocumentResult() = default;

    explicit CustomersDocumentResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value document)
    {
        m_document = std::move(document);
    }

private:
    serde::Value m_document{};
};

class CustomersCreateResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersCreateV2Result : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersRetrieveResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersRetrieveV2Result : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersRetrieveByMerchantReferenceIdResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersListResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersListV2Result : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersListWithCountResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersListWithCountV2Result : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersUpdateResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersUpdateV2Result : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersDeleteResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class CustomersDeleteV2Result : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class GetCustomerMandatesResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

class MigrateGlobalIdResult : public CustomersDocumentResult
{
public:
    using CustomersDocumentResult::CustomersDocumentResult;
};

} // namespace app::handlers::customers

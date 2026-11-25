module;
export module app_store:subscriptions;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::subscriptions {

/** @brief Persisted subscription mirror of the upstream Subscription api model. */
class SubscriptionRecord
{
public:
    SubscriptionRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_subscription_id(std::string value)
    {
        m_subscription_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_subscription_id() const noexcept
    {
        return m_subscription_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_plan_id(std::string value)
    {
        m_plan_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_plan_id() const noexcept
    {
        return m_plan_id;
    }

    void set_item_price_id(std::string value)
    {
        m_item_price_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_price_id() const noexcept
    {
        return m_item_price_id;
    }

    void set_coupon(std::string value)
    {
        m_coupon = std::move(value);
    }

    [[nodiscard]] const std::string& get_coupon() const noexcept
    {
        return m_coupon;
    }

    void set_merchant_reference_id(std::string value)
    {
        m_merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_reference_id() const noexcept
    {
        return m_merchant_reference_id;
    }

    void set_billing_processor_subscription_id(std::string value)
    {
        m_billing_processor_subscription_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_billing_processor_subscription_id() const noexcept
    {
        return m_billing_processor_subscription_id;
    }

    void set_client_secret(std::string value)
    {
        m_client_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_secret() const noexcept
    {
        return m_client_secret;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::string value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    std::string m_subscription_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_customer_id{};
    std::string m_status{};
    std::string m_plan_id{};
    std::string m_item_price_id{};
    std::string m_coupon{};
    std::string m_merchant_reference_id{};
    std::string m_billing_processor_subscription_id{};
    std::string m_client_secret{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::store::subscriptions

template<>
struct serde::Serializable<app::store::subscriptions::SubscriptionRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::subscriptions::SubscriptionRecord::get_id,
                &app::store::subscriptions::SubscriptionRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "subscription_id",
                &app::store::subscriptions::SubscriptionRecord::get_subscription_id,
                &app::store::subscriptions::SubscriptionRecord::set_subscription_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::subscriptions::SubscriptionRecord::get_merchant_id,
                &app::store::subscriptions::SubscriptionRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::subscriptions::SubscriptionRecord::get_profile_id,
                &app::store::subscriptions::SubscriptionRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::store::subscriptions::SubscriptionRecord::get_customer_id,
                &app::store::subscriptions::SubscriptionRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::subscriptions::SubscriptionRecord::get_status,
                &app::store::subscriptions::SubscriptionRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plan_id",
                &app::store::subscriptions::SubscriptionRecord::get_plan_id,
                &app::store::subscriptions::SubscriptionRecord::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::store::subscriptions::SubscriptionRecord::get_item_price_id,
                &app::store::subscriptions::SubscriptionRecord::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon",
                &app::store::subscriptions::SubscriptionRecord::get_coupon,
                &app::store::subscriptions::SubscriptionRecord::set_coupon,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::store::subscriptions::SubscriptionRecord::get_merchant_reference_id,
                &app::store::subscriptions::SubscriptionRecord::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_processor_subscription_id",
                &app::store::subscriptions::SubscriptionRecord::
                    get_billing_processor_subscription_id,
                &app::store::subscriptions::SubscriptionRecord::
                    set_billing_processor_subscription_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::store::subscriptions::SubscriptionRecord::get_client_secret,
                &app::store::subscriptions::SubscriptionRecord::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::subscriptions::SubscriptionRecord::get_created_at,
                &app::store::subscriptions::SubscriptionRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::store::subscriptions::SubscriptionRecord::get_updated_at,
                &app::store::subscriptions::SubscriptionRecord::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "subscriptions";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::subscriptions::tests {

using namespace boost::ut;

suite<"app_store_subscription_record"> subscription_record_suite = []
{
    "stores subscription fields and reports its table"_test = []
    {
        SubscriptionRecord record{};
        record.set_subscription_id("sub_test");
        record.set_merchant_id("merchant_test");
        record.set_status("active");

        expect(record.get_subscription_id() == "sub_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_status() == "active");
        expect(serde::Serializable<SubscriptionRecord>::table_name() == "subscriptions");
    };
};

} // namespace app::store::subscriptions::tests
#endif

export module app_handlers_payment_methods:runtime_modular_migrate_payment_methods;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::payment_methods {

class ModularMigratePaymentMethodsRuntime
{
public:
    explicit ModularMigratePaymentMethodsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ModularMigratePaymentMethodsResult, core::error::AppError>
    modular_migrate_payment_methods(const ModularMigratePaymentMethodsInput& input) const
    {
        m_context.metrics().record_flow("modular_migrate_payment_methods");
        m_context.metrics().counter("PAYMENT_METHOD_MODULAR_MIGRATE_PAYMENT_METHODS").increment();
        app::model::payment_methods::ModularPaymentMethodMigrationResponse response{};
        response.set_total_rows(input.rows.size());
        std::vector<app::model::payment_methods::ModularPaymentMethodMigrationRowResult> results{};
        std::size_t successful{0};
        auto repository = m_context.repository();
        for (std::size_t i = 0; i < input.rows.size(); ++i) {
            const auto& source = input.rows[i];
            app::model::payment_methods::ModularPaymentMethodMigrationRowResult row{};
            row.set_row_number(i + 1);
            const auto merchant_id = source.get_merchant_id().get_value();
            app::model::payment_methods::MerchantIdValue merchant{};
            merchant.set_value(merchant_id);
            row.set_merchant_id(std::move(merchant));
            std::string last4;
            std::string network;
            if (source.get_card()) {
                const auto& number = source.get_card()->get_card_number();
                if (number.size() >= 4) {
                    last4 = number.substr(number.size() - 4);
                }
                if (source.get_card()->get_card_network()) {
                    network = source.get_card()->get_card_network()->get_value();
                }
            }
            const auto fingerprint = m_support.merchant_fingerprint(merchant_id, last4, network);
            row.set_old_fingerprint_id(fingerprint.substr(0, 16));
            const auto duplicate =
                m_context.cache().get("payment_methods.fingerprint:" + fingerprint);
            if (duplicate.hit) {
                const auto existing = repository.find_payment_method_by_string(
                    core::cache::value_as<std::string>(duplicate.value)
                );
                if (existing) {
                    row.set_payment_method_id(existing->get_payment_method_id());
                    row.set_new_fingerprint_id(existing->get_vault_id());
                    row.set_migration_status(app::model::payment_methods::MigrationStatus::Success);
                    ++successful;
                    results.push_back(std::move(row));
                    continue;
                }
            }
            app::store::payment_methods::PaymentMethodRecord record{};
            record.set_payment_method_id(
                m_support.global_id("pm_", merchant_id + std::to_string(i))
            );
            record.set_merchant_id(merchant_id);
            if (source.get_customer_id()) {
                record.set_customer_id(source.get_customer_id()->get_value());
            }
            record.set_type(
                source.get_payment_method() ? source.get_payment_method()->get_value()
                                            : std::string{"card"}
            );
            record.set_network(network);
            record.set_last4(last4);
            record.set_vault_id(fingerprint.substr(0, 16));
            record.set_status("active");
            record.set_created_at(m_support.now_value());
            const auto stored = repository.create_payment_method(std::move(record));
            if (!stored) {
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Failed);
                row.set_error_message("persistence failed");
            } else {
                row.set_payment_method_id(stored->get_payment_method_id());
                row.set_new_fingerprint_id(fingerprint.substr(0, 16));
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Success);
                m_context.cache().put(
                    "payment_methods.fingerprint:" + fingerprint,
                    core::cache::to_cache_value(stored->get_payment_method_id())
                );
                ++successful;
            }
            results.push_back(std::move(row));
        }
        response.set_successful_count(successful);
        response.set_failed_count(input.rows.size() - successful);
        response.set_results(std::move(results));
        return ModularMigratePaymentMethodsResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods

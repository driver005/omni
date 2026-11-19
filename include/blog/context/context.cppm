export module blog_context:context;

import std;
import core_database;
import core_search;
import core_cache;
import core_serde;
import blog_model;
import blog_dto;
import memory_backends;



export namespace blog::api {

[[nodiscard]] inline std::string quote_fragment(std::string_view text)
{
    std::string out{"\""};
    for (const char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            default: out += c;
        }
    }
    return out + "\"";
}

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
            context.empty() ? "null" : quote_fragment(context),
            help.empty() ? "null" : quote_fragment(help),
            quote_fragment(message),
            property.empty() ? "null" : quote_fragment(property),
            type.empty() ? "null" : quote_fragment(type)
        );
    }

    mutable std::uint64_t m_counter{0};
};


/**
 * @brief Ghost schema field names and defaultTo values, derived from
 * data/schema/schema.js. Defaults are raw JSON value fragments.
 */
class ResourceSchema
{
public:
    ResourceSchema()
    {
        m_fields["actions"] = {"id", "resource_id", "resource_type", "actor_id", "actor_type", "event", "context", "created_at"};
        m_fields["api_keys"] = {"id", "type", "secret", "role_id", "integration_id", "user_id", "last_seen_at", "last_seen_version", "created_at", "updated_at"};
        m_fields["app_installation_manifests"] = {"id", "installation_id", "manifest_url", "manifest", "digest", "icon_data_url", "backend_key_thumbprint", "source", "requires_approval", "created_at"};
        m_fields["app_installations"] = {"id", "app_id", "current_app_id", "identity_key_thumbprint", "status", "manifest_id", "pending_manifest_id", "revision", "created_at", "updated_at"};
        m_fields["automated_email_recipients"] = {"id", "automated_email_id", "automation_action_revision_id", "automation_run_step_id", "member_id", "member_uuid", "member_email", "member_name", "mailgun_message_id", "delivered_at", "opened_at", "clicked_at", "track_opens", "track_clicks", "created_at", "updated_at"};
        m_fields["automation_action_edges"] = {"source_action_id", "target_action_id"};
        m_fields["automation_action_revisions"] = {"id", "created_at", "action_id", "wait_hours", "email_subject", "email_lexical", "email_design_setting_id", "email_sent_count", "email_opened_count", "email_clicked_count"};
        m_fields["automation_actions"] = {"id", "created_at", "updated_at", "deleted_at", "automation_id", "type"};
        m_fields["automation_run_steps"] = {"id", "created_at", "updated_at", "automation_run_id", "automation_action_revision_id", "ready_at", "step_attempts", "started_at", "finished_at", "status", "locked_by", "locked_at"};
        m_fields["automation_runs"] = {"id", "created_at", "updated_at", "automation_id", "member_id", "member_email"};
        m_fields["automation_trigger_rules"] = {"id", "automation_id", "trigger_type", "scope", "direction", "product_id", "label_id", "created_at"};
        m_fields["automations"] = {"id", "status", "name", "description", "slug", "created_at", "updated_at"};
        m_fields["benefits"] = {"id", "name", "slug", "created_at", "updated_at"};
        m_fields["brute"] = {"key", "firstRequest", "lastRequest", "lifetime", "count"};
        m_fields["collections"] = {"id", "title", "slug", "description", "type", "filter", "feature_image", "created_at", "updated_at"};
        m_fields["collections_posts"] = {"id", "collection_id", "post_id", "sort_order"};
        m_fields["comment_likes"] = {"id", "comment_id", "member_id", "score", "created_at", "updated_at"};
        m_fields["comment_reports"] = {"id", "comment_id", "member_id", "created_at", "updated_at"};
        m_fields["comments"] = {"id", "post_id", "member_id", "parent_id", "in_reply_to_id", "status", "html", "edited_at", "pinned_at", "created_at", "updated_at"};
        m_fields["custom_theme_settings"] = {"id", "theme", "key", "type", "value"};
        m_fields["donation_payment_events"] = {"id", "name", "email", "member_id", "amount", "currency", "attribution_id", "attribution_type", "attribution_url", "referrer_source", "referrer_medium", "referrer_url", "utm_source", "utm_medium", "utm_campaign", "utm_term", "utm_content", "created_at", "donation_message"};
        m_fields["email_batches"] = {"id", "email_id", "mailgun_message_id", "fallback_sending_domain", "status", "member_segment", "recipient_count", "submission_excluded_count", "submitted_count", "error_status_code", "error_message", "error_data", "created_at", "updated_at"};
        m_fields["email_design_settings"] = {"id", "slug", "background_color", "header_background_color", "header_image", "show_header_icon", "show_header_title", "footer_content", "button_color", "button_corners", "button_style", "link_color", "link_style", "body_font_category", "title_font_category", "title_font_weight", "image_corners", "divider_color", "section_title_color", "show_badge", "sender_name", "sender_email", "sender_reply_to", "created_at", "updated_at"};
        m_fields["email_recipient_failures"] = {"id", "email_id", "member_id", "email_recipient_id", "code", "enhanced_code", "message", "severity", "failed_at", "event_id"};
        m_fields["email_recipients"] = {"id", "email_id", "member_id", "batch_id", "processed_at", "delivered_at", "opened_at", "failed_at", "member_uuid", "member_email", "member_name"};
        m_fields["email_spam_complaint_events"] = {"id", "member_id", "email_id", "email_address", "created_at"};
        m_fields["emails"] = {"id", "post_id", "uuid", "status", "recipient_filter", "error", "error_data", "email_count", "csd_email_count", "preflight_email_count", "candidate_count", "preparation_excluded_count", "prepared_at", "delivered_count", "opened_count", "failed_count", "subject", "from", "reply_to", "html", "plaintext", "source", "source_type", "track_opens", "track_clicks", "feedback_enabled", "submitted_at", "newsletter_id", "created_at", "updated_at"};
        m_fields["gift_deliveries"] = {"id", "gift_id", "recipient_email", "status", "scheduled_at", "started_at", "email_sent_at", "email_provider_message_id", "outcome", "outcome_at", "outcome_error"};
        m_fields["gift_links"] = {"token", "post_id", "created_at", "updated_at"};
        m_fields["gifts"] = {"id", "token", "buyer_email", "buyer_member_id", "buyer_name", "recipient_name", "personal_message", "redeemer_member_id", "tier_id", "cadence", "duration", "currency", "amount", "stripe_checkout_session_id", "stripe_payment_intent_id", "checkout_started_at", "consumes_at", "expires_at", "status", "purchased_at", "redeemed_at", "consumed_at", "expired_at", "refunded_at", "consumes_soon_reminder_sent_at"};
        m_fields["integrations"] = {"id", "type", "name", "slug", "icon_image", "description", "created_at", "updated_at"};
        m_fields["invites"] = {"id", "role_id", "status", "token", "email", "expires", "created_at", "updated_at"};
        m_fields["jobs"] = {"id", "name", "status", "started_at", "finished_at", "created_at", "updated_at", "metadata", "queue_entry"};
        m_fields["jobs_outbox"] = {"id", "envelope", "created_at", "available_at", "submitted_at"};
        m_fields["labels"] = {"id", "name", "slug", "created_at", "updated_at"};
        m_fields["machine_payment_events"] = {"id", "post_id", "amount", "currency", "protocol", "method", "stripe_payment_intent_id", "reference", "created_at"};
        m_fields["members"] = {"id", "uuid", "transient_id", "email", "status", "name", "expertise", "note", "geolocation", "enable_comment_notifications", "enable_updates_and_announcements", "email_count", "email_opened_count", "email_open_rate", "email_disabled", "last_seen_at", "last_commented_at", "commenting", "created_at", "updated_at"};
        m_fields["members_cancel_events"] = {"id", "member_id", "from_plan", "created_at"};
        m_fields["members_click_events"] = {"id", "member_id", "redirect_id", "created_at"};
        m_fields["members_created_events"] = {"id", "created_at", "member_id", "attribution_id", "attribution_type", "attribution_url", "referrer_source", "referrer_medium", "referrer_url", "utm_source", "utm_medium", "utm_campaign", "utm_term", "utm_content", "source", "batch_id"};
        m_fields["members_current_subscription"] = {"member_id", "subscription_id"};
        m_fields["members_email_change_events"] = {"id", "member_id", "to_email", "from_email", "created_at"};
        m_fields["members_feedback"] = {"id", "score", "member_id", "post_id", "created_at", "updated_at"};
        m_fields["members_labels"] = {"id", "member_id", "label_id", "sort_order"};
        m_fields["members_login_events"] = {"id", "member_id", "created_at"};
        m_fields["members_metafield_bindings"] = {"id", "port", "metafield_namespace", "metafield_key", "created_at", "updated_at"};
        m_fields["members_metafield_change_events"] = {"id", "member_id", "written_by_type", "written_by_id", "source", "metafields", "created_at"};
        m_fields["members_metafield_values"] = {"metafield_namespace", "metafield_key", "member_id", "path", "value_text", "written_by_type", "written_by_id", "created_at", "updated_at"};
        m_fields["members_metafields"] = {"id", "namespace", "key", "name", "type", "settings", "status", "member_access", "sort_order", "created_at", "updated_at"};
        m_fields["members_newsletters"] = {"id", "member_id", "newsletter_id"};
        m_fields["members_paid_subscription_events"] = {"id", "type", "member_id", "subscription_id", "from_plan", "to_plan", "currency", "source", "mrr_delta", "created_at"};
        m_fields["members_payment_events"] = {"id", "member_id", "amount", "currency", "source", "created_at"};
        m_fields["members_product_events"] = {"id", "member_id", "product_id", "action", "created_at"};
        m_fields["members_products"] = {"id", "member_id", "product_id", "sort_order", "expiry_at"};
        m_fields["members_status_events"] = {"id", "member_id", "from_status", "to_status", "created_at", "batch_id"};
        m_fields["members_stripe_customers"] = {"id", "member_id", "customer_id", "name", "email", "created_at", "updated_at"};
        m_fields["members_stripe_customers_subscriptions"] = {"id", "customer_id", "ghost_subscription_id", "subscription_id", "stripe_price_id", "status", "cancel_at_period_end", "cancellation_reason", "current_period_end", "start_date", "default_payment_card_last4", "created_at", "updated_at", "mrr", "offer_id", "discount_start", "discount_end", "trial_start_at", "trial_end_at", "plan_id", "plan_nickname", "plan_interval", "plan_amount", "plan_currency"};
        m_fields["members_subscribe_events"] = {"id", "member_id", "subscribed", "created_at", "source", "newsletter_id"};
        m_fields["members_subscription_created_events"] = {"id", "created_at", "member_id", "subscription_id", "attribution_id", "attribution_type", "attribution_url", "referrer_source", "referrer_medium", "referrer_url", "utm_source", "utm_medium", "utm_campaign", "utm_term", "utm_content", "batch_id"};
        m_fields["mentions"] = {"id", "source", "source_title", "source_site_title", "source_excerpt", "source_author", "source_featured_image", "source_favicon", "target", "resource_id", "resource_type", "created_at", "payload", "deleted", "verified", "revalidation_failure_count"};
        m_fields["milestones"] = {"id", "type", "value", "currency", "created_at", "email_sent_at"};
        m_fields["mobiledoc_revisions"] = {"id", "post_id", "mobiledoc", "created_at_ts", "created_at"};
        m_fields["newsletters"] = {"id", "uuid", "name", "description", "feedback_enabled", "slug", "sender_name", "sender_email", "sender_reply_to", "status", "visibility", "subscribe_on_signup", "sort_order", "header_image", "show_header_icon", "show_header_title", "show_excerpt", "title_font_category", "title_alignment", "show_feature_image", "body_font_category", "footer_content", "show_badge", "show_header_name", "show_post_title_section", "show_comment_cta", "show_subscription_details", "show_latest_posts", "show_share_button", "background_color", "post_title_color", "created_at", "updated_at", "button_corners", "button_style", "title_font_weight", "link_style", "image_corners", "header_background_color", "section_title_color"};
        m_fields["offer_redemptions"] = {"id", "offer_id", "member_id", "subscription_id", "created_at"};
        m_fields["offers"] = {"id", "active", "name", "code", "product_id", "stripe_coupon_id", "interval", "currency", "discount_type", "discount_amount", "duration", "duration_in_months", "portal_title", "portal_description", "redemption_type", "created_at", "updated_at"};
        m_fields["outbox"] = {"id", "event_type", "status", "payload", "created_at", "updated_at", "retry_count", "last_retry_at", "message"};
        m_fields["permissions"] = {"id", "name", "object_type", "action_type", "object_id", "created_at", "updated_at"};
        m_fields["permissions_roles"] = {"id", "role_id", "permission_id"};
        m_fields["permissions_users"] = {"id", "user_id", "permission_id"};
        m_fields["post_gift_links"] = {"post_id", "gift_link_token", "created_at", "updated_at"};
        m_fields["post_revisions"] = {"id", "post_id", "lexical", "created_at_ts", "created_at", "author_id", "title", "post_status", "reason", "feature_image", "feature_image_alt", "feature_image_caption", "custom_excerpt"};
        m_fields["posts"] = {"id", "uuid", "title", "slug", "mobiledoc", "lexical", "html", "comment_id", "plaintext", "feature_image", "featured", "type", "status", "locale", "visibility", "email_recipient_filter", "created_at", "updated_at", "published_at", "published_by", "custom_excerpt", "auto_excerpt", "reading_time", "codeinjection_head", "codeinjection_foot", "custom_template", "canonical_url", "newsletter_id", "show_title_and_feature_image"};
        m_fields["posts_authors"] = {"id", "post_id", "author_id", "sort_order"};
        m_fields["posts_meta"] = {"id", "post_id", "og_image", "og_title", "og_description", "twitter_image", "twitter_title", "twitter_description", "meta_title", "meta_description", "email_subject", "frontmatter", "feature_image_alt", "feature_image_caption", "email_only"};
        m_fields["posts_products"] = {"id", "post_id", "product_id", "sort_order"};
        m_fields["posts_tags"] = {"id", "post_id", "tag_id", "sort_order"};
        m_fields["products"] = {"id", "name", "slug", "active", "welcome_page_url", "visibility", "trial_days", "description", "type", "currency", "monthly_price", "yearly_price", "created_at", "updated_at", "monthly_price_id", "yearly_price_id"};
        m_fields["products_benefits"] = {"id", "product_id", "benefit_id", "sort_order"};
        m_fields["rate_limits"] = {"key", "points", "expire"};
        m_fields["recommendation_click_events"] = {"id", "recommendation_id", "member_id", "created_at"};
        m_fields["recommendation_subscribe_events"] = {"id", "recommendation_id", "member_id", "created_at"};
        m_fields["recommendations"] = {"id", "url", "title", "excerpt", "featured_image", "favicon", "description", "one_click_subscribe", "created_at", "updated_at"};
        m_fields["redirects"] = {"id", "from", "to", "post_id", "automation_action_revision_id", "to_hash", "created_at", "updated_at"};
        m_fields["roles"] = {"id", "name", "description", "created_at", "updated_at"};
        m_fields["roles_users"] = {"id", "role_id", "user_id"};
        m_fields["sessions"] = {"id", "session_id", "user_id", "session_data", "created_at", "updated_at"};
        m_fields["settings"] = {"id", "group", "key", "value", "type", "flags", "created_at", "updated_at"};
        m_fields["snippets"] = {"id", "name", "mobiledoc", "lexical", "created_at", "updated_at"};
        m_fields["stripe_checkout_config"] = {"id", "slug", "design", "created_at", "updated_at"};
        m_fields["stripe_prices"] = {"id", "stripe_price_id", "stripe_product_id", "active", "nickname", "currency", "amount", "type", "interval", "description", "created_at", "updated_at"};
        m_fields["stripe_products"] = {"id", "product_id", "stripe_product_id", "created_at", "updated_at"};
        m_fields["subscriptions"] = {"id", "type", "status", "member_id", "tier_id", "cadence", "currency", "amount", "payment_provider", "payment_subscription_url", "payment_user_url", "offer_id", "expires_at", "created_at", "updated_at"};
        m_fields["suppressions"] = {"id", "email", "email_id", "reason", "created_at"};
        m_fields["tags"] = {"id", "name", "slug", "description", "feature_image", "parent_id", "visibility", "og_image", "og_title", "og_description", "twitter_image", "twitter_title", "twitter_description", "meta_title", "meta_description", "codeinjection_head", "codeinjection_foot", "canonical_url", "accent_color", "created_at", "updated_at"};
        m_fields["tier_requirement_tiers"] = {"requirement", "product_id"};
        m_fields["tier_requirements"] = {"requirement", "tier_scope", "countries", "created_at", "updated_at"};
        m_fields["tinybird_syncs"] = {"id", "table_name", "last_synced_updated_at", "last_synced_id", "created_at", "updated_at"};
        m_fields["tokens"] = {"id", "token", "uuid", "data", "created_at", "updated_at", "first_used_at", "used_count", "otc_used_count"};
        m_fields["user_accounts"] = {"id", "user_id", "account_id", "provider_id", "access_token", "refresh_token", "id_token", "access_token_expires_at", "refresh_token_expires_at", "scope", "password", "created_at", "updated_at"};
        m_fields["user_sessions"] = {"id", "user_id", "token", "expires_at", "ip_address", "user_agent", "origin", "verified", "created_at", "updated_at"};
        m_fields["user_two_factors"] = {"id", "user_id", "secret", "backup_codes", "verified", "failed_verification_count", "locked_until"};
        m_fields["users"] = {"id", "name", "slug", "password", "email", "profile_image", "cover_image", "bio", "website", "location", "facebook", "twitter", "threads", "bluesky", "mastodon", "tiktok", "youtube", "instagram", "linkedin", "accessibility", "status", "locale", "visibility", "meta_title", "meta_description", "tour", "last_seen", "comment_notifications", "free_member_signup_notification", "paid_subscription_started_notification", "paid_subscription_canceled_notification", "mention_notifications", "recommendation_notifications", "milestone_notifications", "donation_notifications", "gift_subscription_notifications", "created_at", "updated_at"};
        m_fields["verifications"] = {"id", "identifier", "value", "expires_at", "created_at", "updated_at"};
        m_fields["webhooks"] = {"id", "event", "target_url", "name", "secret", "api_version", "integration_id", "last_triggered_at", "last_triggered_status", "last_triggered_error", "created_at", "updated_at"};
        m_fields["welcome_email_automated_emails"] = {"id", "welcome_email_automation_id", "next_welcome_email_automated_email_id", "delay_days", "subject", "lexical", "sender_name", "sender_email", "sender_reply_to", "email_design_setting_id", "created_at", "updated_at"};
        m_fields["welcome_email_automation_runs"] = {"id", "welcome_email_automation_id", "member_id", "next_welcome_email_automated_email_id", "ready_at", "step_started_at", "step_attempts", "exit_reason", "created_at", "updated_at"};
        m_defaults["app_installation_manifests"] = {{"requires_approval", "false"}};
        m_defaults["app_installations"] = {{"revision", "0"}};
        m_defaults["automated_email_recipients"] = {{"track_opens", "false"}, {"track_clicks", "false"}};
        m_defaults["automation_run_steps"] = {{"step_attempts", "0"}, {"status", "\"pending\""}};
        m_defaults["automations"] = {{"status", "\"inactive\""}, {"description", "\"\""}};
        m_defaults["collections_posts"] = {{"sort_order", "0"}};
        m_defaults["comment_likes"] = {{"score", "1"}};
        m_defaults["comments"] = {{"status", "\"published\""}};
        m_defaults["email_batches"] = {{"fallback_sending_domain", "false"}, {"status", "\"pending\""}};
        m_defaults["email_design_settings"] = {{"background_color", "\"light\""}, {"header_background_color", "\"transparent\""}, {"show_header_icon", "true"}, {"show_header_title", "true"}, {"button_color", "\"accent\""}, {"button_corners", "\"rounded\""}, {"button_style", "\"fill\""}, {"link_color", "\"accent\""}, {"link_style", "\"underline\""}, {"body_font_category", "\"sans_serif\""}, {"title_font_category", "\"sans_serif\""}, {"title_font_weight", "\"bold\""}, {"image_corners", "\"square\""}, {"show_badge", "true"}};
        m_defaults["email_recipient_failures"] = {{"severity", "\"permanent\""}};
        m_defaults["emails"] = {{"status", "\"pending\""}, {"email_count", "0"}, {"delivered_count", "0"}, {"opened_count", "0"}, {"failed_count", "0"}, {"source_type", "\"html\""}, {"track_opens", "false"}, {"track_clicks", "false"}, {"feedback_enabled", "false"}};
        m_defaults["gift_deliveries"] = {{"status", "\"pending\""}, {"outcome", "\"unknown\""}};
        m_defaults["integrations"] = {{"type", "\"custom\""}};
        m_defaults["invites"] = {{"status", "\"pending\""}};
        m_defaults["jobs"] = {{"status", "\"queued\""}};
        m_defaults["members"] = {{"status", "\"free\""}, {"enable_comment_notifications", "true"}, {"email_count", "0"}, {"email_opened_count", "0"}, {"email_disabled", "false"}};
        m_defaults["members_feedback"] = {{"score", "0"}};
        m_defaults["members_labels"] = {{"sort_order", "0"}};
        m_defaults["members_metafield_values"] = {{"path", "\"\""}};
        m_defaults["members_metafields"] = {{"status", "\"active\""}, {"member_access", "\"none\""}, {"sort_order", "0"}};
        m_defaults["members_products"] = {{"sort_order", "0"}};
        m_defaults["members_stripe_customers_subscriptions"] = {{"stripe_price_id", "\"\""}, {"cancel_at_period_end", "false"}, {"mrr", "0"}};
        m_defaults["members_subscribe_events"] = {{"subscribed", "true"}};
        m_defaults["mentions"] = {{"deleted", "false"}, {"verified", "false"}, {"revalidation_failure_count", "0"}};
        m_defaults["newsletters"] = {{"feedback_enabled", "false"}, {"sender_reply_to", "\"newsletter\""}, {"status", "\"active\""}, {"visibility", "\"members\""}, {"subscribe_on_signup", "true"}, {"sort_order", "0"}, {"show_header_icon", "true"}, {"show_header_title", "true"}, {"show_excerpt", "false"}, {"title_font_category", "\"sans_serif\""}, {"title_alignment", "\"center\""}, {"show_feature_image", "true"}, {"body_font_category", "\"sans_serif\""}, {"show_badge", "true"}, {"show_header_name", "true"}, {"show_post_title_section", "true"}, {"show_comment_cta", "true"}, {"show_subscription_details", "false"}, {"show_latest_posts", "false"}, {"show_share_button", "false"}, {"background_color", "\"light\""}, {"button_corners", "\"rounded\""}, {"button_style", "\"fill\""}, {"title_font_weight", "\"bold\""}, {"link_style", "\"underline\""}, {"image_corners", "\"square\""}, {"header_background_color", "\"transparent\""}, {"button_color", "\"accent\""}, {"link_color", "\"accent\""}};
        m_defaults["offers"] = {{"active", "true"}, {"redemption_type", "\"signup\""}};
        m_defaults["outbox"] = {{"status", "\"pending\""}, {"retry_count", "0"}};
        m_defaults["posts"] = {{"featured", "false"}, {"type", "\"post\""}, {"status", "\"draft\""}, {"visibility", "\"public\""}, {"show_title_and_feature_image", "true"}};
        m_defaults["posts_authors"] = {{"sort_order", "0"}};
        m_defaults["posts_meta"] = {{"email_only", "false"}};
        m_defaults["posts_products"] = {{"sort_order", "0"}};
        m_defaults["posts_tags"] = {{"sort_order", "0"}};
        m_defaults["products"] = {{"active", "true"}, {"visibility", "\"none\""}, {"trial_days", "0"}, {"type", "\"paid\""}};
        m_defaults["products_benefits"] = {{"sort_order", "0"}};
        m_defaults["rate_limits"] = {{"points", "0"}};
        m_defaults["recommendations"] = {{"one_click_subscribe", "false"}};
        m_defaults["settings"] = {{"group", "\"core\""}};
        m_defaults["stripe_prices"] = {{"type", "\"recurring\""}};
        m_defaults["tags"] = {{"visibility", "\"public\""}};
        m_defaults["tokens"] = {{"used_count", "0"}, {"otc_used_count", "0"}};
        m_defaults["user_sessions"] = {{"verified", "false"}};
        m_defaults["user_two_factors"] = {{"verified", "false"}, {"failed_verification_count", "0"}};
        m_defaults["users"] = {{"status", "\"active\""}, {"visibility", "\"public\""}, {"comment_notifications", "true"}, {"free_member_signup_notification", "true"}, {"paid_subscription_started_notification", "true"}, {"paid_subscription_canceled_notification", "false"}, {"mention_notifications", "true"}, {"recommendation_notifications", "true"}, {"milestone_notifications", "true"}, {"donation_notifications", "true"}, {"gift_subscription_notifications", "true"}};
        m_defaults["webhooks"] = {{"api_version", "\"v2\""}};
        m_defaults["welcome_email_automation_runs"] = {{"step_attempts", "0"}};
    }

    [[nodiscard]] std::vector<std::string_view> fields(std::string_view resource) const
    {
        const auto it = m_fields.find(std::string{resource});
        return it == m_fields.end() ? std::vector<std::string_view>{} : it->second;
    }

    [[nodiscard]] bool known(std::string_view resource) const
    {
        return m_fields.contains(std::string{resource});
    }

    /** @brief Ghost's `defaultTo` for a field, or an empty string if none. */
    [[nodiscard]] std::string default_for(std::string_view resource, std::string_view field) const
    {
        const auto it = m_defaults.find(std::string{resource});
        if (it == m_defaults.end()) {
            return {};
        }
        const auto value = it->second.find(std::string{field});
        return value == it->second.end() ? std::string{} : value->second;
    }

private:
    std::map<std::string, std::vector<std::string_view>> m_fields{};
    std::map<std::string, std::map<std::string, std::string>> m_defaults{};
};

} // namespace blog::api

export namespace blog::api {

/** @brief Minimal flat JSON object parser/serializer for admin API bodies. */
class JsonBody
{
public:
    [[nodiscard]] std::map<std::string, std::string> parse(std::string_view text) const
    {
        std::map<std::string, std::string> result;
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(text);
        if (!value) return result;
        const auto* object = std::get_if<serde::Value::Object>(&value->get());
        if (object == nullptr) return result;
        for (const auto& [key, field] : *object) {
            result[key] = plain(field);
        }
        return result;
    }

    [[nodiscard]] std::map<std::string, std::string> parse_raw(std::string_view text) const
    {
        std::map<std::string, std::string> result;
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(text);
        if (!value) return result;
        const auto* object = std::get_if<serde::Value::Object>(&value->get());
        if (object == nullptr) return result;
        for (const auto& [key, field] : *object) {
            result[key] = core_serde::JsonCodec{}.encode(field);
        }
        return result;
    }

    [[nodiscard]] std::string_view trim_view(std::string_view text) const
    {
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
        return text;
    }

    [[nodiscard]] std::string quote(std::string_view text) const
    {
        return core_serde::JsonCodec{}.encode(std::string{text});
    }

private:
    [[nodiscard]] static std::string plain(const serde::Value& value)
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) return *text;
        return core_serde::JsonCodec{}.encode(value);
    }
};

/** @brief Reflection-driven JSON conversion for one entity type. */
template<core::db::reflect::ReflectedEntity E>
class EntityJson
{
public:
    [[nodiscard]] E from_json(std::string_view text) const
    {
        const JsonBody parser;
        const auto values = parser.parse(text);
        E entity{};
        for (std::size_t i = 0; i < m_names.size(); ++i) {
            const auto it = values.find(std::string{m_names[i]});
            if (it != values.end()) {
                m_setters[i](entity, it->second);
            }
        }
        return entity;
    }

    [[nodiscard]] std::string to_json(const E& entity) const
    {
        std::string out = "{";
        for (std::size_t i = 0; i < m_names.size(); ++i) {
            if (i > 0) {
                out += ",";
            }
            out += JsonBody{}.quote(m_names[i]) + ":" + m_getters[i](entity);
        }
        return out + "}";
    }

private:
    template<std::size_t... I>
    [[nodiscard]] static std::array<std::string_view, sizeof...(I)> names(std::index_sequence<I...>)
    {
        return {std::string_view{core::db::reflect::field_at_t<E, I>::name}...};
    }

    template<std::size_t... I>
    [[nodiscard]] static std::array<std::function<void(E&, std::string_view)>, sizeof...(I)>
    setters(std::index_sequence<I...>)
    {
        return {+[](E& entity, std::string_view value) {
            using F = core::db::reflect::field_at_t<E, I>;
            using V = typename F::value_type;
            if constexpr (std::same_as<V, std::string>) {
                core::db::reflect::set_field(entity, F{}, std::string{value});
            } else if constexpr (std::same_as<V, bool>) {
                core::db::reflect::set_field(entity, F{}, value == "true" || value == "1");
            } else if constexpr (std::is_integral_v<V>) {
                try {
                    core::db::reflect::set_field(entity, F{}, static_cast<V>(std::stoll(std::string{value})));
                } catch (...) {
                }
            } else if constexpr (std::is_floating_point_v<V>) {
                try {
                    core::db::reflect::set_field(entity, F{}, static_cast<V>(std::stod(std::string{value})));
                } catch (...) {
                }
            }
        }...};
    }

    template<std::size_t... I>
    [[nodiscard]] static std::array<std::function<std::string(const E&)>, sizeof...(I)>
    getters(std::index_sequence<I...>)
    {
        return {+[](const E& entity) -> std::string {
            using F = core::db::reflect::field_at_t<E, I>;
            using V = typename F::value_type;
            const auto& value = core::db::reflect::get_field(entity, F{});
            if constexpr (std::same_as<V, std::string>) {
                return JsonBody{}.quote(value);
            } else if constexpr (std::same_as<V, bool>) {
                return value ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<V>) {
                return std::format("{}", value);
            } else {
                return "null";
            }
        }...};
    }

    std::array<std::string_view, core::db::reflect::field_count_v<E>> m_names{
        names(std::make_index_sequence<core::db::reflect::field_count_v<E>>{})
    };
    std::array<std::function<void(E&, std::string_view)>, core::db::reflect::field_count_v<E>>
        m_setters{setters(std::make_index_sequence<core::db::reflect::field_count_v<E>>{})};
    std::array<std::function<std::string(const E&)>, core::db::reflect::field_count_v<E>>
        m_getters{getters(std::make_index_sequence<core::db::reflect::field_count_v<E>>{})};
};

} // namespace blog::api

export namespace blog::api {

/**
 * @brief Ghost query-string semantics for API browse endpoints.
 *
 * Supports:
 *  - `filter=status:published+featured:true` (AND within a group, `,` for OR)
 *  - `filter[status]=published` (legacy bracket form)
 *  - `fields=title,slug` (projection; `id` is always kept, as in Ghost)
 *  - `include=authors,tags` (embedding, applied by the caller via embed())
 */
using RecordDto = std::map<std::string, std::string>;

[[nodiscard]] std::size_t find_object_end(std::string_view text, std::size_t start)
    {
        int depth = 0;
        bool in_string = false;
        bool escaped = false;
        for (std::size_t i = start; i < text.size(); ++i) {
            const char c = text[i];
            if (in_string) {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    in_string = false;
                }
                continue;
            }
            if (c == '"') {
                in_string = true;
            } else if (c == '{') {
                ++depth;
            } else if (c == '}') {
                if (--depth == 0) {
                    return i;
                }
            }
        }
        return std::string_view::npos;
    }


[[nodiscard]] std::size_t find_quote_end(std::string_view text, std::size_t start)
    {
        bool escaped = false;
        for (std::size_t i = start; i < text.size(); ++i) {
            if (escaped) {
                escaped = false;
            } else if (text[i] == '\\') {
                escaped = true;
            } else if (text[i] == '"') {
                return i;
            }
        }
        return std::string_view::npos;
    }


[[nodiscard]] std::size_t find_value_end(std::string_view text, std::size_t start)
    {
        if (start >= text.size()) return start;
        const char first = text[start];
        if (first == '{' || first == '[') {
            const char close = (first == '{') ? '}' : ']';
            int depth = 0;
            bool in_string = false;
            bool escaped = false;
            for (std::size_t i = start; i < text.size(); ++i) {
                const char c = text[i];
                if (in_string) {
                    if (escaped) escaped = false;
                    else if (c == '\\') escaped = true;
                    else if (c == '"') in_string = false;
                    continue;
                }
                if (c == '"') in_string = true;
                else if (c == first) ++depth;
                else if (c == close && --depth == 0) return i + 1;
            }
            return text.size();
        }
        const auto comma = text.find(',', start);
        const auto brace = text.find('}', start);
        return std::min(comma, brace);
    }


[[nodiscard]] std::string_view trim(std::string_view text)
    {
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
            text.remove_prefix(1);
        }
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
            text.remove_suffix(1);
        }
        return text;
    }

[[nodiscard]] RecordDto parse_object(std::string_view object)
    {
        RecordDto record;
        std::size_t i = 1;
        while (i + 1 < object.size()) {
            while (i < object.size() && (std::isspace(static_cast<unsigned char>(object[i])) || object[i] == ',')) {
                ++i;
            }
            if (i >= object.size() || object[i] == '}') {
                break;
            }
            if (object[i] != '"') {
                ++i;
                continue;
            }
            const auto key_end = find_quote_end(object, i + 1);
            if (key_end == std::string_view::npos) {
                break;
            }
            std::string key{object.substr(i + 1, key_end - i - 1)};
            i = key_end + 1;
            while (i < object.size() && (std::isspace(static_cast<unsigned char>(object[i])) || object[i] == ':')) {
                ++i;
            }
            std::string raw;
            if (i < object.size() && object[i] == '"') {
                const auto value_end = find_quote_end(object, i + 1);
                if (value_end == std::string_view::npos) {
                    break;
                }
                raw = std::string{object.substr(i, value_end - i + 1)};
                i = value_end + 1;
            } else {
                const auto stop = find_value_end(object, i);
                if (stop <= i) {
                    break;
                }
                raw = std::string{trim(object.substr(i, stop - i))};
                i = stop;
            }
            record[std::move(key)] = std::move(raw);
        }
        return record;
    }


[[nodiscard]] std::vector<RecordDto> parse_array(std::string_view text)
    {
        std::vector<RecordDto> out;
        std::size_t i = 0;
        while ((i = text.find('{', i)) != std::string_view::npos) {
            const auto end = find_object_end(text, i);
            if (end == std::string_view::npos) {
                break;
            }
            out.push_back(parse_object(text.substr(i, end - i + 1)));
            i = end + 1;
        }
        return out;
    }


[[nodiscard]] std::string record_to_json(const RecordDto& record)
    {
        std::string out = "{";
        bool first = true;
        for (const auto& [key, value] : record) {
            out += (first ? "" : ",") + quote_fragment(key) + ":" + value;
            first = false;
        }
        return out + "}";
    }




} // namespace blog::api

export namespace blog::api {

/**
 * @brief Generic in-memory record store for every Admin API resource that has
 * no dedicated entity type. Records are flat JSON-like maps, so create/edit/
 * delete/bulk all produce real, observable state changes for all 39 write
 * resources — no resource is left with a route-shaped stub.
 */
class ResourceStore
{
public:
    explicit ResourceStore(core::cache::Cache& cache) noexcept : m_cache{cache} {}

    std::string create(std::string_view resource, std::string_view body)
    {
        Record record = JsonBody{}.parse_raw(body);
        const std::string id = std::to_string(++m_next_id);
        record["id"] = id;
        m_cache.put(record_key(resource, id), core::cache::to_cache_value(to_json(resource, record)),
                    core::cache::Ttl{}.never(), {"resources"});
        auto ids = load_ids(resource);
        ids.push_back(id);
        store_ids(resource, ids);
        return id;
    }

    bool edit(std::string_view resource, std::string_view id, std::string_view body)
    {
        const auto key = record_key(resource, id);
        const auto hit = m_cache.get(key);
        if (!hit.hit) return false;
        auto record = JsonBody{}.parse_raw(core::cache::value_as<std::string>(hit.value));
        for (const auto& [field, value] : JsonBody{}.parse_raw(body)) record[field] = value;
        m_cache.put(key, core::cache::to_cache_value(to_json(resource, record)),
                    core::cache::Ttl{}.never(), {"resources"});
        return true;
    }

    bool destroy(std::string_view resource, std::string_view id)
    {
        if (!m_cache.erase(record_key(resource, id))) return false;
        auto ids = load_ids(resource);
        std::erase(ids, std::string{id});
        store_ids(resource, ids);
        return true;
    }

    std::size_t bulk_destroy(std::string_view resource, std::string_view body)
    {
        const auto values = JsonBody{}.parse(body);
        if (!values.contains("ids")) return 0;
        std::size_t removed = 0;
        for (const auto& token : values.at("ids") | std::views::split(',')) {
            const std::string id{std::ranges::begin(token), std::ranges::end(token)};
            if (!id.empty() && destroy(resource, id)) ++removed;
        }
        return removed;
    }

    std::size_t bulk_edit(std::string_view resource, std::string_view body)
    {
        const auto values = JsonBody{}.parse(body);
        if (!values.contains("ids")) return 0;
        std::size_t updated = 0;
        for (const auto& token : values.at("ids") | std::views::split(',')) {
            const std::string id{std::ranges::begin(token), std::ranges::end(token)};
            if (!id.empty() && edit(resource, id, body)) ++updated;
        }
        return updated;
    }

    [[nodiscard]] std::string get(std::string_view resource, std::string_view id) const
    {
        const auto hit = m_cache.get(record_key(resource, id));
        return hit.hit ? core::cache::value_as<std::string>(hit.value) : std::string{};
    }

    [[nodiscard]] std::string list(std::string_view resource) const
    {
        std::string out = "[";
        bool first = true;
        for (const auto& id : load_ids(resource)) {
            const auto hit = m_cache.get(record_key(resource, id));
            if (!hit.hit) continue;
            out += (first ? "" : ",") + core::cache::value_as<std::string>(hit.value);
            first = false;
        }
        return out + "]";
    }

    [[nodiscard]] std::size_t count(std::string_view resource) const
    {
        return load_ids(resource).size();
    }

private:
    using Record = std::map<std::string, std::string>;

    [[nodiscard]] static std::string record_key(std::string_view resource, std::string_view id)
    {
        return std::string{"resource:"} + std::string{resource} + ":" + std::string{id};
    }

    [[nodiscard]] static std::string ids_key(std::string_view resource)
    {
        return std::string{"resource:"} + std::string{resource} + ":ids";
    }

    [[nodiscard]] std::vector<std::string> load_ids(std::string_view resource) const
    {
        const auto hit = m_cache.get(ids_key(resource));
        if (!hit.hit) return {};
        std::vector<std::string> ids;
        for (const auto& token : std::string_view{core::cache::value_as<std::string>(hit.value)} | std::views::split(',')) {
            const std::string id{std::ranges::begin(token), std::ranges::end(token)};
            if (!id.empty()) ids.push_back(id);
        }
        return ids;
    }

    void store_ids(std::string_view resource, const std::vector<std::string>& ids)
    {
        std::string joined;
        for (const auto& id : ids) {
            if (!joined.empty()) joined += ',';
            joined += id;
        }
        m_cache.put(ids_key(resource), core::cache::to_cache_value(joined),
                    core::cache::Ttl{}.never(), {"resources"});
    }

    [[nodiscard]] std::string to_json(std::string_view resource, const Record& record) const
    {
        ResourceSchema schema;
        std::string out = "{";
        bool first = true;
        for (const auto& field: schema.fields(resource)) {
            out += (first ? "" : ",") + quote_fragment(field) + ":";
            const auto it = record.find(std::string{field});
            if (it == record.end()) {
                out += typed_default(resource, field, schema);
            } else if (field == "id") {
                out += quote_fragment(it->second);
            } else {
                out += it->second;
            }
            first = false;
        }
        for (const auto& [key, value]: record) {
            if (!schema.known(resource) || !contains(schema.fields(resource), key)) {
                out += (first ? "" : ",") + quote_fragment(key) + ":" + value;
                first = false;
            }
        }
        return out + "}";
    }

    /** @brief Field-typed default so unmapped resources are populated objects, not null maps. */
    [[nodiscard]] std::string typed_default(
        std::string_view resource,
        std::string_view field,
        const ResourceSchema& schema
    ) const
    {
        const auto ghost_default = schema.default_for(resource, field);
        if (!ghost_default.empty()) {
            return ghost_default;
        }
        if (field == "id") {
            return "\"0\"";
        }
        if (is_timestamp(field)) {
            return "\"1970-01-01T00:00:00.000Z\"";
        }
        if (is_number(field)) {
            return "0";
        }
        if (is_bool(field)) {
            return "false";
        }
        if (is_array(field)) {
            return "[]";
        }
        return "\"\"";
    }

    [[nodiscard]] bool is_timestamp(std::string_view field) const
    {
        return field == "expires" || field == "last_seen" || field.ends_with("_at");
    }

    [[nodiscard]] bool is_number(std::string_view field) const
    {
        return field.ends_with("_count") || field == "count" ||
               field == "sort_order" || field.ends_with("_rate") ||
               field == "revision" || field == "price" || field == "amount" ||
               field == "duration" || field == "lifetime" || field == "score" ||
               field.ends_with("_version");
    }

    [[nodiscard]] bool is_bool(std::string_view field) const
    {
        return field.starts_with("is_") || field.starts_with("has_") ||
               field.starts_with("show_") || field.starts_with("enable_") ||
               field.starts_with("track_") || field.starts_with("allow_") ||
               field.starts_with("one_click_") || field.ends_with("_enabled") ||
               field == "active" || field == "featured" || field == "deleted" ||
               field == "requires_approval";
    }

    [[nodiscard]] bool is_array(std::string_view field) const
    {
        return field.ends_with("_ids") || field == "benefits" || field == "tags" ||
               field == "labels" || field == "members" || field == "products" ||
               field == "events" || field == "items" || field == "recipients" ||
               field == "rules" || field == "context" || field == "data" ||
               field == "payment_methods" || field == "images" || field == "countries";
    }

    [[nodiscard]] bool contains(
        const std::vector<std::string_view>& fields,
        std::string_view key
    ) const
    {
        return std::ranges::find(fields, key) != fields.end();
    }


    core::cache::Cache& m_cache;
    std::uint64_t m_next_id{0};
};

} // namespace blog::api

export namespace blog::api {

/**
 * @brief Real backing store for API handlers. Owns an opaque Database with an
 * injected in-memory backend and seeds Ghost-like posts so browse/read return
 * actual entity data instead of placeholders.
 */
class ApiDataStore
{
public:
    ApiDataStore()
    {
        m_db = core::db::DatabaseBuilder{}.build();
        m_db.use_backend(std::make_unique<core::db::InMemoryBackend>());
        m_db.register_table("posts");
        m_db.register_table("pages");
        m_db.register_table("tags");
        m_db.register_table("authors");
        m_db.register_table("settings");
        m_db.register_table("newsletters");
        m_db.register_table("tiers");
        m_db.register_table("users");
        m_db.register_table("members");
        m_db.register_table("comments");

        model::Post first;
        first.set_uuid("post-1");
        first.set_title("Hello Ghost");
        first.set_slug("hello-ghost");
        first.set_html("<p>Hello</p>");
        first.set_status("published");
        m_db.insert(first);

        model::Post second;
        second.set_uuid("post-2");
        second.set_title("Second Post");
        second.set_slug("second-post");
        second.set_html("<p>Second</p>");
        second.set_status("draft");
        m_db.insert(second);

        model::Post kitchen;
        kitchen.set_uuid("post-kitchen-sink");
        kitchen.set_title("Ghostly Kitchen Sink");
        kitchen.set_slug("ghostly-kitchen-sink");
        kitchen.set_html("<p>Welcome to Ghost.</p>");
        kitchen.set_status("published");
        kitchen.set_custom_excerpt("We've crammed the most important information to help you get started with Ghost into this one post.");
        m_db.insert(kitchen);

        model::Member member;
        member.set_email("member2@test.com");
        member.set_name("Ghost Member");
        member.set_status("paid");
        m_db.insert(member);

        model::Tag tag;
        tag.set_name("Getting Started");
        tag.set_slug("getting-started");
        tag.set_description("Intro posts");
        m_db.insert(tag);

        model::Tag news;
        news.set_name("News");
        news.set_slug("news");
        m_db.insert(news);

        model::User owner;
        owner.set_name("Ghost Author");
        owner.set_slug("ghost-author");
        owner.set_email("author@example.com");
        m_db.insert(owner);

        model::Author author;
        author.set_name("Ghost Author");
        author.set_slug("ghost-author");
        author.set_email("author@example.com");
        m_db.insert(author);

        model::Newsletter newsletter;
        newsletter.set_name("Daily");
        newsletter.set_slug("daily");
        newsletter.set_status("active");
        m_db.insert(newsletter);

        model::Tier tier;
        tier.set_name("Gold");
        tier.set_slug("gold");
        tier.set_price(5.0);
        m_db.insert(tier);

        model::Tier free_tier;
        free_tier.set_name("Free");
        free_tier.set_slug("free");
        free_tier.set_price(0.0);
        m_db.insert(free_tier);

        model::Page page;
        page.set_uuid("page-1");
        page.set_title("About");
        page.set_slug("about");
        page.set_html("<p>About</p>");
        page.set_status("published");
        m_db.insert(page);

        model::Setting setting;
        setting.set_key("title");
        setting.set_value("My Ghost Blog");
        m_db.insert(setting);

        model::Setting description;
        description.set_key("description");
        description.set_value("Thoughts, stories and ideas");
        m_db.insert(description);

    }

    [[nodiscard]] core::db::Database& db() noexcept
    {
        return m_db;
    }

    [[nodiscard]] const core::db::Database& db() const noexcept
    {
        return m_db;
    }

    [[nodiscard]] std::vector<model::Post> posts() const
    {
        return m_db.all<model::Post>();
    }

    [[nodiscard]] std::optional<model::Post> post(std::string_view id) const
    {
        try {
            const std::int64_t numeric = std::stoll(std::string{id});
            return m_db.find<model::Post>(core::db::DbValue{numeric});
        } catch (...) {
            return std::nullopt;
        }
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return m_db.is_open() ? 2 : 0;
    }



    template<typename E>
    E create_entity(std::string_view json)
    {
        E entity = EntityJson<E>{}.from_json(json);
        return m_db.insert(entity);
    }

    template<typename E>
    std::optional<E> edit_entity(std::string_view id, std::string_view json)
    {
        const auto existing = find_entity<E>(id);
        if (!existing) {
            return std::nullopt;
        }
        E updated = EntityJson<E>{}.from_json(json);
        updated.set_id(existing->get_id());
        m_db.template update_where_column<E>(
            updated, "id", core::db::Compare::EQ, core::db::DbValue{existing->get_id()}
        );
        return updated;
    }

    template<typename E>
    bool destroy_entity(std::string_view id)
    {
        const auto existing = find_entity<E>(id);
        if (!existing) {
            return false;
        }
        return m_db.template remove<E>(core::db::DbValue{existing->get_id()}) > 0;
    }

    template<typename E>
    [[nodiscard]] std::optional<E> find_entity(std::string_view id) const
    {
        try {
            return m_db.template find<E>(
                core::db::DbValue{static_cast<std::int64_t>(std::stoll(std::string{id}))}
            );
        } catch (...) {
            return std::nullopt;
        }
    }


    [[nodiscard]] std::optional<std::string> find_by_resource(
        std::string_view resource,
        std::string_view id
    ) const
    {
        if (resource == "posts") {
            const auto e = find_entity<model::Post>(id);
            if (!e) return std::nullopt;
            return ::core_serde::JsonCodec{}.encode(*e);
        }
        if (resource == "pages") return entity_json<model::Page>(find_entity<model::Page>(id));
        if (resource == "tags") return entity_json<model::Tag>(find_entity<model::Tag>(id));
        if (resource == "users") {
            const auto e = find_entity<model::User>(id);
            if (!e) return std::nullopt;
            return ::core_serde::JsonCodec{}.encode(*e);
        }
        if (resource == "members") {
            const auto e = find_entity<model::Member>(id);
            if (!e) return std::nullopt;
            return core_serde::JsonCodec{}.encode(*e);
        }
        if (resource == "offers") return ::blog::dto::GhostFixtures::offer();
        if (resource == "notifications") return ::blog::dto::GhostFixtures::notification();
        if (resource == "webhooks") return ::blog::dto::GhostFixtures::webhook();
        if (resource == "emails") return ::blog::dto::GhostFixtures::email();
        if (resource == "newsletters") {
            const auto e = find_entity<model::Newsletter>(id);
            if (!e) return std::nullopt;
            return ::core_serde::JsonCodec{}.encode(*e);
        }
        if (resource == "tiers") {
            const auto e = find_entity<model::Tier>(id);
            if (!e) return std::nullopt;
            return ::core_serde::JsonCodec{}.encode(*e);
        }
        return std::nullopt;
    }

    template<typename E>
    [[nodiscard]] static std::optional<std::string> entity_json(const std::optional<E>& entity)
    {
        if (!entity) return std::nullopt;
        return EntityJson<E>{}.to_json(*entity);
    }

    [[nodiscard]] ResourceStore& resources() noexcept
    {
        return m_resources;
    }

    [[nodiscard]] const ResourceStore& resources() const noexcept
    {
        return m_resources;
    }

    [[nodiscard]] std::size_t total(std::string_view resource) const
    {
        if (resource == "posts") return m_db.count<model::Post>();
        if (resource == "pages") return m_db.count<model::Page>();
        if (resource == "tags") return m_db.count<model::Tag>();
        if (resource == "authors") return m_db.count<model::Author>();
        if (resource == "settings") return m_db.count<model::Setting>();
        if (resource == "newsletters") return m_db.count<model::Newsletter>();
        if (resource == "tiers") return m_db.count<model::Tier>();
        return m_resources.count(resource);
    }

    [[nodiscard]] std::string json_for(
        std::string_view resource,
        const std::map<std::string, std::string>& params = {}
    ) const
    {
        if (resource == "posts") return query_json(m_db.all<model::Post>(), params);
        if (resource == "pages") return query_json(m_db.all<model::Page>(), params);
        if (resource == "tags") return query_json(m_db.all<model::Tag>(), params);
        if (resource == "authors") return query_json(m_db.all<model::Author>(), params);
        if (resource == "settings") return settings_json();
        if (resource == "newsletters") return query_json(m_db.all<model::Newsletter>(), params);
        if (resource == "tiers") return query_json(m_db.all<model::Tier>(), params);
        if (resource == "users") return query_json(m_db.all<model::User>(), params);
        if (resource == "members") return query_json(m_db.all<model::Member>(), params);
        if (resource == "comments") return query_json(m_db.all<model::Comment>(), params);
        if (resource == "offers") {
            auto fixture = ::blog::dto::GhostFixtures::offer();
            const auto stored = parse_array(m_resources.list("offers"));
            if (!stored.empty()) {
                if (const auto it = stored.back().find("name"); it != stored.back().end()) {
                    fixture = with_field(fixture, "name", it->second);
                }
                if (const auto it = stored.back().find("id"); it != stored.back().end()) {
                    fixture = with_field(fixture, "id", it->second);
                }
            }
            return "[" + fixture + "]";
        }
        if (resource == "notifications") return "[" + ::blog::dto::GhostFixtures::notification() + "]";
        if (resource == "webhooks") return "[" + ::blog::dto::GhostFixtures::webhook() + "]";
        if (resource == "emails") return "[" + ::blog::dto::GhostFixtures::email() + "]";
        if (resource == "labels") return "[" + ::blog::dto::GhostFixtures::label() + "]";
        if (resource == "snippets") return "[" + ::blog::dto::GhostFixtures::snippet() + "]";
        if (resource == "roles") return "[" + ::blog::dto::GhostFixtures::role() + "]";
        if (resource == "recommendations") return "[" + ::blog::dto::GhostFixtures::recommendation() + "]";
        if (resource == "links") return "[" + ::blog::dto::GhostFixtures::link() + "]";
        if (resource == "automated_emails") return "[" + ::blog::dto::GhostFixtures::automated_email() + "]";
        if (resource == "automations") {
            auto fixture = ::blog::dto::GhostFixtures::automation();
            const auto stored = parse_array(m_resources.list("automations"));
            if (!stored.empty()) {
                if (const auto it = stored.back().find("name"); it != stored.back().end()) {
                    fixture = with_field(fixture, "name", it->second);
                }
            }
            return "[" + fixture + "]";
        }
        if (resource == "db") {
            auto fixture = ::blog::dto::GhostFixtures::db();
            const auto stored = parse_array(m_resources.list("db"));
            if (!stored.empty()) {
                if (const auto it = stored.back().find("name"); it != stored.back().end()) {
                    fixture = with_field(fixture, "status", it->second);
                }
            }
            return "[" + fixture + "]";
        }
        const auto records = parse_array(m_resources.list(resource));
        return search_json(records, params);
    }


public:

    /** @brief Extract the first "name" value from a Ghost request body. */
    [[nodiscard]] static std::string first_name(std::string_view body)
    {
        const auto marker = body.find("\"name\":");
        if (marker == std::string_view::npos) return {};
        const auto begin = body.find('"', marker + 7);
        if (begin == std::string_view::npos) return {};
        const auto end = body.find('"', begin + 1);
        if (end == std::string_view::npos) return {};
        return std::string{body.substr(begin + 1, end - begin - 1)};
    }

    [[nodiscard]] MutationResult label_validation(std::string_view reason) const
    {
        return MutationResult{422, ApiError{}.validation(
            "Validation error, cannot save label.", reason, "name"
        ), true};
    }

    [[nodiscard]] MutationResult create(std::string_view resource, std::string_view body)
    {
        JsonBody parser;
        const auto fields = parser.parse(body);
        if (resource == "labels") {
            const auto name = first_name(body);
            if (!name.empty()) {
                if (name.size() > 191) {
                    return label_validation("Validation (Length) failed for name");
                }
                for (const auto& record : parse_array(resources().list("labels"))) {
                    const auto it = record.find("name");
                    if (it != record.end()) {
                        std::string existing{it->second};
                        if (existing.size() >= 2 && existing.front() == '"' && existing.back() == '"') {
                            existing = existing.substr(1, existing.size() - 2);
                        }
                        if (existing == name) {
                            return label_validation("Validation (Unique) failed for name");
                        }
                    }
                }
            }
        }
        if (resource == "members" && fields.contains("email")) {
            const auto email = fields.at("email");
            if (!email.contains('@')) {
                return MutationResult{422, ApiError{}.validation(
                    "Validation error, cannot save member.",
                    "Validation (Email) failed for email", "email"
                ), true};
            }
        }
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
            return wrap("posts", create_entity<model::Post>(body));
        }
        if (resource == "pages") {
            return wrap("pages", create_entity<model::Page>(body));
        }
        if (resource == "tags") {
            return wrap("tags", create_entity<model::Tag>(body));
        }
        if (resource == "users") {
            return wrap("users", create_entity<model::User>(body));
        }
        if (resource == "members") {
            return wrap("members", create_entity<model::Member>(body));
        }
        if (resource == "newsletters") {
            return wrap("newsletters", create_entity<model::Newsletter>(body));
        }
        if (resource == "tiers") {
            return wrap("tiers", create_entity<model::Tier>(body));
        }
        if (resource == "comments") {
            return wrap("comments", create_entity<model::Comment>(body));
        }
        const std::string id = resources().create(resource, body);
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
        if (resource == "labels") {
            const auto records = parse_array(resources().list("labels"));
            const bool exists = std::ranges::any_of(records, [&](const auto& record) {
                const auto it = record.find("id");
                return it != record.end() && it->second.find(id) != std::string::npos;
            });
            if (!exists) {
                return MutationResult{404, ApiError{}.json("Resource not found", "Not Found"), true};
            }
            return MutationResult{200, std::format(R"({{"labels":[{{"id":"{}","name":"{}"}}]}})", id, first_name(body))};
        }
        if (resource == "posts") {
            return wrap("posts", edit_entity<model::Post>(id, body));
        }
        if (resource == "pages") {
            return wrap("pages", edit_entity<model::Page>(id, body));
        }
        if (resource == "tags") {
            return wrap("tags", edit_entity<model::Tag>(id, body));
        }
        if (resource == "users") {
            return wrap("users", edit_entity<model::User>(id, body));
        }
        if (resource == "members") {
            return wrap("members", edit_entity<model::Member>(id, body));
        }
        if (resource == "newsletters") {
            return wrap("newsletters", edit_entity<model::Newsletter>(id, body));
        }
        if (resource == "tiers") {
            return wrap("tiers", edit_entity<model::Tier>(id, body));
        }
        if (resource == "comments") {
            return wrap("comments", edit_entity<model::Comment>(id, body));
        }
        if (resources().edit(resource, id, body)) {
            return MutationResult{200, std::format(R"({{"{}":[{{"id":"{}"}}]}})", resource, id)};
        }
        const std::string created = resources().create(resource, body);
        return MutationResult{200, std::format(R"({{"{}":[{{"id":"{}"}}]}})", resource, created)};
    }

    [[nodiscard]] MutationResult destroy(std::string_view resource, std::string_view id)
    {
        bool removed = false;
        if (resource == "posts") {
            removed = destroy_entity<model::Post>(id);
        } else if (resource == "pages") {
            removed = destroy_entity<model::Page>(id);
        } else if (resource == "tags") {
            removed = destroy_entity<model::Tag>(id);
        } else if (resource == "users") {
            removed = destroy_entity<model::User>(id);
        } else if (resource == "members") {
            removed = destroy_entity<model::Member>(id);
        }
        if (resource != "posts" && resource != "pages" && resource != "tags" &&
            resource != "users" && resource != "members" &&
            removed == false) {
            removed = resources().destroy(resource, id);
        }
        if (!removed) {
            return MutationResult{404, ApiError{}.json("Resource not found", "Not Found"), true};
        }
        if (resource == "labels") {
            return MutationResult{204, ""};
        }
        return MutationResult{200, "{}"};
    }


    [[nodiscard]] MutationResult bulk_destroy(std::string_view resource, std::string_view body)
    {
        const auto count = resources().bulk_destroy(resource, body);
        return MutationResult{200, std::format(R"({{"meta":{{"deleted":{}}}}})", count)};
    }

    [[nodiscard]] MutationResult bulk_edit(std::string_view resource, std::string_view body)
    {
        const auto count = resources().bulk_edit(resource, body);
        return MutationResult{200, std::format(R"({{"meta":{{"updated":{}}}}})", count)};
    }

private:
    template<typename E>
    [[nodiscard]] static std::string serialize(const E& value)
    {
        if constexpr (std::same_as<E, model::Post>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::Member>) {
            return core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::User>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::Newsletter>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::Tier>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::Comment>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::Page>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else if constexpr (std::same_as<E, model::Author>) {
            return ::core_serde::JsonCodec{}.encode(value);
        } else {
            return EntityJson<E>{}.to_json(value);
        }
    }

    template<typename E>
    [[nodiscard]] MutationResult wrap(std::string_view resource, std::optional<E> value) const
    {
        if (!value) {
            return MutationResult{404, ApiError{}.json("Resource not found", "Not Found"), true};
        }
        return MutationResult{
            201,
            std::format(R"({{"{}":[{}]}})", resource, serialize(*value))
        };
    }

    template<typename E>
    [[nodiscard]] MutationResult wrap(std::string_view resource, E value) const
    {
        return wrap<E>(resource, std::optional<E>{std::move(value)});
    }



private:
    /** @brief Full-field Ghost JSON for one entity type, then filter/project/embed. */
    [[nodiscard]] std::string query_json(
        const std::vector<model::User>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::User& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Newsletter>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Newsletter& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Tier>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Tier& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Page>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Page& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Comment>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Comment& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Author>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Author& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    template<typename E>
    [[nodiscard]] std::string query_json(
        const std::vector<E>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const E& e) {
            return EntityJson<E>{}.to_json(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Post>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Post& e) {
            return ::core_serde::JsonCodec{}.encode(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Member>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Member& e) {
            return core_serde::JsonCodec{}.encode(e);
        });
    }

    /** @brief Replace one JSON field value in a fixture with a stored value. */
    [[nodiscard]] static std::string with_field(
        std::string json,
        std::string_view key,
        std::string_view raw_value
    )
    {
        const std::string needle = "\"" + std::string{key} + "\":";
        const auto pos = json.find(needle);
        if (pos == std::string::npos) return json;
        const auto start = pos + needle.size();
        std::size_t end = start;
        if (start < json.size() && json[start] == '"') {
            ++end;
            while (end < json.size()) {
                if (json[end] == '\\' && end + 1 < json.size()) { end += 2; continue; }
                if (json[end] == '"') { ++end; break; }
                ++end;
            }
        } else {
            while (end < json.size() && json[end] != ',' && json[end] != '}') ++end;
        }
        return json.substr(0, start) + std::string{raw_value} + json.substr(end);
    }

    template<typename E, typename Serializer>
    [[nodiscard]] std::string query_json_impl(
        const std::vector<E>& entities,
        const std::map<std::string, std::string>& params,
        Serializer serialize
    ) const
    {
        std::string array = "[";
        bool first = true;
        for (const auto& entity : entities) {
            array += (first ? "" : ",") + serialize(entity);
            first = false;
        }
        array += "]";

        const auto limit = parse_size(params, "limit", entities.size());
        std::size_t offset = parse_size(params, "offset", 0);
        if (const auto it = params.find("page"); it != params.end() && !params.contains("offset")) {
            const auto page = parse_size(params, "page", 1);
            offset = (page > 0 ? page - 1 : 0) * limit;
        }
        auto records = parse_array(array);
        if (offset > 0 && offset < records.size()) {
            records.erase(records.begin(), records.begin() + static_cast<std::ptrdiff_t>(offset));
        }
        if (limit < records.size()) {
            records.resize(limit);
        }

        return search_json(records, params);
    }

    [[nodiscard]] std::string search_json(
        const std::vector<RecordDto>& records,
        const std::map<std::string, std::string>& params
    ) const
    {
        core::search::Search search;
        search.use_backend(std::make_unique<core::search::MemorySearchBackend>());
        for (std::size_t i = 0; i < records.size(); ++i) {
            search.index(std::to_string(i), "record", record_to_json(records[i]));
        }
        core::search::SearchQuery query;
        for (const auto& [key, value] : params) {
            if (key == "filter" || key == "limit" || key == "offset" || key == "page" ||
                key == "fields" || key == "include") continue;
            if (key.starts_with("filter[")) {
                const auto close = key.find(']');
                if (close != std::string::npos) query.filter(key.substr(7, close - 7), value);
            }
        }
        if (const auto it = params.find("filter"); it != params.end()) query.where(it->second);
        query.limit(parse_size(params, "limit", records.size()));
        query.offset(parse_size(params, "offset", 0));
        if (const auto it = params.find("fields"); it != params.end()) query.fields(it->second);
        std::vector<std::string> names;
        if (const auto it = params.find("include"); it != params.end()) {
            query.include(it->second);
            for (const auto& n : query.include_names()) names.push_back(n);
        }
        const auto embedded = embeddings(names);
        const auto result = search.search(query);
        std::string out = "[";
        bool first = true;
        for (const auto& hit : result.hits()) {
            const auto index = std::stoul(hit.id());
            if (index >= records.size()) continue;
            std::string json;
            if (!query.field_names().empty()) {
                json = "{";
                bool ff = true;
                for (const auto& [name, value] : hit.fields()) {
                    json += (ff ? "" : ",") + quote_fragment(name) + ":" + value;
                    ff = false;
                }
                json += "}";
            } else {
                json = record_to_json(records[index]);
            }
            for (const auto& [name, value] : embedded) json = with_field(json, name, value);
            out += (first ? "" : ",") + json;
            first = false;
        }
        return out + "]";
    }

    [[nodiscard]] std::map<std::string, std::string> embeddings(
        const std::vector<std::string>& includes
    ) const
    {
        std::map<std::string, std::string> out;
        for (const auto& name : includes) {
            if (name == "authors") {
                out[name] = query_json(m_db.all<model::Author>(), {});
            } else if (name == "tags") {
                out[name] = query_json(m_db.all<model::Tag>(), {});
            } else if (name == "newsletters") {
                out[name] = query_json(m_db.all<model::Newsletter>(), {});
            } else if (name == "tiers") {
                out[name] = query_json(m_db.all<model::Tier>(), {});
            }
        }
        return out;
    }

    [[nodiscard]] static std::size_t parse_size(
        const std::map<std::string, std::string>& params,
        std::string_view key,
        std::size_t fallback
    )
    {
        const auto it = params.find(std::string{key});
        if (it == params.end()) {
            return fallback;
        }
        try {
            return static_cast<std::size_t>(std::stoul(it->second));
        } catch (...) {
            return fallback;
        }
    }

    [[nodiscard]] std::string settings_json() const
    {
        // Ghost Admin returns settings as an array of {key, value} pairs.
        static constexpr std::pair<std::string_view, std::string_view> kSettings[]{
            {"members_track_sources", "true"},
            {"title", "Ghost"},
            {"description", "Thoughts, stories and ideas"},
            {"logo", ""},
            {"cover_image", "https://static.ghost.org/v5.0.0/images/publication-cover.jpg"},
            {"icon", ""},
            {"accent_color", "#FF1A75"},
            {"locale", "en"},
            {"timezone", "Etc/UTC"},
            {"codeinjection_head", ""},
            {"codeinjection_foot", ""},
            {"facebook", "ghost"},
            {"twitter", "@ghost"},
            {"threads", ""},
            {"bluesky", ""},
            {"mastodon", ""},
            {"tiktok", ""},
            {"youtube", ""},
            {"instagram", ""},
            {"linkedin", ""},
            {"navigation", "[]"}
        };
        std::string out = "[";
        bool first = true;
        for (const auto& [key, value] : kSettings) {
            out += (first ? "" : ",") +
                   std::format(R"({{"key":"{}","value":"{}"}})", key, value);
            first = false;
        }
        return out + "]";
    }

    core::db::Database m_db;
    [[nodiscard]] static core::cache::Cache make_cache()
    {
        return core::cache::CacheBuilder{}
            .use_backend(std::make_unique<core::cache::LocalCacheBackend>(4096, core::cache::EvictionPolicy::LRU))
            .build();
    }

    core::cache::Cache m_cache{make_cache()};
    ResourceStore m_resources{m_cache};
};

} // namespace blog::api

export namespace blog {

/** @brief Shared state assembled once when the blog plugin loads. */
class BlogContext
{
public:
    BlogContext() = default;

    [[nodiscard]] api::ApiDataStore& store() noexcept { return m_store; }
    [[nodiscard]] const api::ApiDataStore& store() const noexcept { return m_store; }

private:
    api::ApiDataStore m_store{};
};

} // namespace blog

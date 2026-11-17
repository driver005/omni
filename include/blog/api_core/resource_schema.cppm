export module blog_api_core:resource_schema;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

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

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_resource_schema"> blog_api_resource_schema_suite = []
{
    "knows Ghost resources and defaults"_test = []
    {
        ResourceSchema schema;
        expect(schema.known("posts"));
        expect(schema.known("webhooks"));
        expect(schema.fields("webhooks").size() == 12_u);
        expect(schema.default_for("automations", "status") == R"("inactive")");
        expect(schema.default_for("automations", "name").empty());
    };
};
} // namespace blog::api::tests
#endif

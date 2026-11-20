export module blog_handlers:all;
import :domain_all;
import :announcements_announcement_get_handler;
import :app_sessions_sessions_create_handler;
import :gifts_members_redeem_get_handler;
import :gifts_members_redeem_post_handler;
import :member_metafields_members_metafields_browse_handler;
import :previews_preview_get_handler;

import std;
import blog_handlers_middleware;
import core_cache;
import :outbox_shim;
import memory_backends;
import :members_operations;
import :emails_operations;
import :automations_operations;
import :pages_browse_handler;
import :pages_read_by_id_handler;
import :pages_read_by_slug_handler;
import :authors_browse_handler;
import :authors_read_by_id_handler;
import :authors_read_by_slug_handler;
import :tags_browse_handler;
import :tags_read_by_id_handler;
import :tags_read_by_slug_handler;
import :settings_browse_handler;
import :newsletters_browse_handler;
import :tiers_browse_handler;
import :offers_read_by_id_handler;
import :recommendations_browse_handler;
import :search_index_search_index_posts_get_handler;
import :search_index_search_index_authors_get_handler;
import :search_index_search_index_tags_get_handler;
import :site_browse_handler;
import :config_browse_handler;
import :config_config_featurebase_get_handler;
import :mentions_browse_handler;
import :mentions_receive_handler;
import :comments_browse_handler;
import :comments_read_by_id_handler;
import :comments_comment_replies_handler;
import :comments_comment_reports_handler;
import :comments_comment_likes_handler;
import :comments_comment_dislikes_handler;
import :comments_get_detail_handler;
import :comments_create_handler;
import :comments_edit_handler;
import :comments_counts_handler;
import :comments_destroy_handler;
import :comments_like_handler;
import :comments_unlike_handler;
import :comments_dislike_handler;
import :comments_undislike_handler;
import :comments_report_handler;
import :pages_bulk_destroy_handler;
import :pages_bulk_edit_handler;
import :pages_create_handler;
import :pages_edit_handler;
import :pages_destroy_handler;
import :pages_pages_id_copy_post_handler;
import :pages_gift_links_get_handler;
import :pages_gift_links_put_handler;
import :pages_gift_links_post_handler;
import :gift_links_gift_links_remove_all_put_handler;
import :integrations_browse_handler;
import :integrations_read_by_id_handler;
import :integrations_create_handler;
import :integrations_integrations_id_api_key_keyid_refresh_post_handler;
import :integrations_edit_handler;
import :integrations_destroy_handler;
import :schedules_put_detail_handler;
import :gifts_gifts_flush_reminders_put_handler;
import :gifts_gifts_flush_deliveries_put_handler;
import :settings_settings_routes_yaml_get_handler;
import :settings_settings_routes_yaml_post_handler;
import :settings_bulk_edit_handler;
import :settings_settings_access_code_regenerate_post_handler;
import :settings_settings_verifications_put_handler;
import :settings_settings_stripe_connect_delete_handler;
import :users_browse_handler;
import :users_read_by_id_handler;
import :users_read_by_slug_handler;
import :users_read_by_email_handler;
import :users_users_id_token_get_handler;
import :users_users_password_put_handler;
import :users_users_owner_put_handler;
import :users_edit_handler;
import :users_users_id_token_put_handler;
import :users_destroy_handler;
import :tags_create_handler;
import :tags_edit_handler;
import :tags_destroy_handler;
import :tiers_create_handler;
import :tiers_read_by_id_handler;
import :tiers_edit_handler;
import :stripe_stripe_checkout_config_get_handler;
import :stripe_stripe_checkout_config_put_handler;
import :stripe_stripe_checkout_preview_post_handler;
import :stripe_stripe_checkout_branding_get_handler;
import :members_browse_handler;
import :members_create_handler;
import :members_bulk_destroy_handler;
import :members_bulk_edit_handler;
import :offers_browse_handler;
import :offers_create_handler;
import :offers_edit_handler;
import :members_members_stats_count_get_handler;
import :members_members_stats_mrr_get_handler;
import :members_members_events_get_handler;
import :members_members_upload_get_handler;
import :members_members_upload_post_handler;
import :members_members_stripe_connect_get_handler;
import :members_members_metafields_all_handler;
import :apps_all_collection_handler;
import :apps_app_installations_browse_handler;
import :apps_app_installations_add_handler;
import :apps_app_installations_read_handler;
import :apps_app_installations_edit_handler;
import :apps_app_installations_destroy_handler;
import :apps_app_installation_previews_add_handler;
import :members_read_by_id_handler;
import :members_edit_handler;
import :members_destroy_handler;
import :members_members_id_sessions_delete_handler;
import :members_members_id_suppression_delete_handler;
import :members_subscriptions_post_handler;
import :members_put_detail_handler;
import :members_signin_urls_handler;
import :members_members_metafields_read_handler;
import :members_members_metafields_values_handler;
import :members_members_metafields_create_handler;
import :members_members_metafields_reorder_handler;
import :members_members_metafields_edit_handler;
import :members_members_metafields_destroy_handler;
import :members_account_read_handler;
import :members_account_update_handler;
import :members_account_destroy_suppression_handler;
import :members_members_id_commenting_disable_post_handler;
import :members_members_id_commenting_enable_post_handler;
import :stats_stats_member_count_get_handler;
import :stats_stats_mrr_get_handler;
import :stats_subscriptions_get_handler;
import :stats_stats_referrers_get_handler;
import :stats_stats_posts_id_stats_get_handler;
import :stats_stats_top_posts_get_handler;
import :stats_stats_top_posts_views_get_handler;
import :stats_stats_top_content_get_handler;
import :stats_stats_newsletter_stats_get_handler;
import :stats_stats_newsletter_basic_stats_get_handler;
import :stats_stats_newsletter_click_stats_get_handler;
import :stats_stats_subscriber_count_get_handler;
import :stats_stats_posts_id_top_referrers_get_handler;
import :stats_stats_posts_id_growth_get_handler;
import :stats_stats_top_sources_growth_get_handler;
import :stats_stats_posts_visitor_counts_post_handler;
import :stats_stats_posts_member_counts_post_handler;
import :labels_browse_handler;
import :labels_read_by_id_handler;
import :labels_read_by_slug_handler;
import :labels_create_handler;
import :labels_edit_handler;
import :labels_destroy_handler;
import :automations_browse_handler;
import :automations_automations_automation_id_actions_action_id_links_get_handler;
import :automations_read_by_id_handler;
import :automations_automations_id_performance_stats_get_handler;
import :automations_automations_id_runs_get_handler;
import :automations_get_detail_handler;
import :automations_automations_id_email_preview_post_handler;
import :automations_automations_id_email_test_post_handler;
import :automations_automations_poll_put_handler;
import :automations_create_handler;
import :automations_edit_handler;
import :automated_emails_browse_handler;
import :automated_emails_automated_emails_design_get_handler;
import :automated_emails_automated_emails_senders_put_handler;
import :automated_emails_automated_emails_verifications_put_handler;
import :automated_emails_read_by_id_handler;
import :automated_emails_create_handler;
import :automated_emails_automated_emails_design_put_handler;
import :automated_emails_edit_handler;
import :automated_emails_automated_emails_id_preview_post_handler;
import :automated_emails_automated_emails_id_test_post_handler;
import :roles_browse_handler;
import :slugs_get_detail_handler;
import :themes_browse_handler;
import :themes_themes_name_download_get_handler;
import :themes_themes_active_get_handler;
import :themes_themes_upload_post_handler;
import :themes_themes_install_post_handler;
import :themes_themes_name_activate_put_handler;
import :themes_destroy_handler;
import :notifications_browse_handler;
import :notifications_create_handler;
import :notifications_destroy_handler;
import :db_browse_handler;
import :db_create_handler;
import :db_bulk_destroy_handler;
import :db_db_backup_post_handler;
import :db_db_media_inline_post_handler;
import :exports_exports_download_get_handler;
import :exports_create_handler;
import :slack_slack_test_post_handler;
import :tinybird_tinybird_token_get_handler;
import :featurebase_featurebase_token_get_handler;
import :session_create_handler;
import :session_bulk_destroy_handler;
import :session_session_verify_post_handler;
import :session_session_verify_put_handler;
import :identities_browse_handler;
import :authentication_authentication_password_reset_post_handler;
import :authentication_authentication_password_reset_put_handler;
import :authentication_authentication_invitation_post_handler;
import :authentication_authentication_invitation_get_handler;
import :authentication_authentication_setup_post_handler;
import :authentication_authentication_setup_put_handler;
import :authentication_authentication_setup_get_handler;
import :authentication_authentication_reset_post_handler;
import :images_images_upload_post_handler;
import :media_media_upload_post_handler;
import :media_media_thumbnail_upload_put_handler;
import :files_files_upload_post_handler;
import :invites_browse_handler;
import :invites_read_by_id_handler;
import :invites_create_handler;
import :invites_destroy_handler;
import :redirects_redirects_download_get_handler;
import :redirects_redirects_upload_post_handler;
import :upgrades_browse_handler;
import :upgrades_create_handler;
import :upgrades_read_by_id_handler;
import :webhooks_create_handler;
import :webhooks_edit_handler;
import :webhooks_destroy_handler;
import :oembed_browse_handler;
import :actions_browse_handler;
import :email_previews_get_detail_handler;
import :email_previews_post_detail_handler;
import :emails_browse_handler;
import :emails_read_by_id_handler;
import :emails_emails_id_status_get_handler;
import :emails_emails_id_retry_put_handler;
import :emails_batches_get_handler;
import :emails_recipient_failures_get_handler;
import :emails_analytics_get_handler;
import :emails_analytics_put_handler;
import :emails_analytics_delete_handler;
import :snippets_browse_handler;
import :snippets_read_by_id_handler;
import :snippets_create_handler;
import :snippets_edit_handler;
import :snippets_destroy_handler;
import :custom_theme_settings_browse_handler;
import :custom_theme_settings_bulk_edit_handler;
import :newsletters_read_by_id_handler;
import :newsletters_create_handler;
import :newsletters_newsletters_verifications_put_handler;
import :newsletters_edit_handler;
import :links_browse_handler;
import :links_bulk_edit_handler;
import :recommendations_read_by_id_handler;
import :recommendations_create_handler;
import :recommendations_recommendations_check_post_handler;
import :recommendations_edit_handler;
import :recommendations_destroy_handler;
import :incoming_recommendations_browse_handler;
import :feedback_read_by_id_handler;
import :feedback_add_handler;
import :search_index_search_index_pages_get_handler;
import :search_index_search_index_users_get_handler;

import :posts_browse_handler;
import :posts_bulk_destroy_handler;
import :posts_bulk_edit_handler;
import :posts_copy_handler;
import :posts_create_handler;
import :posts_destroy_handler;
import :posts_edit_handler;
import :posts_export_handler;
import :posts_gift_links_browse_handler;
import :posts_gift_links_create_handler;
import :posts_gift_links_edit_handler;
import :posts_read_by_id_handler;
import :posts_read_by_slug_handler;
import :posts_upload_handler;

import :stripe;

export namespace blog::api {



/** @brief Owns the shared core handler plus one domain handler per resource. */
class BlogHandlers
{
public:
    /** @brief Owns a fresh context (tests / standalone use). */
    BlogHandlers() :
        m_owned_context{std::make_unique<blog::BlogContext>()},
        m_ctx{*m_owned_context}
    {
        blog::handlers::middleware::MiddlewareContextRegistry::set_active(&m_ctx);
    }

    /** @brief Use an injected, externally-owned context (plugin load). */
    explicit BlogHandlers(blog::BlogContext& context) :
        m_ctx{context}
    {
        blog::handlers::middleware::MiddlewareContextRegistry::set_active(&m_ctx);
    }

    [[nodiscard]] blog::BlogContext& context() noexcept
    {
        return m_ctx;
    }

    [[nodiscard]] blog::handlers::posts::browse::PostsBrowseHandler& posts_browse() noexcept
    {
        return m_posts_browse;
    }

    [[nodiscard]] blog::handlers::posts::read_by_id::PostsReadByIdHandler& posts_read_by_id() noexcept
    {
        return m_posts_read_by_id;
    }

    [[nodiscard]] blog::handlers::posts::read_by_slug::PostsReadBySlugHandler& posts_read_by_slug() noexcept
    {
        return m_posts_read_by_slug;
    }

    [[nodiscard]] blog::handlers::posts::export_op::PostsExportHandler& posts_export() noexcept
    {
        return m_posts_export;
    }

    [[nodiscard]] blog::handlers::posts::create::PostsCreateHandler& posts_create() noexcept
    {
        return m_posts_create;
    }

    [[nodiscard]] blog::handlers::posts::bulk_destroy::PostsBulkDestroyHandler& posts_bulk_destroy() noexcept
    {
        return m_posts_bulk_destroy;
    }

    [[nodiscard]] blog::handlers::posts::bulk_edit::PostsBulkEditHandler& posts_bulk_edit() noexcept
    {
        return m_posts_bulk_edit;
    }

    [[nodiscard]] blog::handlers::posts::upload::PostsUploadHandler& posts_upload() noexcept
    {
        return m_posts_upload;
    }

    [[nodiscard]] blog::handlers::posts::edit::PostsEditHandler& posts_edit() noexcept
    {
        return m_posts_edit;
    }

    [[nodiscard]] blog::handlers::posts::destroy::PostsDestroyHandler& posts_destroy() noexcept
    {
        return m_posts_destroy;
    }

    [[nodiscard]] blog::handlers::posts::copy::PostsCopyHandler& posts_copy() noexcept
    {
        return m_posts_copy;
    }

    [[nodiscard]] blog::handlers::posts::gift_links_browse::PostsGiftLinksBrowseHandler& posts_gift_links_browse() noexcept
    {
        return m_posts_gift_links_browse;
    }

    [[nodiscard]] blog::handlers::posts::gift_links_edit::PostsGiftLinksEditHandler& posts_gift_links_edit() noexcept
    {
        return m_posts_gift_links_edit;
    }

    [[nodiscard]] blog::handlers::posts::gift_links_create::PostsGiftLinksCreateHandler& posts_gift_links_create() noexcept
    {
        return m_posts_gift_links_create;
    }

 [[nodiscard]] blog::handlers::pages::browse::BrowseHandler& pages_browse() noexcept { return m_pages_browse; }
 [[nodiscard]] blog::handlers::pages::read_by_id::ReadByIdHandler& pages_read_by_id() noexcept { return m_pages_read_by_id; }
 [[nodiscard]] blog::handlers::pages::read_by_slug::ReadBySlugHandler& pages_read_by_slug() noexcept { return m_pages_read_by_slug; }
 [[nodiscard]] blog::handlers::authors::browse::BrowseHandler& authors_browse() noexcept { return m_authors_browse; }
 [[nodiscard]] blog::handlers::authors::read_by_id::ReadByIdHandler& authors_read_by_id() noexcept { return m_authors_read_by_id; }
 [[nodiscard]] blog::handlers::authors::read_by_slug::ReadBySlugHandler& authors_read_by_slug() noexcept { return m_authors_read_by_slug; }
 [[nodiscard]] blog::handlers::tags::browse::BrowseHandler& tags_browse() noexcept { return m_tags_browse; }
 [[nodiscard]] blog::handlers::tags::read_by_id::ReadByIdHandler& tags_read_by_id() noexcept { return m_tags_read_by_id; }
 [[nodiscard]] blog::handlers::tags::read_by_slug::ReadBySlugHandler& tags_read_by_slug() noexcept { return m_tags_read_by_slug; }
 [[nodiscard]] blog::handlers::settings::browse::BrowseHandler& settings_browse() noexcept { return m_settings_browse; }
 [[nodiscard]] blog::handlers::newsletters::browse::BrowseHandler& newsletters_browse() noexcept { return m_newsletters_browse; }
 [[nodiscard]] blog::handlers::tiers::browse::BrowseHandler& tiers_browse() noexcept { return m_tiers_browse; }
 [[nodiscard]] blog::handlers::offers::read_by_id::ReadByIdHandler& offers_read_by_id() noexcept { return m_offers_read_by_id; }
 [[nodiscard]] blog::handlers::recommendations::browse::BrowseHandler& recommendations_browse() noexcept { return m_recommendations_browse; }
 [[nodiscard]] blog::handlers::search_index::search_index_posts_get::SearchIndexPostsGetHandler& search_index_search_index_posts_get() noexcept { return m_search_index_search_index_posts_get; }
 [[nodiscard]] blog::handlers::search_index::search_index_authors_get::SearchIndexAuthorsGetHandler& search_index_search_index_authors_get() noexcept { return m_search_index_search_index_authors_get; }
 [[nodiscard]] blog::handlers::search_index::search_index_tags_get::SearchIndexTagsGetHandler& search_index_search_index_tags_get() noexcept { return m_search_index_search_index_tags_get; }
 [[nodiscard]] blog::handlers::site::browse::BrowseHandler& site_browse() noexcept { return m_site_browse; }
 [[nodiscard]] blog::handlers::config::browse::BrowseHandler& config_browse() noexcept { return m_config_browse; }
 [[nodiscard]] blog::handlers::config::browse::BrowseHandler& config_read() noexcept { return m_config_browse; }
 [[nodiscard]] blog::handlers::config::config_featurebase_get::ConfigFeaturebaseGetHandler& config_config_featurebase_get() noexcept { return m_config_config_featurebase_get; }
 [[nodiscard]] blog::handlers::mentions::browse::BrowseHandler& mentions_browse() noexcept { return m_mentions_browse; }
 [[nodiscard]] blog::handlers::mentions::receive::ReceiveHandler& mentions_receive() noexcept { return m_mentions_receive; }
 [[nodiscard]] blog::handlers::comments::browse::BrowseHandler& comments_browse() noexcept { return m_comments_browse; }
 [[nodiscard]] blog::handlers::comments::read_by_id::ReadByIdHandler& comments_read_by_id() noexcept { return m_comments_read_by_id; }
 [[nodiscard]] blog::handlers::comments::comment_replies::CommentRepliesHandler& comments_comment_replies() noexcept { return m_comments_comment_replies; }
 [[nodiscard]] blog::handlers::comments::comment_reports::CommentReportsHandler& comments_comment_reports() noexcept { return m_comments_comment_reports; }
 [[nodiscard]] blog::handlers::comments::comment_likes::CommentLikesHandler& comments_comment_likes() noexcept { return m_comments_comment_likes; }
 [[nodiscard]] blog::handlers::comments::comment_dislikes::CommentDislikesHandler& comments_comment_dislikes() noexcept { return m_comments_comment_dislikes; }
 [[nodiscard]] blog::handlers::comments::get_detail::GetDetailHandler& comments_get_detail() noexcept { return m_comments_get_detail; }
 [[nodiscard]] blog::handlers::comments::create::CreateHandler& comments_create() noexcept { return m_comments_create; }
 [[nodiscard]] blog::handlers::comments::edit::EditHandler& comments_edit() noexcept { return m_comments_edit; }
 [[nodiscard]] blog::handlers::comments::counts::CountsHandler& comments_counts() noexcept { return m_comments_counts; }
 [[nodiscard]] blog::handlers::comments::destroy::DestroyHandler& comments_destroy() noexcept { return m_comments_destroy; }
 [[nodiscard]] blog::handlers::comments::like::LikeHandler& comments_like() noexcept { return m_comments_like; }
 [[nodiscard]] blog::handlers::comments::unlike::UnlikeHandler& comments_unlike() noexcept { return m_comments_unlike; }
 [[nodiscard]] blog::handlers::comments::dislike::DislikeHandler& comments_dislike() noexcept { return m_comments_dislike; }
 [[nodiscard]] blog::handlers::comments::undislike::UndislikeHandler& comments_undislike() noexcept { return m_comments_undislike; }
 [[nodiscard]] blog::handlers::comments::report::ReportHandler& comments_report() noexcept { return m_comments_report; }
 [[nodiscard]] blog::handlers::pages::bulk_destroy::BulkDestroyHandler& pages_bulk_destroy() noexcept { return m_pages_bulk_destroy; }
 [[nodiscard]] blog::handlers::pages::bulk_edit::BulkEditHandler& pages_bulk_edit() noexcept { return m_pages_bulk_edit; }
 [[nodiscard]] blog::handlers::pages::create::CreateHandler& pages_create() noexcept { return m_pages_create; }
 [[nodiscard]] blog::handlers::pages::edit::EditHandler& pages_edit() noexcept { return m_pages_edit; }
 [[nodiscard]] blog::handlers::pages::destroy::DestroyHandler& pages_destroy() noexcept { return m_pages_destroy; }
 [[nodiscard]] blog::handlers::pages::pages_id_copy_post::PagesIdCopyPostHandler& pages_pages_id_copy_post() noexcept { return m_pages_pages_id_copy_post; }
 [[nodiscard]] blog::handlers::pages::gift_links_get::GiftLinksGetHandler& pages_gift_links_get() noexcept { return m_pages_gift_links_get; }
 [[nodiscard]] blog::handlers::pages::gift_links_put::GiftLinksPutHandler& pages_gift_links_put() noexcept { return m_pages_gift_links_put; }
 [[nodiscard]] blog::handlers::pages::gift_links_post::GiftLinksPostHandler& pages_gift_links_post() noexcept { return m_pages_gift_links_post; }
 [[nodiscard]] blog::handlers::gift_links::gift_links_remove_all_put::GiftLinksRemoveAllPutHandler& gift_links_gift_links_remove_all_put() noexcept { return m_gift_links_gift_links_remove_all_put; }
 [[nodiscard]] blog::handlers::integrations::browse::BrowseHandler& integrations_browse() noexcept { return m_integrations_browse; }
 [[nodiscard]] blog::handlers::integrations::read_by_id::ReadByIdHandler& integrations_read_by_id() noexcept { return m_integrations_read_by_id; }
 [[nodiscard]] blog::handlers::integrations::create::CreateHandler& integrations_create() noexcept { return m_integrations_create; }
 [[nodiscard]] blog::handlers::integrations::integrations_id_api_key_keyid_refresh_post::IntegrationsIdApiKeyKeyidRefreshPostHandler& integrations_integrations_id_api_key_keyid_refresh_post() noexcept { return m_integrations_integrations_id_api_key_keyid_refresh_post; }
 [[nodiscard]] blog::handlers::integrations::edit::EditHandler& integrations_edit() noexcept { return m_integrations_edit; }
 [[nodiscard]] blog::handlers::integrations::destroy::DestroyHandler& integrations_destroy() noexcept { return m_integrations_destroy; }
 [[nodiscard]] blog::handlers::schedules::put_detail::PutDetailHandler& schedules_put_detail() noexcept { return m_schedules_put_detail; }
 [[nodiscard]] blog::handlers::gifts::gifts_flush_reminders_put::GiftsFlushRemindersPutHandler& gifts_gifts_flush_reminders_put() noexcept { return m_gifts_gifts_flush_reminders_put; }
 [[nodiscard]] blog::handlers::gifts::gifts_flush_deliveries_put::GiftsFlushDeliveriesPutHandler& gifts_gifts_flush_deliveries_put() noexcept { return m_gifts_gifts_flush_deliveries_put; }
 [[nodiscard]] blog::handlers::settings::settings_routes_yaml_get::SettingsRoutesYamlGetHandler& settings_settings_routes_yaml_get() noexcept { return m_settings_settings_routes_yaml_get; }
 [[nodiscard]] blog::handlers::settings::settings_routes_yaml_post::SettingsRoutesYamlPostHandler& settings_settings_routes_yaml_post() noexcept { return m_settings_settings_routes_yaml_post; }
 [[nodiscard]] blog::handlers::settings::bulk_edit::BulkEditHandler& settings_bulk_edit() noexcept { return m_settings_bulk_edit; }
 [[nodiscard]] blog::handlers::settings::settings_access_code_regenerate_post::SettingsAccessCodeRegeneratePostHandler& settings_settings_access_code_regenerate_post() noexcept { return m_settings_settings_access_code_regenerate_post; }
 [[nodiscard]] blog::handlers::settings::settings_verifications_put::SettingsVerificationsPutHandler& settings_settings_verifications_put() noexcept { return m_settings_settings_verifications_put; }
 [[nodiscard]] blog::handlers::settings::settings_stripe_connect_delete::SettingsStripeConnectDeleteHandler& settings_settings_stripe_connect_delete() noexcept { return m_settings_settings_stripe_connect_delete; }
 [[nodiscard]] blog::handlers::users::browse::BrowseHandler& users_browse() noexcept { return m_users_browse; }
 [[nodiscard]] blog::handlers::users::read_by_id::ReadByIdHandler& users_read_by_id() noexcept { return m_users_read_by_id; }
 [[nodiscard]] blog::handlers::users::read_by_slug::ReadBySlugHandler& users_read_by_slug() noexcept { return m_users_read_by_slug; }
 [[nodiscard]] blog::handlers::users::read_by_email::ReadByEmailHandler& users_read_by_email() noexcept { return m_users_read_by_email; }
 [[nodiscard]] blog::handlers::users::users_id_token_get::UsersIdTokenGetHandler& users_users_id_token_get() noexcept { return m_users_users_id_token_get; }
 [[nodiscard]] blog::handlers::users::users_password_put::UsersPasswordPutHandler& users_users_password_put() noexcept { return m_users_users_password_put; }
 [[nodiscard]] blog::handlers::users::users_owner_put::UsersOwnerPutHandler& users_users_owner_put() noexcept { return m_users_users_owner_put; }
 [[nodiscard]] blog::handlers::users::edit::EditHandler& users_edit() noexcept { return m_users_edit; }
 [[nodiscard]] blog::handlers::users::users_id_token_put::UsersIdTokenPutHandler& users_users_id_token_put() noexcept { return m_users_users_id_token_put; }
 [[nodiscard]] blog::handlers::users::destroy::DestroyHandler& users_destroy() noexcept { return m_users_destroy; }
 [[nodiscard]] blog::handlers::tags::create::CreateHandler& tags_create() noexcept { return m_tags_create; }
 [[nodiscard]] blog::handlers::tags::edit::EditHandler& tags_edit() noexcept { return m_tags_edit; }
 [[nodiscard]] blog::handlers::tags::destroy::DestroyHandler& tags_destroy() noexcept { return m_tags_destroy; }
 [[nodiscard]] blog::handlers::tiers::create::CreateHandler& tiers_create() noexcept { return m_tiers_create; }
 [[nodiscard]] blog::handlers::tiers::read_by_id::ReadByIdHandler& tiers_read_by_id() noexcept { return m_tiers_read_by_id; }
 [[nodiscard]] blog::handlers::tiers::edit::EditHandler& tiers_edit() noexcept { return m_tiers_edit; }
 [[nodiscard]] blog::handlers::stripe::stripe_checkout_config_get::StripeCheckoutConfigGetHandler& stripe_stripe_checkout_config_get() noexcept { return m_stripe_stripe_checkout_config_get; }
 [[nodiscard]] blog::handlers::stripe::stripe_checkout_config_put::StripeCheckoutConfigPutHandler& stripe_stripe_checkout_config_put() noexcept { return m_stripe_stripe_checkout_config_put; }
 [[nodiscard]] blog::handlers::stripe::stripe_checkout_preview_post::StripeCheckoutPreviewPostHandler& stripe_stripe_checkout_preview_post() noexcept { return m_stripe_stripe_checkout_preview_post; }
 [[nodiscard]] blog::handlers::stripe::stripe_checkout_branding_get::StripeCheckoutBrandingGetHandler& stripe_stripe_checkout_branding_get() noexcept { return m_stripe_stripe_checkout_branding_get; }
 [[nodiscard]] blog::handlers::members::browse::BrowseHandler& members_browse() noexcept { return m_members_browse; }
 [[nodiscard]] blog::handlers::members::create::CreateHandler& members_create() noexcept { return m_members_create; }
 [[nodiscard]] blog::handlers::members::bulk_destroy::BulkDestroyHandler& members_bulk_destroy() noexcept { return m_members_bulk_destroy; }
 [[nodiscard]] blog::handlers::members::bulk_edit::BulkEditHandler& members_bulk_edit() noexcept { return m_members_bulk_edit; }
 [[nodiscard]] blog::handlers::offers::browse::BrowseHandler& offers_browse() noexcept { return m_offers_browse; }
 [[nodiscard]] blog::handlers::offers::create::CreateHandler& offers_create() noexcept { return m_offers_create; }
 [[nodiscard]] blog::handlers::offers::edit::EditHandler& offers_edit() noexcept { return m_offers_edit; }
 [[nodiscard]] blog::handlers::members::members_stats_count_get::MembersStatsCountGetHandler& members_members_stats_count_get() noexcept { return m_members_members_stats_count_get; }
 [[nodiscard]] blog::handlers::members::members_stats_mrr_get::MembersStatsMrrGetHandler& members_members_stats_mrr_get() noexcept { return m_members_members_stats_mrr_get; }
 [[nodiscard]] blog::handlers::members::members_events_get::MembersEventsGetHandler& members_members_events_get() noexcept { return m_members_members_events_get; }
 [[nodiscard]] blog::handlers::members::members_upload_get::MembersUploadGetHandler& members_members_upload_get() noexcept { return m_members_members_upload_get; }
 [[nodiscard]] blog::handlers::members::members_upload_post::MembersUploadPostHandler& members_members_upload_post() noexcept { return m_members_members_upload_post; }
 [[nodiscard]] blog::handlers::members::members_stripe_connect_get::MembersStripeConnectGetHandler& members_members_stripe_connect_get() noexcept { return m_members_members_stripe_connect_get; }
 [[nodiscard]] blog::handlers::members::members_metafields_all::MembersMetafieldsAllHandler& members_members_metafields_all() noexcept { return m_members_members_metafields_all; }
 [[nodiscard]] blog::handlers::apps::all_collection::AllCollectionHandler& apps_all_collection() noexcept { return m_apps_all_collection; }
 [[nodiscard]] blog::handlers::apps::app_installations_browse::AppInstallationsBrowseHandler& apps_app_installations_browse() noexcept { return m_apps_app_installations_browse; }
 [[nodiscard]] blog::handlers::apps::app_installations_add::AppInstallationsAddHandler& apps_app_installations_add() noexcept { return m_apps_app_installations_add; }
 [[nodiscard]] blog::handlers::apps::app_installations_read::AppInstallationsReadHandler& apps_app_installations_read() noexcept { return m_apps_app_installations_read; }
 [[nodiscard]] blog::handlers::apps::app_installations_edit::AppInstallationsEditHandler& apps_app_installations_edit() noexcept { return m_apps_app_installations_edit; }
 [[nodiscard]] blog::handlers::apps::app_installations_destroy::AppInstallationsDestroyHandler& apps_app_installations_destroy() noexcept { return m_apps_app_installations_destroy; }
 [[nodiscard]] blog::handlers::apps::app_installation_previews_add::AppInstallationPreviewsAddHandler& apps_app_installation_previews_add() noexcept { return m_apps_app_installation_previews_add; }
 [[nodiscard]] blog::handlers::members::read_by_id::ReadByIdHandler& members_read_by_id() noexcept { return m_members_read_by_id; }
 [[nodiscard]] blog::handlers::members::edit::EditHandler& members_edit() noexcept { return m_members_edit; }
 [[nodiscard]] blog::handlers::members::destroy::DestroyHandler& members_destroy() noexcept { return m_members_destroy; }
 [[nodiscard]] blog::handlers::members::members_id_sessions_delete::MembersIdSessionsDeleteHandler& members_members_id_sessions_delete() noexcept { return m_members_members_id_sessions_delete; }
 [[nodiscard]] blog::handlers::members::members_id_suppression_delete::MembersIdSuppressionDeleteHandler& members_members_id_suppression_delete() noexcept { return m_members_members_id_suppression_delete; }
 [[nodiscard]] blog::handlers::members::subscriptions_post::SubscriptionsPostHandler& members_subscriptions_post() noexcept { return m_members_subscriptions_post; }
 [[nodiscard]] blog::handlers::members::put_detail::PutDetailHandler& members_put_detail() noexcept { return m_members_put_detail; }
 [[nodiscard]] blog::handlers::members::signin_urls::SigninUrlsHandler& members_signin_urls() noexcept { return m_members_signin_urls; }
 [[nodiscard]] blog::handlers::members::members_metafields_read::MembersMetafieldsReadHandler& members_metafields_read() noexcept { return m_members_metafields_read; }
 [[nodiscard]] blog::handlers::members::members_metafields_values::MembersMetafieldsValuesHandler& members_metafields_values() noexcept { return m_members_metafields_values; }
 [[nodiscard]] blog::handlers::members::members_metafields_create::MembersMetafieldsCreateHandler& members_metafields_create() noexcept { return m_members_metafields_create; }
 [[nodiscard]] blog::handlers::members::members_metafields_reorder::MembersMetafieldsReorderHandler& members_metafields_reorder() noexcept { return m_members_metafields_reorder; }
 [[nodiscard]] blog::handlers::members::members_metafields_edit::MembersMetafieldsEditHandler& members_metafields_edit() noexcept { return m_members_metafields_edit; }
 [[nodiscard]] blog::handlers::members::members_metafields_destroy::MembersMetafieldsDestroyHandler& members_metafields_destroy() noexcept { return m_members_metafields_destroy; }
 [[nodiscard]] blog::handlers::members::members_account::read::MembersAccountReadHandler& members_account_read() noexcept { return m_members_account_read; }
 [[nodiscard]] blog::handlers::members::members_account::update::MembersAccountUpdateHandler& members_account_update() noexcept { return m_members_account_update; }
 [[nodiscard]] blog::handlers::members::members_account::destroy_suppression::MembersAccountDestroySuppressionHandler& members_account_destroy_suppression() noexcept { return m_members_account_destroy_suppression; }
 [[nodiscard]] blog::handlers::members::members_id_commenting_disable_post::MembersIdCommentingDisablePostHandler& members_members_id_commenting_disable_post() noexcept { return m_members_members_id_commenting_disable_post; }
 [[nodiscard]] blog::handlers::members::members_id_commenting_enable_post::MembersIdCommentingEnablePostHandler& members_members_id_commenting_enable_post() noexcept { return m_members_members_id_commenting_enable_post; }
 [[nodiscard]] blog::handlers::stats::stats_member_count_get::StatsMemberCountGetHandler& stats_stats_member_count_get() noexcept { return m_stats_stats_member_count_get; }
 [[nodiscard]] blog::handlers::stats::stats_mrr_get::StatsMrrGetHandler& stats_stats_mrr_get() noexcept { return m_stats_stats_mrr_get; }
 [[nodiscard]] blog::handlers::stats::subscriptions_get::SubscriptionsGetHandler& stats_subscriptions_get() noexcept { return m_stats_subscriptions_get; }
 [[nodiscard]] blog::handlers::stats::stats_referrers_get::StatsReferrersGetHandler& stats_stats_referrers_get() noexcept { return m_stats_stats_referrers_get; }
 [[nodiscard]] blog::handlers::stats::stats_posts_id_stats_get::StatsPostsIdStatsGetHandler& stats_stats_posts_id_stats_get() noexcept { return m_stats_stats_posts_id_stats_get; }
 [[nodiscard]] blog::handlers::stats::stats_top_posts_get::StatsTopPostsGetHandler& stats_stats_top_posts_get() noexcept { return m_stats_stats_top_posts_get; }
 [[nodiscard]] blog::handlers::stats::stats_top_posts_views_get::StatsTopPostsViewsGetHandler& stats_stats_top_posts_views_get() noexcept { return m_stats_stats_top_posts_views_get; }
 [[nodiscard]] blog::handlers::stats::stats_top_content_get::StatsTopContentGetHandler& stats_stats_top_content_get() noexcept { return m_stats_stats_top_content_get; }
 [[nodiscard]] blog::handlers::stats::stats_newsletter_stats_get::StatsNewsletterStatsGetHandler& stats_stats_newsletter_stats_get() noexcept { return m_stats_stats_newsletter_stats_get; }
 [[nodiscard]] blog::handlers::stats::stats_newsletter_basic_stats_get::StatsNewsletterBasicStatsGetHandler& stats_stats_newsletter_basic_stats_get() noexcept { return m_stats_stats_newsletter_basic_stats_get; }
 [[nodiscard]] blog::handlers::stats::stats_newsletter_click_stats_get::StatsNewsletterClickStatsGetHandler& stats_stats_newsletter_click_stats_get() noexcept { return m_stats_stats_newsletter_click_stats_get; }
 [[nodiscard]] blog::handlers::stats::stats_subscriber_count_get::StatsSubscriberCountGetHandler& stats_stats_subscriber_count_get() noexcept { return m_stats_stats_subscriber_count_get; }
 [[nodiscard]] blog::handlers::stats::stats_posts_id_top_referrers_get::StatsPostsIdTopReferrersGetHandler& stats_stats_posts_id_top_referrers_get() noexcept { return m_stats_stats_posts_id_top_referrers_get; }
 [[nodiscard]] blog::handlers::stats::stats_posts_id_growth_get::StatsPostsIdGrowthGetHandler& stats_stats_posts_id_growth_get() noexcept { return m_stats_stats_posts_id_growth_get; }
 [[nodiscard]] blog::handlers::stats::stats_top_sources_growth_get::StatsTopSourcesGrowthGetHandler& stats_stats_top_sources_growth_get() noexcept { return m_stats_stats_top_sources_growth_get; }
 [[nodiscard]] blog::handlers::stats::stats_posts_visitor_counts_post::StatsPostsVisitorCountsPostHandler& stats_stats_posts_visitor_counts_post() noexcept { return m_stats_stats_posts_visitor_counts_post; }
 [[nodiscard]] blog::handlers::stats::stats_posts_member_counts_post::StatsPostsMemberCountsPostHandler& stats_stats_posts_member_counts_post() noexcept { return m_stats_stats_posts_member_counts_post; }
 [[nodiscard]] blog::handlers::labels::browse::BrowseHandler& labels_browse() noexcept { return m_labels_browse; }
 [[nodiscard]] blog::handlers::labels::read_by_id::ReadByIdHandler& labels_read_by_id() noexcept { return m_labels_read_by_id; }
 [[nodiscard]] blog::handlers::labels::read_by_slug::ReadBySlugHandler& labels_read_by_slug() noexcept { return m_labels_read_by_slug; }
 [[nodiscard]] blog::handlers::labels::create::CreateHandler& labels_create() noexcept { return m_labels_create; }
 [[nodiscard]] blog::handlers::labels::edit::EditHandler& labels_edit() noexcept { return m_labels_edit; }
 [[nodiscard]] blog::handlers::labels::destroy::DestroyHandler& labels_destroy() noexcept { return m_labels_destroy; }
 [[nodiscard]] blog::handlers::automations::browse::BrowseHandler& automations_browse() noexcept { return m_automations_browse; }
 [[nodiscard]] blog::handlers::automations::automations_automation_id_actions_action_id_links_get::AutomationsAutomationIdActionsActionIdLinksGetHandler& automations_automations_automation_id_actions_action_id_links_get() noexcept { return m_automations_automations_automation_id_actions_action_id_links_get; }
 [[nodiscard]] blog::handlers::automations::read_by_id::ReadByIdHandler& automations_read_by_id() noexcept { return m_automations_read_by_id; }
 [[nodiscard]] blog::handlers::automations::automations_id_performance_stats_get::AutomationsIdPerformanceStatsGetHandler& automations_automations_id_performance_stats_get() noexcept { return m_automations_automations_id_performance_stats_get; }
 [[nodiscard]] blog::handlers::automations::automations_id_runs_get::AutomationsIdRunsGetHandler& automations_automations_id_runs_get() noexcept { return m_automations_automations_id_runs_get; }
 [[nodiscard]] blog::handlers::automations::get_detail::GetDetailHandler& automations_get_detail() noexcept { return m_automations_get_detail; }
 [[nodiscard]] blog::handlers::automations::automations_id_email_preview_post::AutomationsIdEmailPreviewPostHandler& automations_automations_id_email_preview_post() noexcept { return m_automations_automations_id_email_preview_post; }
 [[nodiscard]] blog::handlers::automations::automations_id_email_test_post::AutomationsIdEmailTestPostHandler& automations_automations_id_email_test_post() noexcept { return m_automations_automations_id_email_test_post; }
 [[nodiscard]] blog::handlers::automations::automations_poll_put::AutomationsPollPutHandler& automations_automations_poll_put() noexcept { return m_automations_automations_poll_put; }
 [[nodiscard]] blog::handlers::automations::create::CreateHandler& automations_create() noexcept { return m_automations_create; }
 [[nodiscard]] blog::handlers::automations::edit::EditHandler& automations_edit() noexcept { return m_automations_edit; }
 [[nodiscard]] blog::handlers::automated_emails::browse::BrowseHandler& automated_emails_browse() noexcept { return m_automated_emails_browse; }
 [[nodiscard]] blog::handlers::automated_emails::automated_emails_design_get::AutomatedEmailsDesignGetHandler& automated_emails_automated_emails_design_get() noexcept { return m_automated_emails_automated_emails_design_get; }
 [[nodiscard]] blog::handlers::automated_emails::automated_emails_senders_put::AutomatedEmailsSendersPutHandler& automated_emails_automated_emails_senders_put() noexcept { return m_automated_emails_automated_emails_senders_put; }
 [[nodiscard]] blog::handlers::automated_emails::automated_emails_verifications_put::AutomatedEmailsVerificationsPutHandler& automated_emails_automated_emails_verifications_put() noexcept { return m_automated_emails_automated_emails_verifications_put; }
 [[nodiscard]] blog::handlers::automated_emails::read_by_id::ReadByIdHandler& automated_emails_read_by_id() noexcept { return m_automated_emails_read_by_id; }
 [[nodiscard]] blog::handlers::automated_emails::create::CreateHandler& automated_emails_create() noexcept { return m_automated_emails_create; }
 [[nodiscard]] blog::handlers::automated_emails::automated_emails_design_put::AutomatedEmailsDesignPutHandler& automated_emails_automated_emails_design_put() noexcept { return m_automated_emails_automated_emails_design_put; }
 [[nodiscard]] blog::handlers::automated_emails::edit::EditHandler& automated_emails_edit() noexcept { return m_automated_emails_edit; }
 [[nodiscard]] blog::handlers::automated_emails::automated_emails_id_preview_post::AutomatedEmailsIdPreviewPostHandler& automated_emails_automated_emails_id_preview_post() noexcept { return m_automated_emails_automated_emails_id_preview_post; }
 [[nodiscard]] blog::handlers::automated_emails::automated_emails_id_test_post::AutomatedEmailsIdTestPostHandler& automated_emails_automated_emails_id_test_post() noexcept { return m_automated_emails_automated_emails_id_test_post; }
 [[nodiscard]] blog::handlers::roles::browse::BrowseHandler& roles_browse() noexcept { return m_roles_browse; }
 [[nodiscard]] blog::handlers::slugs::get_detail::GetDetailHandler& slugs_get_detail() noexcept { return m_slugs_get_detail; }
 [[nodiscard]] blog::handlers::themes::browse::BrowseHandler& themes_browse() noexcept { return m_themes_browse; }
 [[nodiscard]] blog::handlers::themes::themes_name_download_get::ThemesNameDownloadGetHandler& themes_themes_name_download_get() noexcept { return m_themes_themes_name_download_get; }
 [[nodiscard]] blog::handlers::themes::themes_active_get::ThemesActiveGetHandler& themes_themes_active_get() noexcept { return m_themes_themes_active_get; }
 [[nodiscard]] blog::handlers::themes::themes_upload_post::ThemesUploadPostHandler& themes_themes_upload_post() noexcept { return m_themes_themes_upload_post; }
 [[nodiscard]] blog::handlers::themes::themes_install_post::ThemesInstallPostHandler& themes_themes_install_post() noexcept { return m_themes_themes_install_post; }
 [[nodiscard]] blog::handlers::themes::themes_name_activate_put::ThemesNameActivatePutHandler& themes_themes_name_activate_put() noexcept { return m_themes_themes_name_activate_put; }
 [[nodiscard]] blog::handlers::themes::destroy::DestroyHandler& themes_destroy() noexcept { return m_themes_destroy; }
 [[nodiscard]] blog::handlers::notifications::browse::BrowseHandler& notifications_browse() noexcept { return m_notifications_browse; }
 [[nodiscard]] blog::handlers::notifications::create::CreateHandler& notifications_create() noexcept { return m_notifications_create; }
 [[nodiscard]] blog::handlers::notifications::destroy::DestroyHandler& notifications_destroy() noexcept { return m_notifications_destroy; }
 [[nodiscard]] blog::handlers::db::browse::BrowseHandler& db_browse() noexcept { return m_db_browse; }
 [[nodiscard]] blog::handlers::db::create::CreateHandler& db_create() noexcept { return m_db_create; }
 [[nodiscard]] blog::handlers::db::bulk_destroy::BulkDestroyHandler& db_bulk_destroy() noexcept { return m_db_bulk_destroy; }
 [[nodiscard]] blog::handlers::db::db_backup_post::DbBackupPostHandler& db_db_backup_post() noexcept { return m_db_db_backup_post; }
 [[nodiscard]] blog::handlers::db::db_media_inline_post::DbMediaInlinePostHandler& db_db_media_inline_post() noexcept { return m_db_db_media_inline_post; }
 [[nodiscard]] blog::handlers::exports::exports_download_get::ExportsDownloadGetHandler& exports_exports_download_get() noexcept { return m_exports_exports_download_get; }
 [[nodiscard]] blog::handlers::exports::create::CreateHandler& exports_create() noexcept { return m_exports_create; }
 [[nodiscard]] blog::handlers::slack::slack_test_post::SlackTestPostHandler& slack_slack_test_post() noexcept { return m_slack_slack_test_post; }
 [[nodiscard]] blog::handlers::tinybird::tinybird_token_get::TinybirdTokenGetHandler& tinybird_tinybird_token_get() noexcept { return m_tinybird_tinybird_token_get; }
 [[nodiscard]] blog::handlers::featurebase::featurebase_token_get::FeaturebaseTokenGetHandler& featurebase_featurebase_token_get() noexcept { return m_featurebase_featurebase_token_get; }
 [[nodiscard]] blog::handlers::session::create::CreateHandler& session_create() noexcept { return m_session_create; }
 [[nodiscard]] blog::handlers::session::bulk_destroy::BulkDestroyHandler& session_bulk_destroy() noexcept { return m_session_bulk_destroy; }
 [[nodiscard]] blog::handlers::session::session_verify_post::SessionVerifyPostHandler& session_session_verify_post() noexcept { return m_session_session_verify_post; }
 [[nodiscard]] blog::handlers::session::session_verify_put::SessionVerifyPutHandler& session_session_verify_put() noexcept { return m_session_session_verify_put; }
 [[nodiscard]] blog::handlers::identities::browse::BrowseHandler& identities_browse() noexcept { return m_identities_browse; }
 [[nodiscard]] blog::handlers::identities::browse::BrowseHandler& identities_read() noexcept { return m_identities_browse; }
 [[nodiscard]] blog::handlers::authentication::authentication_password_reset_post::AuthenticationPasswordResetPostHandler& authentication_authentication_password_reset_post() noexcept { return m_authentication_authentication_password_reset_post; }
 [[nodiscard]] blog::handlers::authentication::authentication_password_reset_put::AuthenticationPasswordResetPutHandler& authentication_authentication_password_reset_put() noexcept { return m_authentication_authentication_password_reset_put; }
 [[nodiscard]] blog::handlers::authentication::authentication_invitation_post::AuthenticationInvitationPostHandler& authentication_authentication_invitation_post() noexcept { return m_authentication_authentication_invitation_post; }
 [[nodiscard]] blog::handlers::authentication::authentication_invitation_get::AuthenticationInvitationGetHandler& authentication_authentication_invitation_get() noexcept { return m_authentication_authentication_invitation_get; }
 [[nodiscard]] blog::handlers::authentication::authentication_setup_post::AuthenticationSetupPostHandler& authentication_authentication_setup_post() noexcept { return m_authentication_authentication_setup_post; }
 [[nodiscard]] blog::handlers::authentication::authentication_setup_put::AuthenticationSetupPutHandler& authentication_authentication_setup_put() noexcept { return m_authentication_authentication_setup_put; }
 [[nodiscard]] blog::handlers::authentication::authentication_setup_get::AuthenticationSetupGetHandler& authentication_authentication_setup_get() noexcept { return m_authentication_authentication_setup_get; }
 [[nodiscard]] blog::handlers::authentication::authentication_reset_post::AuthenticationResetPostHandler& authentication_authentication_reset_post() noexcept { return m_authentication_authentication_reset_post; }
 [[nodiscard]] blog::handlers::images::images_upload_post::ImagesUploadPostHandler& images_images_upload_post() noexcept { return m_images_images_upload_post; }
 [[nodiscard]] blog::handlers::media::media_upload_post::MediaUploadPostHandler& media_media_upload_post() noexcept { return m_media_media_upload_post; }
 [[nodiscard]] blog::handlers::media::media_thumbnail_upload_put::MediaThumbnailUploadPutHandler& media_media_thumbnail_upload_put() noexcept { return m_media_media_thumbnail_upload_put; }
 [[nodiscard]] blog::handlers::files::files_upload_post::FilesUploadPostHandler& files_files_upload_post() noexcept { return m_files_files_upload_post; }
 [[nodiscard]] blog::handlers::invites::browse::BrowseHandler& invites_browse() noexcept { return m_invites_browse; }
 [[nodiscard]] blog::handlers::invites::read_by_id::ReadByIdHandler& invites_read_by_id() noexcept { return m_invites_read_by_id; }
 [[nodiscard]] blog::handlers::invites::create::CreateHandler& invites_create() noexcept { return m_invites_create; }
 [[nodiscard]] blog::handlers::invites::destroy::DestroyHandler& invites_destroy() noexcept { return m_invites_destroy; }
 [[nodiscard]] blog::handlers::redirects::redirects_download_get::RedirectsDownloadGetHandler& redirects_redirects_download_get() noexcept { return m_redirects_redirects_download_get; }
 [[nodiscard]] blog::handlers::redirects::redirects_upload_post::RedirectsUploadPostHandler& redirects_redirects_upload_post() noexcept { return m_redirects_redirects_upload_post; }
 [[nodiscard]] blog::handlers::upgrades::browse::BrowseHandler& upgrades_browse() noexcept { return m_upgrades_browse; }
 [[nodiscard]] blog::handlers::upgrades::create::CreateHandler& upgrades_create() noexcept { return m_upgrades_create; }
 [[nodiscard]] blog::handlers::upgrades::read_by_id::ReadByIdHandler& upgrades_read_by_id() noexcept { return m_upgrades_read_by_id; }
 [[nodiscard]] blog::handlers::webhooks::create::CreateHandler& webhooks_create() noexcept { return m_webhooks_create; }
 [[nodiscard]] blog::handlers::webhooks::edit::EditHandler& webhooks_edit() noexcept { return m_webhooks_edit; }
 [[nodiscard]] blog::handlers::webhooks::destroy::DestroyHandler& webhooks_destroy() noexcept { return m_webhooks_destroy; }
 [[nodiscard]] blog::handlers::oembed::browse::BrowseHandler& oembed_browse() noexcept { return m_oembed_browse; }
 [[nodiscard]] blog::handlers::actions::browse::BrowseHandler& actions_browse() noexcept { return m_actions_browse; }
 [[nodiscard]] blog::handlers::announcements::announcement_get::AnnouncementGetHandler& announcements_announcement_get() noexcept { return m_announcements_announcement_get; }
 [[nodiscard]] blog::handlers::app_sessions::sessions_create::AppSessionsCreateHandler& app_sessions_sessions_create() noexcept { return m_app_sessions_sessions_create; }
 [[nodiscard]] blog::handlers::gifts_members::redeem_get::RedeemGetHandler& gifts_members_redeem_get() noexcept { return m_gifts_members_redeem_get; }
 [[nodiscard]] blog::handlers::gifts_members::redeem_post::RedeemPostHandler& gifts_members_redeem_post() noexcept { return m_gifts_members_redeem_post; }
 [[nodiscard]] blog::handlers::member_metafields_members::metafields_browse::MemberMetafieldsMembersMetafieldsBrowseHandler& member_metafields_members_metafields_browse() noexcept { return m_member_metafields_members_metafields_browse; }
 [[nodiscard]] blog::handlers::previews::preview_get::PreviewGetHandler& previews_preview_get() noexcept { return m_previews_preview_get; }
 [[nodiscard]] blog::handlers::email_previews::get_detail::GetDetailHandler& email_previews_get_detail() noexcept { return m_email_previews_get_detail; }
 [[nodiscard]] blog::handlers::email_previews::post_detail::PostDetailHandler& email_previews_post_detail() noexcept { return m_email_previews_post_detail; }
 [[nodiscard]] blog::handlers::emails::browse::BrowseHandler& emails_browse() noexcept { return m_emails_browse; }
 [[nodiscard]] blog::handlers::emails::read_by_id::ReadByIdHandler& emails_read_by_id() noexcept { return m_emails_read_by_id; }
 [[nodiscard]] blog::handlers::emails::emails_id_status_get::EmailsIdStatusGetHandler& emails_emails_id_status_get() noexcept { return m_emails_emails_id_status_get; }
 [[nodiscard]] blog::handlers::emails::emails_id_retry_put::EmailsIdRetryPutHandler& emails_emails_id_retry_put() noexcept { return m_emails_emails_id_retry_put; }
 [[nodiscard]] blog::handlers::emails::batches_get::BatchesGetHandler& emails_batches_get() noexcept { return m_emails_batches_get; }
 [[nodiscard]] blog::handlers::emails::recipient_failures_get::RecipientFailuresGetHandler& emails_recipient_failures_get() noexcept { return m_emails_recipient_failures_get; }
 [[nodiscard]] blog::handlers::emails::analytics_get::AnalyticsGetHandler& emails_analytics_get() noexcept { return m_emails_analytics_get; }
 [[nodiscard]] blog::handlers::emails::analytics_put::AnalyticsPutHandler& emails_analytics_put() noexcept { return m_emails_analytics_put; }
 [[nodiscard]] blog::handlers::emails::analytics_delete::AnalyticsDeleteHandler& emails_analytics_delete() noexcept { return m_emails_analytics_delete; }
 [[nodiscard]] blog::handlers::snippets::browse::BrowseHandler& snippets_browse() noexcept { return m_snippets_browse; }
 [[nodiscard]] blog::handlers::snippets::read_by_id::ReadByIdHandler& snippets_read_by_id() noexcept { return m_snippets_read_by_id; }
 [[nodiscard]] blog::handlers::snippets::create::CreateHandler& snippets_create() noexcept { return m_snippets_create; }
 [[nodiscard]] blog::handlers::snippets::edit::EditHandler& snippets_edit() noexcept { return m_snippets_edit; }
 [[nodiscard]] blog::handlers::snippets::destroy::DestroyHandler& snippets_destroy() noexcept { return m_snippets_destroy; }
 [[nodiscard]] blog::handlers::custom_theme_settings::browse::BrowseHandler& custom_theme_settings_browse() noexcept { return m_custom_theme_settings_browse; }
 [[nodiscard]] blog::handlers::custom_theme_settings::bulk_edit::BulkEditHandler& custom_theme_settings_bulk_edit() noexcept { return m_custom_theme_settings_bulk_edit; }
 [[nodiscard]] blog::handlers::newsletters::read_by_id::ReadByIdHandler& newsletters_read_by_id() noexcept { return m_newsletters_read_by_id; }
 [[nodiscard]] blog::handlers::newsletters::create::CreateHandler& newsletters_create() noexcept { return m_newsletters_create; }
 [[nodiscard]] blog::handlers::newsletters::newsletters_verifications_put::NewslettersVerificationsPutHandler& newsletters_newsletters_verifications_put() noexcept { return m_newsletters_newsletters_verifications_put; }
 [[nodiscard]] blog::handlers::newsletters::edit::EditHandler& newsletters_edit() noexcept { return m_newsletters_edit; }
 [[nodiscard]] blog::handlers::links::browse::BrowseHandler& links_browse() noexcept { return m_links_browse; }
 [[nodiscard]] blog::handlers::links::bulk_edit::BulkEditHandler& links_bulk_edit() noexcept { return m_links_bulk_edit; }
 [[nodiscard]] blog::handlers::recommendations::read_by_id::ReadByIdHandler& recommendations_read_by_id() noexcept { return m_recommendations_read_by_id; }
 [[nodiscard]] blog::handlers::recommendations::create::CreateHandler& recommendations_create() noexcept { return m_recommendations_create; }
 [[nodiscard]] blog::handlers::recommendations::recommendations_check_post::RecommendationsCheckPostHandler& recommendations_recommendations_check_post() noexcept { return m_recommendations_recommendations_check_post; }
 [[nodiscard]] blog::handlers::recommendations::edit::EditHandler& recommendations_edit() noexcept { return m_recommendations_edit; }
 [[nodiscard]] blog::handlers::recommendations::destroy::DestroyHandler& recommendations_destroy() noexcept { return m_recommendations_destroy; }
 [[nodiscard]] blog::handlers::incoming_recommendations::browse::BrowseHandler& incoming_recommendations_browse() noexcept { return m_incoming_recommendations_browse; }
 [[nodiscard]] blog::handlers::feedback::read_by_id::ReadByIdHandler& feedback_read_by_id() noexcept { return m_feedback_read_by_id; }
 [[nodiscard]] blog::handlers::feedback::add::AddHandler& feedback_add() noexcept { return m_feedback_add; }
 [[nodiscard]] blog::handlers::search_index::search_index_pages_get::SearchIndexPagesGetHandler& search_index_search_index_pages_get() noexcept { return m_search_index_search_index_pages_get; }
 [[nodiscard]] blog::handlers::search_index::search_index_users_get::SearchIndexUsersGetHandler& search_index_search_index_users_get() noexcept { return m_search_index_search_index_users_get; }


private:
    std::unique_ptr<blog::BlogContext> m_owned_context{};
    blog::BlogContext& m_ctx;
    blog::handlers::posts::browse::PostsBrowseHandler m_posts_browse{m_ctx};
    blog::handlers::posts::read_by_id::PostsReadByIdHandler m_posts_read_by_id{m_ctx};
    blog::handlers::posts::read_by_slug::PostsReadBySlugHandler m_posts_read_by_slug{m_ctx};
    blog::handlers::posts::export_op::PostsExportHandler m_posts_export{m_ctx};
    blog::handlers::posts::create::PostsCreateHandler m_posts_create{m_ctx};
    blog::handlers::posts::bulk_destroy::PostsBulkDestroyHandler m_posts_bulk_destroy{m_ctx};
    blog::handlers::posts::bulk_edit::PostsBulkEditHandler m_posts_bulk_edit{m_ctx};
    blog::handlers::posts::upload::PostsUploadHandler m_posts_upload{m_ctx};
    blog::handlers::posts::edit::PostsEditHandler m_posts_edit{m_ctx};
    blog::handlers::posts::destroy::PostsDestroyHandler m_posts_destroy{m_ctx};
    blog::handlers::posts::copy::PostsCopyHandler m_posts_copy{m_ctx};
    blog::handlers::posts::gift_links_browse::PostsGiftLinksBrowseHandler m_posts_gift_links_browse{m_ctx};
    blog::handlers::posts::gift_links_edit::PostsGiftLinksEditHandler m_posts_gift_links_edit{m_ctx};
    blog::handlers::posts::gift_links_create::PostsGiftLinksCreateHandler m_posts_gift_links_create{m_ctx};
 blog::handlers::pages::browse::BrowseHandler m_pages_browse{m_ctx};
 blog::handlers::pages::read_by_id::ReadByIdHandler m_pages_read_by_id{m_ctx};
 blog::handlers::pages::read_by_slug::ReadBySlugHandler m_pages_read_by_slug{m_ctx};
 blog::handlers::authors::browse::BrowseHandler m_authors_browse{m_ctx};
 blog::handlers::authors::read_by_id::ReadByIdHandler m_authors_read_by_id{m_ctx};
 blog::handlers::authors::read_by_slug::ReadBySlugHandler m_authors_read_by_slug{m_ctx};
 blog::handlers::tags::browse::BrowseHandler m_tags_browse{m_ctx};
 blog::handlers::tags::read_by_id::ReadByIdHandler m_tags_read_by_id{m_ctx};
 blog::handlers::tags::read_by_slug::ReadBySlugHandler m_tags_read_by_slug{m_ctx};
 blog::handlers::settings::browse::BrowseHandler m_settings_browse{m_ctx};
 blog::handlers::newsletters::browse::BrowseHandler m_newsletters_browse{m_ctx};
 blog::handlers::tiers::browse::BrowseHandler m_tiers_browse{m_ctx};
 blog::handlers::offers::read_by_id::ReadByIdHandler m_offers_read_by_id{m_ctx};
 blog::handlers::recommendations::browse::BrowseHandler m_recommendations_browse{m_ctx};
 blog::handlers::search_index::search_index_posts_get::SearchIndexPostsGetHandler m_search_index_search_index_posts_get{m_ctx};
 blog::handlers::search_index::search_index_authors_get::SearchIndexAuthorsGetHandler m_search_index_search_index_authors_get{m_ctx};
 blog::handlers::search_index::search_index_tags_get::SearchIndexTagsGetHandler m_search_index_search_index_tags_get{m_ctx};
 blog::handlers::site::browse::BrowseHandler m_site_browse{m_ctx};
 blog::handlers::config::browse::BrowseHandler m_config_browse{m_ctx};
 blog::handlers::config::config_featurebase_get::ConfigFeaturebaseGetHandler m_config_config_featurebase_get{m_ctx};
 blog::handlers::mentions::browse::BrowseHandler m_mentions_browse{m_ctx};
 blog::handlers::mentions::receive::ReceiveHandler m_mentions_receive{m_ctx};
 blog::handlers::comments::browse::BrowseHandler m_comments_browse{m_ctx};
 blog::handlers::comments::read_by_id::ReadByIdHandler m_comments_read_by_id{m_ctx};
 blog::handlers::comments::comment_replies::CommentRepliesHandler m_comments_comment_replies{m_ctx};
 blog::handlers::comments::comment_reports::CommentReportsHandler m_comments_comment_reports{m_ctx};
 blog::handlers::comments::comment_likes::CommentLikesHandler m_comments_comment_likes{m_ctx};
 blog::handlers::comments::comment_dislikes::CommentDislikesHandler m_comments_comment_dislikes{m_ctx};
 blog::handlers::comments::get_detail::GetDetailHandler m_comments_get_detail{m_ctx};
 blog::handlers::comments::create::CreateHandler m_comments_create{m_ctx};
 blog::handlers::comments::edit::EditHandler m_comments_edit{m_ctx};
 blog::handlers::comments::counts::CountsHandler m_comments_counts{m_ctx};
 blog::handlers::comments::destroy::DestroyHandler m_comments_destroy{m_ctx};
 blog::handlers::comments::like::LikeHandler m_comments_like{m_ctx};
 blog::handlers::comments::unlike::UnlikeHandler m_comments_unlike{m_ctx};
 blog::handlers::comments::dislike::DislikeHandler m_comments_dislike{m_ctx};
 blog::handlers::comments::undislike::UndislikeHandler m_comments_undislike{m_ctx};
 blog::handlers::comments::report::ReportHandler m_comments_report{m_ctx};
 blog::handlers::pages::bulk_destroy::BulkDestroyHandler m_pages_bulk_destroy{m_ctx};
 blog::handlers::pages::bulk_edit::BulkEditHandler m_pages_bulk_edit{m_ctx};
 blog::handlers::pages::create::CreateHandler m_pages_create{m_ctx};
 blog::handlers::pages::edit::EditHandler m_pages_edit{m_ctx};
 blog::handlers::pages::destroy::DestroyHandler m_pages_destroy{m_ctx};
 blog::handlers::pages::pages_id_copy_post::PagesIdCopyPostHandler m_pages_pages_id_copy_post{m_ctx};
 blog::handlers::pages::gift_links_get::GiftLinksGetHandler m_pages_gift_links_get{m_ctx};
 blog::handlers::pages::gift_links_put::GiftLinksPutHandler m_pages_gift_links_put{m_ctx};
 blog::handlers::pages::gift_links_post::GiftLinksPostHandler m_pages_gift_links_post{m_ctx};
 blog::handlers::gift_links::gift_links_remove_all_put::GiftLinksRemoveAllPutHandler m_gift_links_gift_links_remove_all_put{m_ctx};
 blog::handlers::integrations::browse::BrowseHandler m_integrations_browse{m_ctx};
 blog::handlers::integrations::read_by_id::ReadByIdHandler m_integrations_read_by_id{m_ctx};
 blog::handlers::integrations::create::CreateHandler m_integrations_create{m_ctx};
 blog::handlers::integrations::integrations_id_api_key_keyid_refresh_post::IntegrationsIdApiKeyKeyidRefreshPostHandler m_integrations_integrations_id_api_key_keyid_refresh_post{m_ctx};
 blog::handlers::integrations::edit::EditHandler m_integrations_edit{m_ctx};
 blog::handlers::integrations::destroy::DestroyHandler m_integrations_destroy{m_ctx};
 blog::handlers::schedules::put_detail::PutDetailHandler m_schedules_put_detail{m_ctx};
 blog::handlers::gifts::gifts_flush_reminders_put::GiftsFlushRemindersPutHandler m_gifts_gifts_flush_reminders_put{m_ctx};
 blog::handlers::gifts::gifts_flush_deliveries_put::GiftsFlushDeliveriesPutHandler m_gifts_gifts_flush_deliveries_put{m_ctx};
 blog::handlers::settings::settings_routes_yaml_get::SettingsRoutesYamlGetHandler m_settings_settings_routes_yaml_get{m_ctx};
 blog::handlers::settings::settings_routes_yaml_post::SettingsRoutesYamlPostHandler m_settings_settings_routes_yaml_post{m_ctx};
 blog::handlers::settings::bulk_edit::BulkEditHandler m_settings_bulk_edit{m_ctx};
 blog::handlers::settings::settings_access_code_regenerate_post::SettingsAccessCodeRegeneratePostHandler m_settings_settings_access_code_regenerate_post{m_ctx};
 blog::handlers::settings::settings_verifications_put::SettingsVerificationsPutHandler m_settings_settings_verifications_put{m_ctx};
 blog::handlers::settings::settings_stripe_connect_delete::SettingsStripeConnectDeleteHandler m_settings_settings_stripe_connect_delete{m_ctx};
 blog::handlers::users::browse::BrowseHandler m_users_browse{m_ctx};
 blog::handlers::users::read_by_id::ReadByIdHandler m_users_read_by_id{m_ctx};
 blog::handlers::users::read_by_slug::ReadBySlugHandler m_users_read_by_slug{m_ctx};
 blog::handlers::users::read_by_email::ReadByEmailHandler m_users_read_by_email{m_ctx};
 blog::handlers::users::users_id_token_get::UsersIdTokenGetHandler m_users_users_id_token_get{m_ctx};
 blog::handlers::users::users_password_put::UsersPasswordPutHandler m_users_users_password_put{m_ctx};
 blog::handlers::users::users_owner_put::UsersOwnerPutHandler m_users_users_owner_put{m_ctx};
 blog::handlers::users::edit::EditHandler m_users_edit{m_ctx};
 blog::handlers::users::users_id_token_put::UsersIdTokenPutHandler m_users_users_id_token_put{m_ctx};
 blog::handlers::users::destroy::DestroyHandler m_users_destroy{m_ctx};
 blog::handlers::tags::create::CreateHandler m_tags_create{m_ctx};
 blog::handlers::tags::edit::EditHandler m_tags_edit{m_ctx};
 blog::handlers::tags::destroy::DestroyHandler m_tags_destroy{m_ctx};
 blog::handlers::tiers::create::CreateHandler m_tiers_create{m_ctx};
 blog::handlers::tiers::read_by_id::ReadByIdHandler m_tiers_read_by_id{m_ctx};
 blog::handlers::tiers::edit::EditHandler m_tiers_edit{m_ctx};
 blog::handlers::stripe::stripe_checkout_config_get::StripeCheckoutConfigGetHandler m_stripe_stripe_checkout_config_get{m_ctx, m_stripe};
 blog::handlers::stripe::stripe_checkout_config_put::StripeCheckoutConfigPutHandler m_stripe_stripe_checkout_config_put{m_ctx, m_stripe};
 blog::handlers::stripe::stripe_checkout_preview_post::StripeCheckoutPreviewPostHandler m_stripe_stripe_checkout_preview_post{m_ctx, m_stripe};
 blog::handlers::stripe::stripe_checkout_branding_get::StripeCheckoutBrandingGetHandler m_stripe_stripe_checkout_branding_get{m_ctx, m_stripe};
 blog::handlers::members::browse::BrowseHandler m_members_browse{m_ctx};
 blog::handlers::members::create::CreateHandler m_members_create{m_ctx};
 blog::handlers::members::bulk_destroy::BulkDestroyHandler m_members_bulk_destroy{m_ctx};
 blog::handlers::members::bulk_edit::BulkEditHandler m_members_bulk_edit{m_ctx};
 blog::handlers::offers::browse::BrowseHandler m_offers_browse{m_ctx};
 blog::handlers::offers::create::CreateHandler m_offers_create{m_ctx};
 blog::handlers::offers::edit::EditHandler m_offers_edit{m_ctx};
 blog::handlers::members::members_stats_count_get::MembersStatsCountGetHandler m_members_members_stats_count_get{m_ctx};
 blog::handlers::members::members_stats_mrr_get::MembersStatsMrrGetHandler m_members_members_stats_mrr_get{m_ctx};
 blog::handlers::members::members_events_get::MembersEventsGetHandler m_members_members_events_get{m_ctx};
 blog::handlers::members::members_upload_get::MembersUploadGetHandler m_members_members_upload_get{m_ctx};
 blog::handlers::members::members_upload_post::MembersUploadPostHandler m_members_members_upload_post{m_ctx};
 blog::handlers::members::members_stripe_connect_get::MembersStripeConnectGetHandler m_members_members_stripe_connect_get{m_ctx};
 blog::handlers::members::members_metafields_all::MembersMetafieldsAllHandler m_members_members_metafields_all{m_ctx, m_members_ops};
 blog::handlers::apps::all_collection::AllCollectionHandler m_apps_all_collection{m_ctx};
 blog::handlers::apps::app_installations_browse::AppInstallationsBrowseHandler m_apps_app_installations_browse{m_ctx};
 blog::handlers::apps::app_installations_add::AppInstallationsAddHandler m_apps_app_installations_add{m_ctx};
 blog::handlers::apps::app_installations_read::AppInstallationsReadHandler m_apps_app_installations_read{m_ctx};
 blog::handlers::apps::app_installations_edit::AppInstallationsEditHandler m_apps_app_installations_edit{m_ctx};
 blog::handlers::apps::app_installations_destroy::AppInstallationsDestroyHandler m_apps_app_installations_destroy{m_ctx};
 blog::handlers::apps::app_installation_previews_add::AppInstallationPreviewsAddHandler m_apps_app_installation_previews_add{m_ctx};
 blog::handlers::members::read_by_id::ReadByIdHandler m_members_read_by_id{m_ctx};
 blog::handlers::members::edit::EditHandler m_members_edit{m_ctx};
 blog::handlers::members::destroy::DestroyHandler m_members_destroy{m_ctx};
 blog::handlers::members::members_id_sessions_delete::MembersIdSessionsDeleteHandler m_members_members_id_sessions_delete{m_ctx};
 blog::handlers::members::members_id_suppression_delete::MembersIdSuppressionDeleteHandler m_members_members_id_suppression_delete{m_ctx};
 blog::handlers::members::subscriptions_post::SubscriptionsPostHandler m_members_subscriptions_post{m_ctx, m_members_ops};
 blog::handlers::members::put_detail::PutDetailHandler m_members_put_detail{m_ctx, m_members_ops};
 blog::handlers::members::signin_urls::SigninUrlsHandler m_members_signin_urls{m_ctx, m_members_ops};
 blog::handlers::members::members_metafields_read::MembersMetafieldsReadHandler m_members_metafields_read{m_ctx, m_members_ops};
 blog::handlers::members::members_metafields_values::MembersMetafieldsValuesHandler m_members_metafields_values{m_ctx, m_members_ops};
 blog::handlers::members::members_metafields_create::MembersMetafieldsCreateHandler m_members_metafields_create{m_ctx, m_members_ops};
 blog::handlers::members::members_metafields_reorder::MembersMetafieldsReorderHandler m_members_metafields_reorder{m_ctx, m_members_ops};
 blog::handlers::members::members_metafields_edit::MembersMetafieldsEditHandler m_members_metafields_edit{m_ctx, m_members_ops};
 blog::handlers::members::members_metafields_destroy::MembersMetafieldsDestroyHandler m_members_metafields_destroy{m_ctx, m_members_ops};
 blog::handlers::members::members_account::read::MembersAccountReadHandler m_members_account_read{m_ctx, m_members_ops};
 blog::handlers::members::members_account::update::MembersAccountUpdateHandler m_members_account_update{m_ctx, m_members_ops};
 blog::handlers::members::members_account::destroy_suppression::MembersAccountDestroySuppressionHandler m_members_account_destroy_suppression{m_ctx, m_members_ops};
 blog::handlers::members::members_id_commenting_disable_post::MembersIdCommentingDisablePostHandler m_members_members_id_commenting_disable_post{m_ctx, m_members_ops};
 blog::handlers::members::members_id_commenting_enable_post::MembersIdCommentingEnablePostHandler m_members_members_id_commenting_enable_post{m_ctx, m_members_ops};
 blog::handlers::stats::stats_member_count_get::StatsMemberCountGetHandler m_stats_stats_member_count_get{m_ctx};
 blog::handlers::stats::stats_mrr_get::StatsMrrGetHandler m_stats_stats_mrr_get{m_ctx};
 blog::handlers::stats::subscriptions_get::SubscriptionsGetHandler m_stats_subscriptions_get{m_ctx};
 blog::handlers::stats::stats_referrers_get::StatsReferrersGetHandler m_stats_stats_referrers_get{m_ctx};
 blog::handlers::stats::stats_posts_id_stats_get::StatsPostsIdStatsGetHandler m_stats_stats_posts_id_stats_get{m_ctx};
 blog::handlers::stats::stats_top_posts_get::StatsTopPostsGetHandler m_stats_stats_top_posts_get{m_ctx};
 blog::handlers::stats::stats_top_posts_views_get::StatsTopPostsViewsGetHandler m_stats_stats_top_posts_views_get{m_ctx};
 blog::handlers::stats::stats_top_content_get::StatsTopContentGetHandler m_stats_stats_top_content_get{m_ctx};
 blog::handlers::stats::stats_newsletter_stats_get::StatsNewsletterStatsGetHandler m_stats_stats_newsletter_stats_get{m_ctx};
 blog::handlers::stats::stats_newsletter_basic_stats_get::StatsNewsletterBasicStatsGetHandler m_stats_stats_newsletter_basic_stats_get{m_ctx};
 blog::handlers::stats::stats_newsletter_click_stats_get::StatsNewsletterClickStatsGetHandler m_stats_stats_newsletter_click_stats_get{m_ctx};
 blog::handlers::stats::stats_subscriber_count_get::StatsSubscriberCountGetHandler m_stats_stats_subscriber_count_get{m_ctx};
 blog::handlers::stats::stats_posts_id_top_referrers_get::StatsPostsIdTopReferrersGetHandler m_stats_stats_posts_id_top_referrers_get{m_ctx};
 blog::handlers::stats::stats_posts_id_growth_get::StatsPostsIdGrowthGetHandler m_stats_stats_posts_id_growth_get{m_ctx};
 blog::handlers::stats::stats_top_sources_growth_get::StatsTopSourcesGrowthGetHandler m_stats_stats_top_sources_growth_get{m_ctx};
 blog::handlers::stats::stats_posts_visitor_counts_post::StatsPostsVisitorCountsPostHandler m_stats_stats_posts_visitor_counts_post{m_ctx};
 blog::handlers::stats::stats_posts_member_counts_post::StatsPostsMemberCountsPostHandler m_stats_stats_posts_member_counts_post{m_ctx};
 blog::handlers::labels::browse::BrowseHandler m_labels_browse{m_ctx};
 blog::handlers::labels::read_by_id::ReadByIdHandler m_labels_read_by_id{m_ctx};
 blog::handlers::labels::read_by_slug::ReadBySlugHandler m_labels_read_by_slug{m_ctx};
 blog::handlers::labels::create::CreateHandler m_labels_create{m_ctx};
 blog::handlers::labels::edit::EditHandler m_labels_edit{m_ctx};
 blog::handlers::labels::destroy::DestroyHandler m_labels_destroy{m_ctx};
 blog::handlers::automations::browse::BrowseHandler m_automations_browse{m_ctx};
 blog::handlers::automations::automations_automation_id_actions_action_id_links_get::AutomationsAutomationIdActionsActionIdLinksGetHandler m_automations_automations_automation_id_actions_action_id_links_get{m_ctx};
 blog::handlers::automations::read_by_id::ReadByIdHandler m_automations_read_by_id{m_ctx};
 blog::handlers::automations::automations_id_performance_stats_get::AutomationsIdPerformanceStatsGetHandler m_automations_automations_id_performance_stats_get{m_ctx};
 blog::handlers::automations::automations_id_runs_get::AutomationsIdRunsGetHandler m_automations_automations_id_runs_get{m_ctx};
 blog::handlers::automations::get_detail::GetDetailHandler m_automations_get_detail{m_ctx};
 blog::handlers::automations::automations_id_email_preview_post::AutomationsIdEmailPreviewPostHandler m_automations_automations_id_email_preview_post{m_ctx, m_automations_ops};
 blog::handlers::automations::automations_id_email_test_post::AutomationsIdEmailTestPostHandler m_automations_automations_id_email_test_post{m_ctx, m_automations_ops};
 blog::handlers::automations::automations_poll_put::AutomationsPollPutHandler m_automations_automations_poll_put{m_ctx};
 blog::handlers::automations::create::CreateHandler m_automations_create{m_ctx};
 blog::handlers::automations::edit::EditHandler m_automations_edit{m_ctx};
 blog::handlers::automated_emails::browse::BrowseHandler m_automated_emails_browse{m_ctx};
 blog::handlers::automated_emails::automated_emails_design_get::AutomatedEmailsDesignGetHandler m_automated_emails_automated_emails_design_get{m_ctx};
 blog::handlers::automated_emails::automated_emails_senders_put::AutomatedEmailsSendersPutHandler m_automated_emails_automated_emails_senders_put{m_ctx};
 blog::handlers::automated_emails::automated_emails_verifications_put::AutomatedEmailsVerificationsPutHandler m_automated_emails_automated_emails_verifications_put{m_ctx};
 blog::handlers::automated_emails::read_by_id::ReadByIdHandler m_automated_emails_read_by_id{m_ctx};
 blog::handlers::automated_emails::create::CreateHandler m_automated_emails_create{m_ctx};
 blog::handlers::automated_emails::automated_emails_design_put::AutomatedEmailsDesignPutHandler m_automated_emails_automated_emails_design_put{m_ctx};
 blog::handlers::automated_emails::edit::EditHandler m_automated_emails_edit{m_ctx};
 blog::handlers::automated_emails::automated_emails_id_preview_post::AutomatedEmailsIdPreviewPostHandler m_automated_emails_automated_emails_id_preview_post{m_ctx};
 blog::handlers::automated_emails::automated_emails_id_test_post::AutomatedEmailsIdTestPostHandler m_automated_emails_automated_emails_id_test_post{m_ctx};
 blog::handlers::roles::browse::BrowseHandler m_roles_browse{m_ctx};
 blog::handlers::slugs::get_detail::GetDetailHandler m_slugs_get_detail{m_ctx};
 blog::handlers::themes::browse::BrowseHandler m_themes_browse{m_ctx};
 blog::handlers::themes::themes_name_download_get::ThemesNameDownloadGetHandler m_themes_themes_name_download_get{m_ctx};
 blog::handlers::themes::themes_active_get::ThemesActiveGetHandler m_themes_themes_active_get{m_ctx};
 blog::handlers::themes::themes_upload_post::ThemesUploadPostHandler m_themes_themes_upload_post{m_ctx};
 blog::handlers::themes::themes_install_post::ThemesInstallPostHandler m_themes_themes_install_post{m_ctx};
 blog::handlers::themes::themes_name_activate_put::ThemesNameActivatePutHandler m_themes_themes_name_activate_put{m_ctx};
 blog::handlers::themes::destroy::DestroyHandler m_themes_destroy{m_ctx};
 blog::handlers::notifications::browse::BrowseHandler m_notifications_browse{m_ctx};
 blog::handlers::notifications::create::CreateHandler m_notifications_create{m_ctx};
 blog::handlers::notifications::destroy::DestroyHandler m_notifications_destroy{m_ctx};
 blog::handlers::db::browse::BrowseHandler m_db_browse{m_ctx};
 blog::handlers::db::create::CreateHandler m_db_create{m_ctx};
 blog::handlers::db::bulk_destroy::BulkDestroyHandler m_db_bulk_destroy{m_ctx};
 blog::handlers::db::db_backup_post::DbBackupPostHandler m_db_db_backup_post{m_ctx};
 blog::handlers::db::db_media_inline_post::DbMediaInlinePostHandler m_db_db_media_inline_post{m_ctx};
 blog::handlers::exports::exports_download_get::ExportsDownloadGetHandler m_exports_exports_download_get{m_ctx};
 blog::handlers::exports::create::CreateHandler m_exports_create{m_ctx};
 blog::handlers::slack::slack_test_post::SlackTestPostHandler m_slack_slack_test_post{m_ctx};
 blog::handlers::tinybird::tinybird_token_get::TinybirdTokenGetHandler m_tinybird_tinybird_token_get{m_ctx};
 blog::handlers::featurebase::featurebase_token_get::FeaturebaseTokenGetHandler m_featurebase_featurebase_token_get{m_ctx};
 blog::handlers::session::create::CreateHandler m_session_create{m_ctx};
 blog::handlers::session::bulk_destroy::BulkDestroyHandler m_session_bulk_destroy{m_ctx};
 blog::handlers::session::session_verify_post::SessionVerifyPostHandler m_session_session_verify_post{m_ctx};
 blog::handlers::session::session_verify_put::SessionVerifyPutHandler m_session_session_verify_put{m_ctx};
 blog::handlers::identities::browse::BrowseHandler m_identities_browse{m_ctx};
 blog::handlers::authentication::authentication_password_reset_post::AuthenticationPasswordResetPostHandler m_authentication_authentication_password_reset_post{m_ctx};
 blog::handlers::authentication::authentication_password_reset_put::AuthenticationPasswordResetPutHandler m_authentication_authentication_password_reset_put{m_ctx};
 blog::handlers::authentication::authentication_invitation_post::AuthenticationInvitationPostHandler m_authentication_authentication_invitation_post{m_ctx};
 blog::handlers::authentication::authentication_invitation_get::AuthenticationInvitationGetHandler m_authentication_authentication_invitation_get{m_ctx};
 blog::handlers::authentication::authentication_setup_post::AuthenticationSetupPostHandler m_authentication_authentication_setup_post{m_ctx};
 blog::handlers::authentication::authentication_setup_put::AuthenticationSetupPutHandler m_authentication_authentication_setup_put{m_ctx};
 blog::handlers::authentication::authentication_setup_get::AuthenticationSetupGetHandler m_authentication_authentication_setup_get{m_ctx};
 blog::handlers::authentication::authentication_reset_post::AuthenticationResetPostHandler m_authentication_authentication_reset_post{m_ctx};
 blog::handlers::images::images_upload_post::ImagesUploadPostHandler m_images_images_upload_post{m_ctx};
 blog::handlers::media::media_upload_post::MediaUploadPostHandler m_media_media_upload_post{m_ctx};
 blog::handlers::media::media_thumbnail_upload_put::MediaThumbnailUploadPutHandler m_media_media_thumbnail_upload_put{m_ctx};
 blog::handlers::files::files_upload_post::FilesUploadPostHandler m_files_files_upload_post{m_ctx};
 blog::handlers::invites::browse::BrowseHandler m_invites_browse{m_ctx};
 blog::handlers::invites::read_by_id::ReadByIdHandler m_invites_read_by_id{m_ctx};
 blog::handlers::invites::create::CreateHandler m_invites_create{m_ctx};
 blog::handlers::invites::destroy::DestroyHandler m_invites_destroy{m_ctx};
 blog::handlers::redirects::redirects_download_get::RedirectsDownloadGetHandler m_redirects_redirects_download_get{m_ctx};
 blog::handlers::redirects::redirects_upload_post::RedirectsUploadPostHandler m_redirects_redirects_upload_post{m_ctx};
 blog::handlers::upgrades::browse::BrowseHandler m_upgrades_browse{m_ctx};
 blog::handlers::upgrades::create::CreateHandler m_upgrades_create{m_ctx};
 blog::handlers::upgrades::read_by_id::ReadByIdHandler m_upgrades_read_by_id{m_ctx};
 blog::handlers::webhooks::create::CreateHandler m_webhooks_create{m_ctx};
 blog::handlers::webhooks::edit::EditHandler m_webhooks_edit{m_ctx};
 blog::handlers::webhooks::destroy::DestroyHandler m_webhooks_destroy{m_ctx};
 blog::handlers::oembed::browse::BrowseHandler m_oembed_browse{m_ctx};
 blog::handlers::actions::browse::BrowseHandler m_actions_browse{m_ctx};
 blog::handlers::announcements::announcement_get::AnnouncementGetHandler m_announcements_announcement_get{m_ctx};
 blog::handlers::app_sessions::sessions_create::AppSessionsCreateHandler m_app_sessions_sessions_create{m_ctx};
 blog::handlers::gifts_members::redeem_get::RedeemGetHandler m_gifts_members_redeem_get{m_ctx};
 blog::handlers::gifts_members::redeem_post::RedeemPostHandler m_gifts_members_redeem_post{m_ctx};
 blog::handlers::member_metafields_members::metafields_browse::MemberMetafieldsMembersMetafieldsBrowseHandler m_member_metafields_members_metafields_browse{m_ctx};
 blog::handlers::previews::preview_get::PreviewGetHandler m_previews_preview_get{m_ctx};
 blog::handlers::email_previews::get_detail::GetDetailHandler m_email_previews_get_detail{m_ctx};
 blog::handlers::email_previews::post_detail::PostDetailHandler m_email_previews_post_detail{m_ctx};
 blog::handlers::emails::browse::BrowseHandler m_emails_browse{m_ctx};
 blog::handlers::emails::read_by_id::ReadByIdHandler m_emails_read_by_id{m_ctx};
 blog::handlers::emails::emails_id_status_get::EmailsIdStatusGetHandler m_emails_emails_id_status_get{m_ctx};
 blog::handlers::emails::emails_id_retry_put::EmailsIdRetryPutHandler m_emails_emails_id_retry_put{m_ctx};
 blog::handlers::emails::batches_get::BatchesGetHandler m_emails_batches_get{m_ctx, m_emails_ops};
 blog::handlers::emails::recipient_failures_get::RecipientFailuresGetHandler m_emails_recipient_failures_get{m_ctx, m_emails_ops};
 blog::handlers::emails::analytics_get::AnalyticsGetHandler m_emails_analytics_get{m_ctx, m_emails_ops};
 blog::handlers::emails::analytics_put::AnalyticsPutHandler m_emails_analytics_put{m_ctx, m_emails_ops};
 blog::handlers::emails::analytics_delete::AnalyticsDeleteHandler m_emails_analytics_delete{m_ctx, m_emails_ops};
 blog::handlers::snippets::browse::BrowseHandler m_snippets_browse{m_ctx};
 blog::handlers::snippets::read_by_id::ReadByIdHandler m_snippets_read_by_id{m_ctx};
 blog::handlers::snippets::create::CreateHandler m_snippets_create{m_ctx};
 blog::handlers::snippets::edit::EditHandler m_snippets_edit{m_ctx};
 blog::handlers::snippets::destroy::DestroyHandler m_snippets_destroy{m_ctx};
 blog::handlers::custom_theme_settings::browse::BrowseHandler m_custom_theme_settings_browse{m_ctx};
 blog::handlers::custom_theme_settings::bulk_edit::BulkEditHandler m_custom_theme_settings_bulk_edit{m_ctx};
 blog::handlers::newsletters::read_by_id::ReadByIdHandler m_newsletters_read_by_id{m_ctx};
 blog::handlers::newsletters::create::CreateHandler m_newsletters_create{m_ctx};
 blog::handlers::newsletters::newsletters_verifications_put::NewslettersVerificationsPutHandler m_newsletters_newsletters_verifications_put{m_ctx};
 blog::handlers::newsletters::edit::EditHandler m_newsletters_edit{m_ctx};
 blog::handlers::links::browse::BrowseHandler m_links_browse{m_ctx};
 blog::handlers::links::bulk_edit::BulkEditHandler m_links_bulk_edit{m_ctx};
 blog::handlers::recommendations::read_by_id::ReadByIdHandler m_recommendations_read_by_id{m_ctx};
 blog::handlers::recommendations::create::CreateHandler m_recommendations_create{m_ctx};
 blog::handlers::recommendations::recommendations_check_post::RecommendationsCheckPostHandler m_recommendations_recommendations_check_post{m_ctx};
 blog::handlers::recommendations::edit::EditHandler m_recommendations_edit{m_ctx};
 blog::handlers::recommendations::destroy::DestroyHandler m_recommendations_destroy{m_ctx};
 blog::handlers::incoming_recommendations::browse::BrowseHandler m_incoming_recommendations_browse{m_ctx};
 blog::handlers::feedback::read_by_id::ReadByIdHandler m_feedback_read_by_id{m_ctx};
 blog::handlers::feedback::add::AddHandler m_feedback_add{m_ctx};
 blog::handlers::search_index::search_index_pages_get::SearchIndexPagesGetHandler m_search_index_search_index_pages_get{m_ctx};
 blog::handlers::search_index::search_index_users_get::SearchIndexUsersGetHandler m_search_index_search_index_users_get{m_ctx};
    core::cache::Cache m_cache{make_cache()};
    blog::api::members::MembersOperations m_members_ops{m_cache};
    blog::api::emails::EmailsOperations m_emails_ops{m_cache};
    blog::api::automations::AutomationsOperations m_automations_ops{m_cache};
    StripeHandler m_stripe{};

private:
[[nodiscard]] core::cache::Cache make_cache()
{
    return core::cache::CacheBuilder{}
        .use_backend(std::make_unique<core::cache::LocalCacheBackend>(4096, core::cache::EvictionPolicy::LRU))
        .build();
}
};

} // namespace blog::api

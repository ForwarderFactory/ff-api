#pragma once

#include <database.hpp>
#include <limhamn/http/http_server.hpp>

namespace ff {
    netkit::http::server::response handle_try_upload_post_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_try_upload_post_comment_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_try_setup_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_try_register_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_try_login_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_try_upload_forwarder_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_try_upload_file_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_forwarder_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_file_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_get_forwarders_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_get_files_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_set_approval_for_uploads_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_update_profile_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_get_profile_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_get_announcements_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_announcement(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_edit_announcement_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_create_announcement_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_rate_forwarder_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_rate_file_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_comment_forwarder_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_comment_file_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_comment_forwarder_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_comment_file_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_stay_logged_in(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_try_logout_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_update_user_settings(const netkit::http::server::request& request, database& db);

    netkit::http::server::response handle_api_create_post_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_post_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_edit_post_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_close_post_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_get_posts_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_comment_post_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_comment_post_endpoint(const netkit::http::server::request& request, database& db);

    netkit::http::server::response handle_api_create_topic_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_delete_topic_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_get_topics_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_edit_topic_endpoint(const netkit::http::server::request& request, database& db);
    netkit::http::server::response handle_api_close_topic_endpoint(const netkit::http::server::request& request, database& db);

    netkit::http::server::response handle_root_endpoint(const netkit::http::server::request& request, database& db);
} // namespace ff

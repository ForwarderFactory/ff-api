#pragma once

#include <vector>
#include <string>

namespace ff {
    struct Settings {
#ifndef FF_DEBUG
        std::string access_file{"/var/log/ff/access.log"};
        std::string warning_file{"/var/log/ff/warning.log"};
        std::string error_file{"/var/log/ff/error.log"};
        std::string notice_file{"/var/log/ff/notice.log"};
        bool output_to_std{false};
        bool halt_on_error{false};
        std::string sqlite_database_file{"/var/db/ff/ff.db"};
        std::string session_directory{"/var/lib/ff/sessions"};
        std::string data_directory{"/var/lib/ff/data"};
        std::string temp_directory{"/var/tmp/ff"};
        bool public_registration{true};
        std::vector<std::pair<std::string, std::string>> custom_paths{};
        int64_t max_request_size{250 * 1024 * 1024}; // 250mb
        std::string site_url{"https://api.forwarderfactory.com"};
        bool enable_email_verification{true};
#else
        std::string access_file{"./access.log"};
        std::string warning_file{"./warning.log"};
        std::string error_file{"./error.log"};
        std::string notice_file{"./notice.log"};
        std::string sqlite_database_file{"./ff-debug.db"};
        bool output_to_std{true};
        bool halt_on_error{false};
        std::string session_directory{"./sessions"};
        std::string data_directory{"./data"};
        std::string temp_directory{"./tmp"};
        bool public_registration{true};
        std::vector<std::pair<std::string, std::string>> custom_paths{};
        int64_t max_request_size{1024 * 1024 * 1024};
        std::string site_url{"http://localhost:8080"};
        bool enable_email_verification{false};
#endif
        int port{8080};
        bool log_access_to_file{true};
        bool log_warning_to_file{true};
        bool log_error_to_file{true};
        bool log_notice_to_file{true};
        std::size_t password_min_length{8};
        std::size_t password_max_length{64};
        std::size_t username_min_length{3};
        std::size_t username_max_length{32};
        std::string allowed_characters{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
        bool allow_all_characters{false};
        std::string session_cookie_name{"ff_session"};
        int default_user_type{0};
        bool preview_files{true};
        std::string email_username{};
        std::string email_password{};
        std::string email_from{};
        std::string smtp_server{};
        int smtp_port{465};
        std::string psql_username{"postgres"};
        std::string psql_password{"postgrespasswordhere"};
        std::string psql_database{"ff"};
        std::string psql_host{"localhost"};
        int psql_port{5432};
        bool enabled_database{false};
        bool trust_x_forwarded_for{false};
        int rate_limit{100};
        std::vector<std::string> blacklisted_ips{};
        std::vector<std::string> whitelisted_ips{"127.0.0.1"};
        int64_t max_file_size_hash{1024 * 1024 * 1024};
        bool cache_static{false};
        bool cache_exists{false};
        bool convert_images_to_webp{true};
        bool convert_videos_to_webm{false};
        bool topics_require_admin{false};
    };

    inline Settings settings{};
} // namespace ff

#ifndef FF_VERSION
#define FF_VERSION "Undefined"
#endif

#include <algorithm>
#include <sstream>
#include <filesystem>
#include <ff.hpp>
#include <endpoint_handlers.hpp>
#include <scrypto.hpp>
#include <nlohmann/json.hpp>
#include <limhamn/http/http_utils.hpp>
#include <static_exists.hpp>

void ff::print_help(const bool stream) {
    std::stringstream ss;

    ss << "ff-api [options]" << "\n";
    ss << "  -p, --port               Specify the port number to run ff-api on" << "\n";
    ss << "  -c, --config-file        Specify the configuration file to use" << "\n";
    ss << "  -gc, --generate-config   Generate a default configuration file" << "\n";
    ss << "  -he, --halt-on-error     Halt the server on error" << "\n";
    ss << "  -nhe, --no-halt-on-error Do not halt the server on error" << "\n";
    ss << "  -h, --help               Display help information" << "\n";
    ss << "  -v, --version            Display the version number" << "\n";

    stream ? std::cout << ss.str() : std::cerr << ss.str();
}

void ff::print_version(const bool stream) {
    std::stringstream ss;

    ss << "Version: " << FF_VERSION << "\n";

    stream ? std::cout << ss.str() : std::cerr << ss.str();
}

std::string ff::open_file(const std::string& file_path) {
    std::ifstream file{file_path};
    std::string content{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    file.close();
    return content;
}

void ff::prepare_wd() {
    const auto log_error = [](const std::string& error_msg) {
        ff::logger.write_to_log(limhamn::logger::type::error, error_msg);
        std::exit(EXIT_FAILURE);
    };
    const auto create_directory = [](const std::string& path) -> bool {
        try {
            if (path == "." || path == "..") return true;
            if (std::filesystem::is_directory(path)) return true;
            if (!std::filesystem::create_directories(path)) {
                return false;
            }

            return true;
        } catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    };
    const auto check_if_exists = [](const std::string& path) -> bool {
        return std::filesystem::exists(path);
    };
    const auto remove_all_in_directory = [&check_if_exists](const std::string& path) -> void {
        if (!check_if_exists(path)) {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
#if FF_DEBUG
            ff::logger.write_to_log(limhamn::logger::type::notice, "Removing: " + entry.path().string() + "\n");
#endif
            std::filesystem::remove_all(entry.path());
        }
    };

    if (!check_if_exists(ff::settings.session_directory)) {
        ff::logger.write_to_log(limhamn::logger::type::notice, "The session directory does not exist. Creating it.\n");
        if (!create_directory(ff::settings.session_directory)) {
            log_error("Failed to create the session directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The session directory was created.\n");
    }

    if (!check_if_exists(ff::settings.data_directory)) {
        ff::logger.write_to_log(limhamn::logger::type::notice, "The data directory does not exist. Creating it.\n");
        if (!create_directory(ff::settings.data_directory)) {
            log_error("Failed to create the data directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The data directory was created.\n");
    }

    if (!check_if_exists(ff::settings.temp_directory)) {
        ff::logger.write_to_log(limhamn::logger::type::notice, "The temp directory does not exist. Creating it.\n");
        if (!create_directory(ff::settings.temp_directory)) {
            log_error("Failed to create the temp directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The temp directory was created.\n");
    }

    if (!check_if_exists(ff::settings.sqlite_database_file) && !ff::settings.enabled_database) {
        std::filesystem::path database_file_path{ff::settings.sqlite_database_file};
        std::filesystem::path database_file_directory{database_file_path.parent_path()};

        ff::logger.write_to_log(limhamn::logger::type::notice, "The database file directory does not exist. Creating it.\n");
        if (!create_directory(database_file_directory)) {
            log_error("Failed to create the database file directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The database file directory was created.\n");
    }

    remove_all_in_directory(ff::settings.temp_directory);
    remove_all_in_directory(ff::settings.session_directory);
}

void ff::start_server() {
    try {
#ifdef FF_ENABLE_SQLITE
#ifndef FF_ENABLE_POSTGRESQL
        settings.enabled_database = false;
#endif
#endif

#ifdef FF_ENABLE_POSTGRESQL
#ifndef FF_ENABLE_SQLITE
        settings.enabled_database = true;
#endif
#endif

#if FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Using database type: " + std::string(settings.enabled_database ? "PostgreSQL" : "SQLite") + "\n");
#endif

        std::shared_ptr<ff::database> database = std::make_shared<ff::database>(settings.enabled_database);

        if (settings.enabled_database) {
#ifdef FF_ENABLE_POSTGRESQL
            database->get_postgres().open(settings.psql_host,
                settings.psql_username,
                settings.psql_password,
                settings.psql_database,
                settings.psql_port);

#ifdef FF_DEBUG
        	ff::logger.write_to_log(limhamn::logger::type::notice, "PostgreSQL database opened with host: " + settings.psql_host + ", username: " + settings.psql_username + ", password: " + settings.psql_password + ", database: " + settings.psql_database + "\n");
#endif
#endif
        } else {
#ifdef FF_ENABLE_SQLITE
            database->get_sqlite().open(settings.sqlite_database_file);
#endif
        }

        if (!database->good()) {
            ff::fatal = true;
            throw std::runtime_error{"Error opening the database file."};
        }

        setup_database(*database);

        if (!ff::ensure_admin_account_exists(*database)) {
            ff::needs_setup = true;
        	ff::logger.write_to_log(limhamn::logger::type::notice, "Setup is required, please make a request to /api/try_setup\n");
        }

        netkit::http::server::sync_server server(netkit::http::server::server_settings{
            .port = settings.port,
            .enable_session = true,
            .session_directory = settings.session_directory,
            .session_cookie_name = settings.session_cookie_name,
            .associated_session_cookies = {
                "username",
                "user_type",
            },
            .max_request_size = settings.max_request_size,
            .blacklisted_ips = settings.blacklisted_ips,
            .trust_x_forwarded_for = settings.trust_x_forwarded_for,
#ifndef FF_DEBUG
        	.session_is_secure = true,
#endif
            }, [&](const netkit::http::server::request& request) -> netkit::http::server::response {
            ff::logger.write_to_log(limhamn::logger::type::access, "Request received from " + request.ip_address + " to " + request.endpoint + " received, handling it.\n");

            const std::unordered_map<std::string, std::function<netkit::http::server::response(const netkit::http::server::request&, ff::database&)>> handlers{
                {"/api/try_setup", ff::handle_api_try_setup_endpoint},

                {"/api/try_upload_forwarder", ff::handle_api_try_upload_forwarder_endpoint},
                {"/api/try_upload_file", ff::handle_api_try_upload_file_endpoint},
                {"/api/try_login", ff::handle_api_try_login_endpoint},
                {"/api/try_register", ff::handle_api_try_register_endpoint},
                {"/api/get_forwarders", ff::handle_api_get_forwarders_endpoint},
                {"/api/get_files", ff::handle_api_get_files_endpoint},
                {"/api/set_approval_for_uploads", ff::handle_api_set_approval_for_uploads_endpoint},
                {"/api/rate_forwarder", ff::handle_api_rate_forwarder_endpoint},
                {"/api/rate_file", ff::handle_api_rate_file_endpoint},
                {"/api/comment_forwarder", ff::handle_api_comment_forwarder_endpoint},
                {"/api/comment_file", ff::handle_api_comment_file_endpoint},
                {"/api/delete_comment_forwarder", ff::handle_api_delete_comment_forwarder_endpoint},
                {"/api/delete_comment_file", ff::handle_api_delete_comment_file_endpoint},
                {"/api/update_profile", ff::handle_api_update_profile_endpoint},
                {"/api/get_profile", ff::handle_api_get_profile_endpoint},
                {"/api/create_announcement", ff::handle_api_create_announcement_endpoint},
                {"/api/get_announcements", ff::handle_api_get_announcements_endpoint},
                {"/api/delete_announcement", ff::handle_api_delete_announcement},
                {"/api/edit_announcement", ff::handle_api_edit_announcement_endpoint},
                {"/api/stay_logged_in", ff::handle_api_stay_logged_in},
                {"/api/try_logout", ff::handle_api_try_logout_endpoint},
                {"/api/delete_forwarder", ff::handle_api_delete_forwarder_endpoint},
                {"/api/delete_file", ff::handle_api_delete_file_endpoint},

                {"/api/create_post", ff::handle_api_create_post_endpoint},
                {"/api/delete_post", ff::handle_api_delete_post_endpoint},
                {"/api/edit_post", ff::handle_api_edit_post_endpoint},
                {"/api/close_post", ff::handle_api_close_post_endpoint},
                {"/api/get_posts", ff::handle_api_get_posts_endpoint},
                {"/api/comment_post", ff::handle_api_comment_post_endpoint},
                {"/api/delete_comment_post", ff::handle_api_delete_comment_post_endpoint},
                {"/api/create_topic", ff::handle_api_create_topic_endpoint},
                {"/api/delete_topic", ff::handle_api_delete_topic_endpoint},
                {"/api/get_topics", ff::handle_api_get_topics_endpoint},
                {"/api/edit_topic", ff::handle_api_edit_topic_endpoint},
                {"/api/close_topic", ff::handle_api_close_topic_endpoint},
            	{"/api/update_user_settings", ff::handle_api_update_user_settings},
                //{"/api/pin_post_to_topic", ff::handle_api_pin_post_to_topic},
            };
            const std::unordered_map<std::string, std::function<netkit::http::server::response(const netkit::http::server::request&, ff::database&)>> setup_handlers{
                {"/api/try_setup", ff::handle_api_try_setup_endpoint},
            };

            // handle custom paths
            for (const auto& it : ff::settings.custom_paths) {
                if (it.first == request.endpoint) {
                    netkit::http::server::response response{};

                    if (!ff::static_exists.is_file(it.second)) {
                        response.content_type = "text/html";
                        response.http_status = 404;
                        response.body = "<p>404 Not Found</p>";

                        return response;
                    }

                    response.body = ff::cache_manager.open_file(it.second);
                    response.http_status = 200;
                    response.content_type = limhamn::http::utils::get_appropriate_content_type(it.first);

                    return response;
                }
            }

            if (needs_setup && setup_handlers.contains(request.endpoint)) {
                return setup_handlers.at(request.endpoint)(request, *database);
            } else if (needs_setup) {
                return setup_handlers.at("/api/try_setup")(request, *database);
            }

            if (handlers.contains(request.endpoint)) {
                return handlers.at(request.endpoint)(request, *database);
            }

            // check if a file upload exists and if so, download and serve
            std::string file = request.endpoint;

            if (file.back() == '/') {
                file.pop_back();
            }

            if (file.find("/download/") != std::string::npos) {
                std::filesystem::path file_path = file.substr(10); // remove /download/
                file_path = file_path.lexically_normal(); // normalize the path

                if (ff::is_file(*database, file_path.string())) {
                    const auto& h = ff::download_file(*database, ff::UserProperties{
                        .username = request.session.contains("username") ? request.session.at("username") : "",
                        .ip_address = request.ip_address,
                        .user_agent = request.user_agent,
                    }, file_path.string());

#if FF_DEBUG
                    logger.write_to_log(limhamn::logger::type::notice, "File download request for: " + h.path + "\n");
#endif

                    netkit::http::server::response response{};

                    response.body = open_file(h.path);
                    response.http_status = 200;
                    response.content_type = limhamn::http::utils::get_appropriate_content_type(h.name);

                    if (settings.preview_files) {
                        response.headers.push_back({"Content-Disposition", "inline; filename=\"" + h.name + "\""});
                    } else {
                        response.headers.push_back({"Content-Disposition", "attachment; filename=\"" + h.name + "\""});
                    }

                    return response;
                }
            } else if (file.find("/view/") != std::string::npos) {
                std::filesystem::path file_path = file;
                file_path = file_path.lexically_normal(); // normalize the path

                if (file_path.string().find("/view/") == 0) {
                    return handlers.at("/")(request, *database);
                }
            } else if (file.find("/file/") != std::string::npos) {
                return handlers.at("/")(request, *database);
            } else if (file.find("/profile/") != std::string::npos) {
                return handlers.at("/")(request, *database);
            } else if (file.find("/topic") != std::string::npos) {
                return handlers.at("/")(request, *database);
            } else if (file.find("/post/") != std::string::npos) {
                return handlers.at("/")(request, *database);
            }

            // handle activation URLs
            if (file.find("/activate/") != std::string::npos && settings.enable_email_verification) {
                const auto& list = database->query("SELECT * FROM activation_urls WHERE url = ?;", file);
                for (const auto& it : list) {
                    try {
                        const auto json = get_json_from_table(*database, "users", "username", it.at("username"));
                        nlohmann::json user_json;
                        try {
                            user_json = nlohmann::json::parse(json);
                        } catch (const std::exception&) {
                            break;
                        }

                        user_json["activated"] = true;

                        set_json_in_table(*database, "users", "username", it.at("username"), user_json.dump());

                        database->exec("DELETE FROM activation_urls WHERE url = ?;", file);

                        // redirect to /
                        netkit::http::server::response response{};
                        response.http_status = 302;
                        response.headers.push_back({"Location", "/"});
                        return response;
                    } catch (const std::exception&) {
                        netkit::http::server::response resp;
                        resp.content_type = "text/html";
                        resp.http_status = 500;
                        resp.body = "<p>500 Internal Server Error</p>";
                        return resp;
                    }
                }
            }

            netkit::http::server::response response{};

            response.content_type = "text/html";
            response.http_status = 404;
            response.body = "<p>404 Not Found</p>";

            return response;
        });

    	server.run();
    } catch (const std::exception& e) {
        ff::logger.write_to_log(limhamn::logger::type::error, "An error occurred: " + std::string{e.what()} + "\n");

        // a little bit ugly but whatever
        if (std::string(e.what()).find("Address already in use") != std::string::npos) {
            ff::fatal = true;
        }

    	if (std::string(e.what()).find("Error creating the ") != std::string::npos) {
            ff::fatal = true;
        }

        if (ff::fatal) {
            ff::logger.write_to_log(limhamn::logger::type::error, "The last error was too severe to recover, and the server will now halt.\n");
            std::exit(EXIT_FAILURE);
        }

        if (ff::settings.halt_on_error) {
            ff::logger.write_to_log(limhamn::logger::type::error, "Halting the server due to an error.\n");
            std::exit(EXIT_FAILURE);
        }

        start_server();
    }
}

std::string ff::get_temp_path() {
    std::string ret = settings.temp_directory + "/" + scrypto::generate_random_string(32);
    while (std::filesystem::exists(ret)) {
        ret = settings.temp_directory + "/" + scrypto::generate_random_string(32);
    }
    return ret;
}

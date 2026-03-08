#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <memory>

#include <reframework/API.hpp>

namespace vspp
{
    namespace core
    {

        /**
         * @brief Thread-safe Logger class.
         * Outputs logs to a file and the REFramework console.
         */
        class Logger
        {
        public:
            static Logger &get();

            void initialize(const std::filesystem::path &log_path);
            void close();

            // Standard logging methods
            void info(const std::string &message);
            void warn(const std::string &message);
            void error(const std::string &message);
            void debug(const std::string &message);

            // Check if debug mode is enabled (Env var or file)
            bool is_debug_enabled() const { return m_debug_enabled; }

        private:
            Logger() = default;
            ~Logger() = default;
            Logger(const Logger &) = delete;
            Logger &operator=(const Logger &) = delete;

            std::filesystem::path m_log_path;
            std::ofstream m_file_stream;
            mutable std::recursive_mutex m_mutex; // Changed to recursive_mutex to prevent deadlocks
            bool m_initialized{false};
            bool m_debug_enabled{false};

            void check_environment();
            void write_log(const std::string &level, const std::string &message, bool to_api = true);
        };

    } // namespace core
} // namespace vspp

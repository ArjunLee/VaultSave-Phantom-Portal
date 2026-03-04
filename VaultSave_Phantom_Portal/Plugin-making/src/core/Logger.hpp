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
            void info(const std::string &message);
            void warn(const std::string &message);
            void error(const std::string &message);
            void debug(const std::string &message);

        private:
            Logger() = default;
            ~Logger() = default;
            Logger(const Logger &) = delete;
            Logger &operator=(const Logger &) = delete;

            std::filesystem::path m_log_path;
            std::ofstream m_file_stream;
            std::mutex m_mutex;
            bool m_initialized{false};

            void write_to_file(const std::string &level, const std::string &message);
        };

    } // namespace core
} // namespace vspp

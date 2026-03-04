#include "Logger.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace vspp
{
    namespace core
    {

        Logger &Logger::get()
        {
            static Logger instance;
            return instance;
        }

        void Logger::initialize(const std::filesystem::path &log_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_initialized && m_file_stream.is_open())
            {
                m_file_stream.close();
            }

            m_log_path = log_path;

            // Create directories if they don't exist
            if (m_log_path.has_parent_path())
            {
                std::filesystem::create_directories(m_log_path.parent_path());
            }

            m_file_stream.open(m_log_path, std::ios::out | std::ios::app);
            m_initialized = true;
            // check_file_size(); // Avoid recursive lock or complex logic during init

            // Write initial log manually to avoid mutex recursion if info() calls write_to_file
            // But write_to_file locks mutex too. Recursive mutex needed?
            // std::mutex is not recursive.
            // info() calls write_to_file() which locks mutex.
            // initialize() locks mutex.
            // calling info() from initialize() will DEADLOCK.

            // Wait, previous code had:
            // info("Logger initialized.");
            // info() -> write_to_file() -> lock(m_mutex).
            // initialize() -> lock(m_mutex).
            // This IS a deadlock if called from same thread?
            // std::mutex is non-recursive. Yes.

            // I should fix this deadlock too.
        }

        void Logger::close()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_file_stream.is_open())
            {
                m_file_stream.close();
            }
            m_initialized = false;
        }

        void Logger::info(const std::string &message)
        {
            write_to_file("INFO", message);
            if (auto *api = reframework::API::try_get())
            {
                api->log_info("[VaultSave] %s", message.c_str());
            }
        }

        void Logger::warn(const std::string &message)
        {
            write_to_file("WARN", message);
            if (auto *api = reframework::API::try_get())
            {
                api->log_warn("[VaultSave] %s", message.c_str());
            }
        }

        void Logger::error(const std::string &message)
        {
            write_to_file("ERROR", message);
            if (auto *api = reframework::API::try_get())
            {
                api->log_error("[VaultSave] %s", message.c_str());
            }
        }

        void Logger::debug(const std::string &message)
        {
#ifdef _DEBUG
            write_to_file("DEBUG", message);
            if (auto *api = reframework::API::try_get())
            {
                api->log_info("[VaultSave] [DEBUG] %s", message.c_str());
            }
#endif
        }

        void Logger::write_to_file(const std::string &level, const std::string &message)
        {
            // Use recursive_mutex if we want recursive calls, but std::mutex is faster.
            // We will just implement rotation logic inline or in a private helper that expects lock to be held.
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!m_initialized || !m_file_stream.is_open())
                return;

            // Check size (simple check using tellp)
            if (m_file_stream.tellp() > 10 * 1024 * 1024) // 10MB
            {
                m_file_stream.close();
                // Simple rotation: delete old log and start new
                // For better rotation, we could rename to .bak
                try
                {
                    std::filesystem::remove(m_log_path);
                }
                catch (...)
                {
                }

                m_file_stream.open(m_log_path, std::ios::out | std::ios::app);
                if (m_file_stream.is_open())
                {
                    m_file_stream << "[SYSTEM] Log file rotated due to size limit.\n";
                }
            }

            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << "[" << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << "] [" << level << "] " << message << "\n";

            m_file_stream << ss.str();
            m_file_stream.flush();
        }

    } // namespace core
} // namespace vspp

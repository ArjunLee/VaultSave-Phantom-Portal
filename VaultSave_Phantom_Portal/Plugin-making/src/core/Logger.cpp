#include "Logger.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#endif

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
            std::lock_guard<std::recursive_mutex> lock(m_mutex);

            if (m_initialized)
            {
                close();
            }

            m_log_path = log_path;

            check_environment();

            if (m_log_path.has_parent_path())
            {
                std::filesystem::create_directories(m_log_path.parent_path());
            }

            // Open in append mode
            m_file_stream.open(m_log_path, std::ios::out | std::ios::app);

            if (m_file_stream.is_open())
            {
                m_initialized = true;

                // Write session separator
                m_file_stream << "\n====== VaultSavePhantomPortal LOG START ======\n";
                m_file_stream.flush();

                info("Logger initialized. Environment: " + std::string(m_debug_enabled ? "Development" : "Production"));
            }
        }

        void Logger::check_environment()
        {
            m_debug_enabled = false;

            // 1. Check Env Var: VaultSaveDev = JunSnake
#ifdef _WIN32
            char *buf = nullptr;
            size_t sz = 0;
            if (_dupenv_s(&buf, &sz, "VaultSaveDev") == 0 && buf != nullptr)
            {
                if (std::string(buf) == "JunSnake")
                {
                    m_debug_enabled = true;
                }
                free(buf);
            }
#else
            const char *env = std::getenv("VaultSaveDev");
            if (env && std::string(env) == "JunSnake")
            {
                m_debug_enabled = true;
            }
#endif

            // 2. Check file override (debug.enable)
            if (!m_debug_enabled && m_log_path.has_parent_path())
            {
                if (std::filesystem::exists(m_log_path.parent_path() / "debug.enable"))
                {
                    m_debug_enabled = true;
                }
            }
        }

        void Logger::close()
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            if (m_file_stream.is_open())
            {
                m_file_stream.close();
            }
            m_initialized = false;
        }

        void Logger::info(const std::string &message)
        {
            write_log("info", message, true);
        }

        void Logger::warn(const std::string &message)
        {
            write_log("warn", message, true);
        }

        void Logger::error(const std::string &message)
        {
            write_log("error", message, true);
        }

        void Logger::debug(const std::string &message)
        {
            if (m_debug_enabled)
            {
                write_log("debug", message, false); // Debug logs usually don't go to API unless very verbose needed
            }
        }

        void Logger::write_log(const std::string &level, const std::string &message, bool to_api)
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);

            // 1. Write to file
            if (m_initialized && m_file_stream.is_open())
            {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);

                std::stringstream ss;
                ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] "
                   << "[vspp] [" << level << "] [VaultSavePhantomPortal] " << message << "\n";

                m_file_stream << ss.str();
                m_file_stream.flush();
            }

            // 2. Write to REFramework API
            if (to_api)
            {
                if (auto *api = reframework::API::try_get())
                {
                    try
                    {
                        std::string formatted_msg = "[VaultSavePhantomPortal] " + message;
                        if (level == "info")
                            api->log_info(formatted_msg.c_str());
                        else if (level == "warn")
                            api->log_warn(formatted_msg.c_str());
                        else if (level == "error")
                            api->log_error(formatted_msg.c_str());
                    }
                    catch (...)
                    {
                        // Fallback or ignore if API fails
                    }
                }
            }
        }

    } // namespace core
} // namespace vspp

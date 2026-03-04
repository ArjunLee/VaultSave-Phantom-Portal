#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

namespace vspp {
namespace ui {

class LocalizationManager {
public:
    static LocalizationManager& get();

    void set_language(const std::string& lang_code);
    std::string get_text(const std::string& key);

private:
    LocalizationManager();
    ~LocalizationManager() = default;
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    std::string m_current_language{"System"};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_translations;
    std::mutex m_mutex;

    void load_translations();
};

} // namespace ui
} // namespace vspp

#include "LocalizationManager.hpp"

namespace vspp {
namespace ui {

LocalizationManager& LocalizationManager::get() {
    static LocalizationManager instance;
    return instance;
}

LocalizationManager::LocalizationManager() {
    load_translations();
}

void LocalizationManager::set_language(const std::string& lang_code) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current_language = lang_code;
}

std::string LocalizationManager::get_text(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Fallback to English if key missing in current lang
    std::string lang = m_current_language;
    if (m_translations.find(lang) == m_translations.end()) {
        lang = "en";
    }

    if (m_translations[lang].find(key) != m_translations[lang].end()) {
        return m_translations[lang][key];
    }

    if (lang != "en" && m_translations["en"].find(key) != m_translations["en"].end()) {
        return m_translations["en"][key];
    }

    return key;
}

void LocalizationManager::load_translations() {
    // In a real app, load from .json files. Here we hardcode a few for the demo.
    // Ideally this should parse the JSONs from the reframework/data/ folder.
    
    auto& en = m_translations["en"];
    en["window_title"] = "VaultSave Phantom Portal";
    en["save_pos"] = "Save Position";
    en["teleport"] = "Teleport";
    en["slot"] = "Slot";
    en["status"] = "Status";
    en["welcome"] = "Welcome to Phantom Portal!";
    en["settings"] = "Settings";
    
    auto& zh_cn = m_translations["zh_cn"];
    zh_cn["window_title"] = "幽灵传送门 (VaultSave)";
    zh_cn["save_pos"] = "保存坐标";
    zh_cn["teleport"] = "传送";
    zh_cn["slot"] = "存档槽";
    zh_cn["status"] = "状态";
    zh_cn["welcome"] = "欢迎使用幽灵传送门！";
    zh_cn["settings"] = "设置";
    
    // ... add others
}

} // namespace ui
} // namespace vspp

#include "core/WordApp.h"
#include "utils/FileUtils.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

WordApp::WordApp() {
    // 企业版初始化 - 使用模块化组件
    cout << "✓ WordApp core initialized with enterprise modules" << endl;
}

void WordApp::initialize_data() {
    // 企业版使用模块化数据管理，此方法保留以兼容接口
    cout << "✓ Enterprise data management modules ready" << endl;
}

json WordApp::get_learn_words(int page, int words_per_page) {
    // 使用 UserDataManager 获取学习单词，支持断点续传
    return data_manager.get_learn_words(page, words_per_page);
}

json WordApp::get_exam_words() {
    return data_manager.get_exam_words(20);
}

bool WordApp::update_mistakes_batch(const vector<string>& words_to_update) {
    return data_manager.update_mistakes_batch(words_to_update);
}

json WordApp::get_review_words(int page, int words_per_page) {
    return data_manager.get_review_words(page, words_per_page);
}

json WordApp::get_all_review_words() {
    return data_manager.get_all_review_words();
}

json WordApp::get_stats() {
    return data_manager.get_stats();
}

map<string, json> WordApp::get_local_dictionary() {
    return {
        {"hello", {{"en", "A greeting or expression of good will"}, {"zh", "你好；问候"}}},
        {"world", {{"en", "The earth and all its inhabitants"}, {"zh", "世界；地球"}}},
        {"computer", {{"en", "An electronic device for processing data"}, {"zh", "计算机；电脑"}}},
        {"study", {{"en", "To learn about something by reading, memorizing facts, attending school, etc."}, {"zh", "学习；研究"}}},
        {"learn", {{"en", "To gain knowledge or skill by studying, practicing, being taught, or experiencing something"}, {"zh", "学习；学会"}}},
        {"beautiful", {{"en", "Having beauty; pleasing to the senses or mind"}, {"zh", "美丽的；漂亮的"}}},
        {"amazing", {{"en", "Causing great surprise or wonder; astonishing"}, {"zh", "令人惊奇的；了不起的"}}},
        {"wonderful", {{"en", "Inspiring delight, pleasure, or admiration; extremely good"}, {"zh", "精彩的；美妙的"}}},
        {"excellent", {{"en", "Extremely good; outstanding"}, {"zh", "优秀的；杰出的"}}},
        {"important", {{"en", "Of great significance or value; likely to have a profound effect"}, {"zh", "重要的；重大的"}}},
        {"interesting", {{"en", "Arousing curiosity or interest; holding or catching the attention"}, {"zh", "有趣的；令人感兴趣的"}}},
        {"knowledge", {{"en", "Facts, information, and skills acquired through experience or education"}, {"zh", "知识；学问"}}},
        {"education", {{"en", "The process of receiving or giving systematic instruction"}, {"zh", "教育；培养"}}},
        {"progress", {{"en", "Forward movement toward a destination or goal"}, {"zh", "进步；进展"}}},
        {"success", {{"en", "The accomplishment of an aim or purpose"}, {"zh", "成功；成就"}}}
    };
}

json WordApp::dictionary_search(const string& word) {
    if (word.empty()) {
        return json{{"success", false}, {"error", "No word provided"}};
    }
    
    try {
        // 使用本地词典
        auto local_dict = get_local_dictionary();
        string lower_word = word;
        transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
        
        if (local_dict.find(lower_word) != local_dict.end()) {
            return json{
                {"success", true},
                {"word", word},
                {"data", {
                    {"word", word},
                    {"phonetics", json::array()},
                    {"meanings", json::array({
                        {
                            {"partOfSpeech", "noun/verb"},
                            {"definitions", json::array({
                                {{"definition", local_dict[lower_word]["en"]}}
                            })}
                        }
                    })},
                    {"chinese_translation", local_dict[lower_word]["zh"]},
                    {"source", "Local Dictionary"}
                }}
            };
        } else {
            return json{
                {"success", true},
                {"word", word},
                {"data", {
                    {"word", word},
                    {"phonetics", json::array()},
                    {"meanings", json::array({
                        {
                            {"partOfSpeech", ""},
                            {"definitions", json::array({
                                {{"definition", "No detailed information found for \"" + word + "\""}}
                            })}
                        }
                    })},
                    {"chinese_translation", "未找到 \"" + word + "\" 的翻译"},
                    {"source", "Not Found"}
                }}
            };
        }
    } catch (const exception& e) {
        return json{
            {"success", false},
            {"error", "Translation service unavailable: " + string(e.what())}
        };
    }
}

// ===== 用户认证相关方法实现 =====

json WordApp::login_user(const string& username) {
    json result = auth_manager.login(username);
    if (result["success"].get<bool>()) {
        // 登录成功，切换数据管理器到该用户
        data_manager.set_current_user(username);
    }
    return result;
}

json WordApp::register_user(const string& username) {
    return auth_manager.register_user(username);
}

json WordApp::get_current_user() {
    return auth_manager.get_current_user_info();
}

json WordApp::switch_user(const string& username) {
    json result = auth_manager.switch_user(username);
    if (result["success"].get<bool>()) {
        // 切换成功，更新数据管理器
        data_manager.set_current_user(username);
    }
    return result;
}

json WordApp::logout_user() {
    json result = auth_manager.logout();
    if (result["success"].get<bool>()) {
        // 登出成功，清空数据管理器
        data_manager.set_current_user("");
    }
    return result;
}

json WordApp::get_user_list() {
    return auth_manager.get_user_list();
}

json WordApp::delete_user(const string& username) {
    return auth_manager.delete_user(username);
}

// ===== 用户数据管理方法实现 =====

json WordApp::reset_user_progress(bool reset_mistakes, bool reset_position) {
    return data_manager.reset_progress(reset_mistakes, reset_position);
}

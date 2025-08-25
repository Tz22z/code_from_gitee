#include "UserDataManager.h"
#include "FileUtils.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;

UserDataManager::UserDataManager() : current_user("") {
    // 生产环境配置
    if (getenv("PRODUCTION")) {
        USERS_DIR = "/var/www/word-app/users/";
    } else {
        USERS_DIR = "data/users/";
    }
}

string UserDataManager::get_user_data_file(const string& username) {
    return USERS_DIR + username + ".json";
}

bool UserDataManager::load_user_data(const string& username) {
    string user_file = get_user_data_file(username);
    ifstream file(user_file);
    if (!file.is_open()) {
        cerr << "Error: Cannot open user data file for " << username << endl;
        return false;
    }
    
    try {
        file >> user_data;
        file.close();
        return true;
    } catch (const exception& e) {
        cerr << "Error loading user data: " << e.what() << endl;
        return false;
    }
}

bool UserDataManager::save_user_data() {
    if (current_user.empty()) {
        return false;
    }
    
    string user_file = get_user_data_file(current_user);
    ofstream file(user_file);
    if (!file.is_open()) {
        cerr << "Error: Cannot save user data file for " << current_user << endl;
        return false;
    }
    
    try {
        file << user_data.dump(4);
        file.close();
        return true;
    } catch (const exception& e) {
        cerr << "Error saving user data: " << e.what() << endl;
        return false;
    }
}

bool UserDataManager::set_current_user(const string& username) {
    if (username.empty()) {
        current_user = "";
        user_data = json::object();
        return true;
    }
    
    if (load_user_data(username)) {
        current_user = username;
        return true;
    }
    return false;
}

string UserDataManager::get_current_user() const {
    return current_user;
}

json UserDataManager::get_learn_words(int page, int words_per_page) {
    if (current_user.empty() || !user_data.contains("words")) {
        return {
            {"success", false},
            {"error", "No user data loaded"}
        };
    }
    
    vector<string> all_words;
    for (auto& [word, word_data] : user_data["words"].items()) {
        all_words.push_back(word);
    }
    sort(all_words.begin(), all_words.end());
    
    // 如果page为0，从上次学习位置开始
    if (page == 0) {
        int last_position = get_learn_position();
        page = (last_position / words_per_page) + 1;
    }
    
    int start_index = (page - 1) * words_per_page;
    int end_index = min(start_index + words_per_page, (int)all_words.size());
    
    if (start_index >= (int)all_words.size()) {
        // 已经学完所有单词
        return {
            {"success", true},
            {"words", json::array()},
            {"totalPages", (all_words.size() + words_per_page - 1) / words_per_page},
            {"currentPage", page},
            {"username", current_user},
            {"completed", true},
            {"message", "Congratulations! You have completed all words."}
        };
    }
    
    vector<string> paginated_words;
    for (int i = start_index; i < end_index; i++) {
        paginated_words.push_back(all_words[i]);
    }
    
    // 更新学习位置
    update_learn_position(start_index);
    
    int total_pages = (all_words.size() + words_per_page - 1) / words_per_page;
    
    return {
        {"success", true},
        {"words", paginated_words},
        {"totalPages", total_pages},
        {"currentPage", page},
        {"username", current_user},
        {"progress", {
            {"current_position", start_index},
            {"total_words", all_words.size()},
            {"completion_percentage", (double)start_index / all_words.size() * 100}
        }}
    };
}

json UserDataManager::get_exam_words(int count) {
    if (current_user.empty() || !user_data.contains("words")) {
        return {
            {"success", false},
            {"error", "No user data loaded"}
        };
    }
    
    vector<string> all_words;
    for (auto& [word, word_data] : user_data["words"].items()) {
        all_words.push_back(word);
    }
    
    if (all_words.size() <= count) {
        return {
            {"success", true},
            {"words", all_words}
        };
    }
    
    random_device rd;
    mt19937 g(rd());
    shuffle(all_words.begin(), all_words.end(), g);
    all_words.resize(count);
    
    return {
        {"success", true},
        {"words", all_words}
    };
}

bool UserDataManager::update_mistakes_batch(const vector<string>& words_to_update) {
    if (current_user.empty()) return false;
    
    // 重新加载数据以确保最新状态
    load_user_data(current_user);
    
    for (const string& word : words_to_update) {
        if (user_data["words"].contains(word)) {
            user_data["words"][word]["mistakes"] = user_data["words"][word]["mistakes"].get<int>() + 1;
            user_data["words"][word]["last_seen"] = time(nullptr);
        }
    }
    
    return save_user_data();
}

bool UserDataManager::update_correct_batch(const vector<string>& words_correct) {
    if (current_user.empty()) return false;
    
    // 重新加载数据以确保最新状态
    load_user_data(current_user);
    
    for (const string& word : words_correct) {
        if (user_data["words"].contains(word)) {
            user_data["words"][word]["correct_count"] = user_data["words"][word]["correct_count"].get<int>() + 1;
            user_data["words"][word]["last_seen"] = time(nullptr);
        }
    }
    
    return save_user_data();
}

json UserDataManager::get_review_words(int page, int words_per_page) {
    if (current_user.empty() || !user_data.contains("words")) {
        return {
            {"success", false},
            {"error", "No user data loaded"}
        };
    }
    
    vector<json> all_review_words;
    for (auto& [word, word_data] : user_data["words"].items()) {
        int mistakes = word_data["mistakes"].get<int>();
        if (mistakes > 0) {
            all_review_words.push_back({
                {"word", word},
                {"mistakes", mistakes},
                {"correct_count", word_data.value("correct_count", 0)},
                {"last_seen", word_data.value("last_seen", 0)}
            });
        }
    }
    
    // 按错误次数降序排序，错误次数相同则按最后见到时间升序
    sort(all_review_words.begin(), all_review_words.end(), 
         [](const json& a, const json& b) {
             int mistakes_a = a["mistakes"].get<int>();
             int mistakes_b = b["mistakes"].get<int>();
             if (mistakes_a != mistakes_b) {
                 return mistakes_a > mistakes_b;
             }
             return a["last_seen"].get<long>() < b["last_seen"].get<long>();
         });



    // 如果page为0或1，从上次复习位置开始（但复习位置要合理）
    if (page == 0 || (page == 1 && get_review_position() > 0)) {
        int last_position = get_review_position();
        // 如果上次位置超出范围，重置为0
        if (last_position >= (int)all_review_words.size()) {
            last_position = 0;
            update_review_position(0);
        }
        page = (last_position / words_per_page) + 1;
    }
    
    int start_index = (page - 1) * words_per_page;
    int end_index = min(start_index + words_per_page, (int)all_review_words.size());
    
    if (start_index >= (int)all_review_words.size()) {
        // 已经复习完所有错误单词
        return {
            {"success", true},
            {"review_words", json::array()},
            {"totalPages", (all_review_words.size() + words_per_page - 1) / words_per_page},
            {"currentPage", page},
            {"username", current_user},
            {"completed", true},
            {"message", "Congratulations! You have reviewed all mistake words."}
        };
    }
    
    vector<json> paginated_review_words;
    for (int i = start_index; i < end_index; i++) {
        paginated_review_words.push_back(all_review_words[i]);
    }
    
    // 更新复习位置
    // 如果这是最后一页，重置复习位置为0，这样下次复习可以从头开始
    if (end_index >= (int)all_review_words.size()) {
        update_review_position(0);
    } else {
        update_review_position(start_index);
    }
    
    int total_pages = (all_review_words.size() + words_per_page - 1) / words_per_page;
    
    return {
        {"success", true},
        {"review_words", paginated_review_words},
        {"totalPages", total_pages},
        {"currentPage", page},
        {"username", current_user},
        {"total_review", all_review_words.size()},
        {"progress", {
            {"current_position", start_index},
            {"total_review_words", all_review_words.size()},
            {"completion_percentage", all_review_words.size() > 0 ? (double)start_index / all_review_words.size() * 100 : 0}
        }}
    };
}

json UserDataManager::get_all_review_words() {
    if (current_user.empty() || !user_data.contains("words")) {
        return {
            {"success", false},
            {"error", "No user data loaded"}
        };
    }
    
    vector<json> all_review_words;
    for (auto& [word, word_data] : user_data["words"].items()) {
        int mistakes = word_data["mistakes"].get<int>();
        if (mistakes > 0) {
            all_review_words.push_back({
                {"word", word},
                {"mistakes", mistakes},
                {"correct_count", word_data.value("correct_count", 0)},
                {"last_seen", word_data.value("last_seen", 0)}
            });
        }
    }
    
    // 按错误次数降序排序，错误次数相同则按最后见到时间升序
    sort(all_review_words.begin(), all_review_words.end(), 
         [](const json& a, const json& b) {
             int mistakes_a = a["mistakes"].get<int>();
             int mistakes_b = b["mistakes"].get<int>();
             if (mistakes_a != mistakes_b) {
                 return mistakes_a > mistakes_b;
             }
             return a["last_seen"].get<long>() < b["last_seen"].get<long>();
         });
    
    return {
        {"success", true},
        {"review_words", all_review_words},
        {"total_review", all_review_words.size()}
    };
}

json UserDataManager::get_stats() {
    if (current_user.empty() || !user_data.contains("words")) {
        return {
            {"success", false},
            {"error", "No user data loaded"}
        };
    }
    
    int total_words = user_data["words"].size();
    int review_count = 0;
    int total_mistakes = 0;
    int total_correct = 0;
    
    for (auto& [word, word_data] : user_data["words"].items()) {
        int mistakes = word_data["mistakes"].get<int>();
        int correct = word_data.value("correct_count", 0);
        
        if (mistakes > 0) {
            review_count++;
        }
        total_mistakes += mistakes;
        total_correct += correct;
    }
    
    int known_count = total_words - review_count;
    double accuracy = (total_mistakes + total_correct) > 0 ? 
                     (double)total_correct / (total_mistakes + total_correct) * 100 : 100.0;
    
    return {
        {"success", true},
        {"stats", {
            {"total", total_words},
            {"known", known_count},
            {"review", review_count},
            {"total_mistakes", total_mistakes},
            {"total_correct", total_correct},
            {"accuracy", accuracy},
            {"current_position", get_learn_position()},
            {"completion_percentage", total_words > 0 ? (double)get_learn_position() / total_words * 100 : 0}
        }},
        {"username", current_user}
    };
}

bool UserDataManager::update_learn_position(int position) {
    if (current_user.empty()) return false;
    
    user_data["user_info"]["last_learn_position"] = position;
    return save_user_data();
}

int UserDataManager::get_learn_position() {
    if (current_user.empty() || !user_data.contains("user_info")) {
        return 0;
    }
    
    return user_data["user_info"].value("last_learn_position", 0);
}

bool UserDataManager::update_review_position(int position) {
    if (current_user.empty()) return false;
    
    user_data["user_info"]["last_review_position"] = position;
    return save_user_data();
}

int UserDataManager::get_review_position() {
    if (current_user.empty() || !user_data.contains("user_info")) {
        return 0;
    }
    
    return user_data["user_info"].value("last_review_position", 0);
}

json UserDataManager::reset_progress(bool reset_mistakes, bool reset_position) {
    if (current_user.empty()) {
        return {
            {"success", false},
            {"error", "No user logged in"}
        };
    }
    
    int reset_count = 0;
    
    if (reset_mistakes) {
        for (auto& [word, word_data] : user_data["words"].items()) {
            if (word_data["mistakes"].get<int>() > 0) {
                reset_count++;
            }
            word_data["mistakes"] = 0;
            word_data["correct_count"] = 0;
            word_data["last_seen"] = 0;
        }
    }
    
    if (reset_position) {
        user_data["user_info"]["last_learn_position"] = 0;
    }
    
    if (save_user_data()) {
        return {
            {"success", true},
            {"message", "Progress reset successfully"},
            {"reset_mistakes", reset_mistakes},
            {"reset_position", reset_position},
            {"words_reset", reset_count}
        };
    } else {
        return {
            {"success", false},
            {"error", "Failed to save reset progress"}
        };
    }
}

json UserDataManager::get_learning_history(int limit) {
    if (current_user.empty()) {
        return {
            {"success", false},
            {"error", "No user logged in"}
        };
    }
    
    // 这里可以扩展为真正的历史记录功能
    // 目前返回基本的用户信息
    return {
        {"success", true},
        {"history", {
            {"user_info", user_data["user_info"]},
            {"recent_activity", "Learning history feature coming soon"}
        }}
    };
}

bool UserDataManager::record_learning_session(const string& session_type, int words_count, 
                                             int correct_count, int duration) {
    if (current_user.empty()) return false;
    
    // 更新总学习时间
    int total_time = user_data["user_info"].value("total_learning_time", 0);
    user_data["user_info"]["total_learning_time"] = total_time + duration;
    
    // 更新最后活动时间
    user_data["user_info"]["last_activity"] = time(nullptr);
    
    return save_user_data();
}

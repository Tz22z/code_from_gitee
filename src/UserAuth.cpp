#include "UserAuth.h"
#include "FileUtils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <regex>

namespace fs = std::filesystem;

UserAuth::UserAuth() : current_user("") {
    // 生产环境配置
    if (getenv("PRODUCTION")) {
        USERS_DIR = "/var/www/word-app/users/";
        WORDS_FILE = "/var/www/word-app/data/words.txt";
    } else {
        USERS_DIR = "data/users/";
        WORDS_FILE = "data/words.txt";
    }
    
    // 确保用户数据目录存在
    try {
        fs::create_directories(USERS_DIR);
        cout << "✓ User authentication system initialized" << endl;
        cout << "  Users directory: " << USERS_DIR << endl;
    } catch (const fs::filesystem_error& e) {
        cerr << "✗ Failed to create users directory: " << e.what() << endl;
    }
}

UserAuth::~UserAuth() {
    // 清理会话信息
    active_sessions.clear();
}

string UserAuth::get_user_data_file(const string& username) {
    return USERS_DIR + username + ".json";
}

bool UserAuth::user_exists(const string& username) {
    string user_file = get_user_data_file(username);
    return fs::exists(user_file);
}

bool UserAuth::is_valid_username(const string& username) {
    // 用户名验证规则：3-20个字符，只能包含字母、数字、下划线、连字符
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    
    regex username_pattern("^[a-zA-Z0-9_-]+$");
    return regex_match(username, username_pattern);
}

bool UserAuth::create_user_data(const string& username) {
    try {
        ifstream words_file(WORDS_FILE);
        if (!words_file.is_open()) {
            cerr << "Error: Cannot open " << WORDS_FILE << endl;
            return false;
        }
        
        // 创建用户数据结构
        json user_data = json::object();
        
        // 用户信息
        user_data["user_info"] = {
            {"username", username},
            {"created_at", time(nullptr)},
            {"last_login", time(nullptr)},
            {"last_learn_position", 0},
            {"total_sessions", 0},
            {"total_learning_time", 0}
        };
        
        // 单词数据
        user_data["words"] = json::object();
        string word;
        while (getline(words_file, word)) {
            if (!word.empty()) {
                // 移除可能的回车符
                word.erase(word.find_last_not_of(" \n\r\t") + 1);
                user_data["words"][word] = {
                    {"mistakes", 0},
                    {"correct_count", 0},
                    {"last_seen", 0}
                };
            }
        }
        words_file.close();
        
        // 保存用户数据文件
        string user_file = get_user_data_file(username);
        ofstream data_file(user_file);
        if (!data_file.is_open()) {
            cerr << "Error: Cannot create user file " << user_file << endl;
            return false;
        }
        
        data_file << user_data.dump(4);
        data_file.close();
        
        cout << "✓ Created new user: " << username << endl;
        return true;
        
    } catch (const exception& e) {
        cerr << "Error creating user data: " << e.what() << endl;
        return false;
    }
}

json UserAuth::login(const string& username) {
    if (!is_valid_username(username)) {
        return {
            {"success", false},
            {"error", "Invalid username format. Use 3-20 characters: letters, numbers, underscore, hyphen only."}
        };
    }
    
    // 检查用户是否存在，不存在则自动创建
    if (!user_exists(username)) {
        if (!create_user_data(username)) {
            return {
                {"success", false},
                {"error", "Failed to create user data"}
            };
        }
    }
    
    // 设置当前用户
    current_user = username;
    
    // 更新最后登录时间
    update_last_login(username);
    
    // 创建会话信息
    active_sessions[username] = {
        {"login_time", time(nullptr)},
        {"session_id", username + "_" + to_string(time(nullptr))}
    };
    
    return {
        {"success", true},
        {"message", "Login successful"},
        {"username", username},
        {"session_id", active_sessions[username]["session_id"]},
        {"is_new_user", !user_exists(username)}
    };
}

json UserAuth::register_user(const string& username) {
    if (!is_valid_username(username)) {
        return {
            {"success", false},
            {"error", "Invalid username format"}
        };
    }
    
    if (user_exists(username)) {
        return {
            {"success", false},
            {"error", "User already exists"}
        };
    }
    
    if (create_user_data(username)) {
        return {
            {"success", true},
            {"message", "User registered successfully"},
            {"username", username}
        };
    } else {
        return {
            {"success", false},
            {"error", "Failed to create user"}
        };
    }
}

string UserAuth::get_current_user() const {
    return current_user;
}

json UserAuth::get_current_user_info() {
    if (current_user.empty()) {
        return {
            {"success", false},
            {"error", "No user logged in"}
        };
    }
    
    string user_file = get_user_data_file(current_user);
    ifstream file(user_file);
    if (!file.is_open()) {
        return {
            {"success", false},
            {"error", "Cannot load user data"}
        };
    }
    
    json user_data;
    file >> user_data;
    file.close();
    
    return {
        {"success", true},
        {"username", current_user},
        {"user_info", user_data["user_info"]},
        {"session_active", active_sessions.find(current_user) != active_sessions.end()}
    };
}

json UserAuth::switch_user(const string& username) {
    if (username == current_user) {
        return {
            {"success", true},
            {"message", "Already logged in as " + username}
        };
    }
    
    // 登出当前用户
    if (!current_user.empty()) {
        logout();
    }
    
    // 登录新用户
    return login(username);
}

json UserAuth::logout() {
    if (current_user.empty()) {
        return {
            {"success", false},
            {"error", "No user logged in"}
        };
    }
    
    string logged_out_user = current_user;
    
    // 清除会话
    active_sessions.erase(current_user);
    current_user = "";
    
    return {
        {"success", true},
        {"message", "Logged out successfully"},
        {"previous_user", logged_out_user}
    };
}

json UserAuth::get_user_list() {
    vector<json> users;
    
    try {
        for (const auto& entry : fs::directory_iterator(USERS_DIR)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                string username = entry.path().stem().string();
                
                // 读取用户信息
                ifstream file(entry.path());
                if (file.is_open()) {
                    json user_data;
                    file >> user_data;
                    file.close();
                    
                    users.push_back({
                        {"username", username},
                        {"created_at", user_data["user_info"]["created_at"]},
                        {"last_login", user_data["user_info"]["last_login"]},
                        {"is_current", username == current_user}
                    });
                }
            }
        }
        
        // 按最后登录时间排序
        sort(users.begin(), users.end(), [](const json& a, const json& b) {
            return a["last_login"].get<long>() > b["last_login"].get<long>();
        });
        
    } catch (const fs::filesystem_error& e) {
        return {
            {"success", false},
            {"error", string("Failed to read users directory: ") + e.what()}
        };
    }
    
    return {
        {"success", true},
        {"users", users},
        {"total_users", users.size()},
        {"current_user", current_user}
    };
}

json UserAuth::delete_user(const string& username) {
    if (username == current_user) {
        return {
            {"success", false},
            {"error", "Cannot delete currently logged in user"}
        };
    }
    
    if (!user_exists(username)) {
        return {
            {"success", false},
            {"error", "User does not exist"}
        };
    }
    
    try {
        string user_file = get_user_data_file(username);
        fs::remove(user_file);
        
        return {
            {"success", true},
            {"message", "User deleted successfully"},
            {"deleted_user", username}
        };
    } catch (const fs::filesystem_error& e) {
        return {
            {"success", false},
            {"error", string("Failed to delete user: ") + e.what()}
        };
    }
}

void UserAuth::update_last_login(const string& username) {
    string user_file = get_user_data_file(username);
    ifstream file(user_file);
    if (!file.is_open()) return;
    
    json user_data;
    file >> user_data;
    file.close();
    
    user_data["user_info"]["last_login"] = time(nullptr);
    user_data["user_info"]["total_sessions"] = user_data["user_info"]["total_sessions"].get<int>() + 1;
    
    ofstream out_file(user_file);
    out_file << user_data.dump(4);
    out_file.close();
}

json UserAuth::get_user_stats(const string& username) {
    string target_user = username.empty() ? current_user : username;
    
    if (target_user.empty()) {
        return {
            {"success", false},
            {"error", "No user specified"}
        };
    }
    
    string user_file = get_user_data_file(target_user);
    ifstream file(user_file);
    if (!file.is_open()) {
        return {
            {"success", false},
            {"error", "Cannot load user data"}
        };
    }
    
    json user_data;
    file >> user_data;
    file.close();
    
    // 计算统计信息
    int total_words = user_data["words"].size();
    int words_with_mistakes = 0;
    int total_mistakes = 0;
    
    for (auto& [word, word_data] : user_data["words"].items()) {
        int mistakes = word_data["mistakes"].get<int>();
        if (mistakes > 0) {
            words_with_mistakes++;
            total_mistakes += mistakes;
        }
    }
    
    return {
        {"success", true},
        {"username", target_user},
        {"stats", {
            {"total_words", total_words},
            {"known_words", total_words - words_with_mistakes},
            {"review_needed", words_with_mistakes},
            {"total_mistakes", total_mistakes},
            {"accuracy", total_words > 0 ? (double)(total_words - words_with_mistakes) / total_words * 100 : 0},
            {"total_sessions", user_data["user_info"]["total_sessions"]},
            {"created_at", user_data["user_info"]["created_at"]},
            {"last_login", user_data["user_info"]["last_login"]}
        }}
    };
}

#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

/**
 * @brief 用户认证和会话管理类
 * 
 * 负责用户登录、会话管理、用户数据存储等认证相关功能
 */
class UserAuth {
private:
    string USERS_DIR;           ///< 用户数据目录
    string WORDS_FILE;          ///< 单词模板文件路径
    string current_user;        ///< 当前登录用户
    map<string, json> active_sessions; ///< 活跃用户会话

    /**
     * @brief 获取用户数据文件路径
     * @param username 用户名
     * @return 用户数据文件完整路径
     */
    string get_user_data_file(const string& username);

    /**
     * @brief 检查用户是否存在
     * @param username 用户名
     * @return 用户是否存在
     */
    bool user_exists(const string& username);

    /**
     * @brief 为新用户创建数据文件
     * @param username 用户名
     * @return 创建是否成功
     */
    bool create_user_data(const string& username);

    /**
     * @brief 验证用户名格式
     * @param username 用户名
     * @return 用户名是否有效
     */
    bool is_valid_username(const string& username);

public:
    /**
     * @brief 构造函数，初始化用户认证系统
     */
    UserAuth();

    /**
     * @brief 析构函数
     */
    ~UserAuth();

    /**
     * @brief 用户登录
     * @param username 用户名
     * @return JSON格式的登录结果
     */
    json login(const string& username);

    /**
     * @brief 用户注册（自动创建）
     * @param username 用户名
     * @return JSON格式的注册结果
     */
    json register_user(const string& username);

    /**
     * @brief 获取当前登录用户
     * @return 当前用户名，未登录返回空字符串
     */
    string get_current_user() const;

    /**
     * @brief 获取当前用户完整信息
     * @return JSON格式的用户信息
     */
    json get_current_user_info();

    /**
     * @brief 切换用户
     * @param username 目标用户名
     * @return JSON格式的切换结果
     */
    json switch_user(const string& username);

    /**
     * @brief 用户登出
     * @return JSON格式的登出结果
     */
    json logout();

    /**
     * @brief 获取所有已注册用户列表
     * @return JSON格式的用户列表
     */
    json get_user_list();

    /**
     * @brief 删除用户（管理功能）
     * @param username 要删除的用户名
     * @return JSON格式的删除结果
     */
    json delete_user(const string& username);

    /**
     * @brief 更新用户最后登录时间
     * @param username 用户名
     */
    void update_last_login(const string& username);

    /**
     * @brief 获取用户统计信息
     * @param username 用户名，为空则使用当前用户
     * @return JSON格式的用户统计
     */
    json get_user_stats(const string& username = "");
};

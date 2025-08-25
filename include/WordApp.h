#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "UserAuth.h"
#include "UserDataManager.h"

using json = nlohmann::json;
using namespace std;

/**
 * @brief 单词学习应用核心类
 * 
 * 负责管理单词数据、学习进度、错误统计等核心业务逻辑
 */
class WordApp {
private:
    UserAuth auth_manager;              ///< 用户认证管理器
    UserDataManager data_manager;       ///< 用户数据管理器

    /**
     * @brief 获取本地词典数据
     * @return 本地词典映射
     */
    map<string, json> get_local_dictionary();
    
    /**
     * @brief 查询金山词霸API
     * @param word 要查询的单词
     * @return JSON格式的API响应
     */
    json query_iciba_api(const string& word);
    
    /**
     * @brief 转换金山词霸API响应格式
     * @param word 查询的单词
     * @param iciba_data 金山词霸API原始响应
     * @return 转换后的标准格式JSON
     */
    json convert_iciba_response(const string& word, const json& iciba_data);
    
    /**
     * @brief 转换Free Dictionary API响应格式
     * @param word 查询的单词
     * @param api_data Free Dictionary API原始响应
     * @return 转换后的标准格式JSON
     */
    json convert_free_dictionary_response(const string& word, const json& api_data);
    
    /**
     * @brief 获取单词的中文翻译
     * @param word 要翻译的单词
     * @param definitions 英文定义列表（用于辅助翻译）
     * @return 中文翻译字符串
     */
    string get_chinese_translation(const string& word, const vector<string>& definitions = {});

public:
    /**
     * @brief 构造函数，初始化应用
     */
    WordApp();

    /**
     * @brief 初始化数据文件
     * 
     * 如果数据文件不存在，从单词文件创建新的数据文件
     */
    void initialize_data();

    /**
     * @brief 获取学习单词（分页）
     * @param page 页码（从1开始）
     * @param words_per_page 每页单词数
     * @return JSON格式的分页单词数据
     */
    json get_learn_words(int page = 1, int words_per_page = 20);

    /**
     * @brief 获取考试单词（随机）
     * @return JSON格式的随机单词数组
     */
    json get_exam_words();

    /**
     * @brief 批量更新错误次数
     * @param words_to_update 需要增加错误计数的单词列表
     * @return 操作是否成功
     */
    bool update_mistakes_batch(const vector<string>& words_to_update);

    /**
     * @brief 获取复习单词列表（分页）
     * @param page 页码（从1开始）
     * @param words_per_page 每页单词数
     * @return JSON格式的复习单词数据（按错误次数排序）
     */
    json get_review_words(int page = 1, int words_per_page = 20);

    /**
     * @brief 获取所有复习单词列表（用于侧边栏显示）
     * @return JSON格式的所有复习单词数据
     */
    json get_all_review_words();

    /**
     * @brief 获取学习统计信息
     * @return JSON格式的统计数据
     */
    json get_stats();

    /**
     * @brief 词典搜索功能
     * @param word 要查询的单词
     * @return JSON格式的查询结果
     */
    json dictionary_search(const string& word);

    // ===== 用户认证相关方法 =====
    
    /**
     * @brief 用户登录
     * @param username 用户名
     * @return JSON格式的登录结果
     */
    json login_user(const string& username);

    /**
     * @brief 用户注册
     * @param username 用户名  
     * @return JSON格式的注册结果
     */
    json register_user(const string& username);

    /**
     * @brief 获取当前用户信息
     * @return JSON格式的用户信息
     */
    json get_current_user();

    /**
     * @brief 切换用户
     * @param username 新用户名
     * @return JSON格式的切换结果
     */
    json switch_user(const string& username);

    /**
     * @brief 用户登出
     * @return JSON格式的登出结果
     */
    json logout_user();

    /**
     * @brief 获取所有用户列表
     * @return JSON格式的用户列表
     */
    json get_user_list();

    /**
     * @brief 删除用户
     * @param username 要删除的用户名
     * @return JSON格式的删除结果
     */
    json delete_user(const string& username);

    // ===== 用户数据管理方法 =====

    /**
     * @brief 重置用户学习进度
     * @param reset_mistakes 是否重置错误记录
     * @param reset_position 是否重置学习位置
     * @return JSON格式的重置结果
     */
    json reset_user_progress(bool reset_mistakes = false, bool reset_position = true);
};

#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

/**
 * @brief 用户数据管理类
 * 
 * 负责管理用户的学习数据，包括单词进度、错误统计、学习位置等
 */
class UserDataManager {
private:
    string current_user;        ///< 当前用户
    string USERS_DIR;          ///< 用户数据目录
    json user_data;            ///< 当前用户数据

    /**
     * @brief 获取用户数据文件路径
     * @param username 用户名
     * @return 用户数据文件路径
     */
    string get_user_data_file(const string& username);

    /**
     * @brief 加载用户数据
     * @param username 用户名
     * @return 加载是否成功
     */
    bool load_user_data(const string& username);

    /**
     * @brief 保存用户数据
     * @return 保存是否成功
     */
    bool save_user_data();

public:
    /**
     * @brief 构造函数
     */
    UserDataManager();

    /**
     * @brief 设置当前用户
     * @param username 用户名
     * @return 设置是否成功
     */
    bool set_current_user(const string& username);

    /**
     * @brief 获取当前用户
     * @return 当前用户名
     */
    string get_current_user() const;

    /**
     * @brief 获取学习单词（分页，支持断点续传）
     * @param page 页码（从1开始，0表示从上次位置开始）
     * @param words_per_page 每页单词数
     * @return JSON格式的分页单词数据
     */
    json get_learn_words(int page = 0, int words_per_page = 20);

    /**
     * @brief 获取考试单词（随机）
     * @param count 单词数量
     * @return JSON格式的随机单词数组
     */
    json get_exam_words(int count = 20);

    /**
     * @brief 批量更新错误次数
     * @param words_to_update 需要增加错误计数的单词列表
     * @return 操作是否成功
     */
    bool update_mistakes_batch(const vector<string>& words_to_update);

    /**
     * @brief 批量更新正确次数
     * @param words_correct 正确的单词列表
     * @return 操作是否成功
     */
    bool update_correct_batch(const vector<string>& words_correct);

    /**
     * @brief 获取复习单词列表（支持分页）
     * @param page 页码（从1开始，0表示从上次位置开始）
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
     * @brief 更新学习位置
     * @param position 新的学习位置
     * @return 更新是否成功
     */
    bool update_learn_position(int position);

    /**
     * @brief 获取学习位置
     * @return 当前学习位置
     */
    int get_learn_position();

    /**
     * @brief 更新复习位置
     * @param position 新的复习位置
     * @return 更新是否成功
     */
    bool update_review_position(int position);

    /**
     * @brief 获取复习位置
     * @return 当前复习位置
     */
    int get_review_position();

    /**
     * @brief 重置用户进度
     * @param reset_mistakes 是否重置错误统计
     * @param reset_position 是否重置学习位置
     * @return JSON格式的重置结果
     */
    json reset_progress(bool reset_mistakes = false, bool reset_position = true);

    /**
     * @brief 获取用户学习历史
     * @param limit 返回记录数限制
     * @return JSON格式的学习历史
     */
    json get_learning_history(int limit = 50);

    /**
     * @brief 记录学习会话
     * @param session_type 会话类型（learn/exam/review）
     * @param words_count 学习单词数
     * @param correct_count 正确数
     * @param duration 学习时长（秒）
     * @return 记录是否成功
     */
    bool record_learning_session(const string& session_type, int words_count, 
                                int correct_count, int duration);
};

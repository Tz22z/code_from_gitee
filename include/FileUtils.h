1#pragma once

#include <string>

using namespace std;

/**
 * @brief 文件工具类
 * 
 * 提供文件操作相关的工具函数
 */
class FileUtils {
public:
    /**
     * @brief 检查文件是否存在
     * @param filepath 文件路径
     * @return 文件是否存在
     */
    static bool file_exists(const string& filepath);
    
    /**
     * @brief 读取文本文件内容
     * @param filepath 文件路径
     * @return 文件内容字符串
     */
    static string read_text_file(const string& filepath);
    
    /**
     * @brief 写入文本文件
     * @param filepath 文件路径
     * @param content 要写入的内容
     * @return 是否写入成功
     */
    static bool write_text_file(const string& filepath, const string& content);
    
    /**
     * @brief 获取文件扩展名
     * @param filename 文件名
     * @return 扩展名（不包含点号）
     */
    static string get_file_extension(const string& filename);
};

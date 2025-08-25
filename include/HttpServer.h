#pragma once

#include <httplib.h>
#include <memory>
#include "WordApp.h"

/**
 * @brief HTTP服务器类
 * 
 * 负责处理HTTP请求、路由管理、静态文件服务等
 */
class HttpServer {
private:
    httplib::Server server;           ///< HTTP服务器实例
    std::shared_ptr<WordApp> app;     ///< 单词应用实例
    
    /**
     * @brief 设置CORS头部
     */
    void setup_cors();
    
    /**
     * @brief 设置静态文件路由
     */
    void setup_static_routes();
    
    /**
     * @brief 设置API路由
     */
    void setup_api_routes();
    
    /**
     * @brief 获取文件的MIME类型
     * @param filename 文件名
     * @return MIME类型字符串
     */
    string get_content_type(const string& filename);

public:
    /**
     * @brief 构造函数
     * @param word_app 单词应用实例
     */
    explicit HttpServer(std::shared_ptr<WordApp> word_app);
    
    /**
     * @brief 启动服务器
     * @param host 绑定的主机地址
     * @param port 监听端口
     * @return 是否启动成功
     */
    bool start(const string& host = "0.0.0.0", int port = 8080);
    
    /**
     * @brief 停止服务器
     */
    void stop();
};

/**
 * @file main.cpp
 * @brief 单词学习应用主程序入口
 * @author Word Learning Team
 * @version 2.0
 * @date 2024
 */

#include "WordApp.h"
#include "HttpServer.h"
#include <iostream>
#include <memory>
#include <signal.h>

using namespace std;

// 全局服务器指针，用于信号处理
std::shared_ptr<HttpServer> global_server;

/**
 * @brief 信号处理函数，优雅地关闭服务器
 * @param signum 信号编号
 */
void signal_handler(int signum) {
    cout << "\nReceived signal " << signum << ". Shutting down gracefully..." << endl;
    if (global_server) {
        global_server->stop();
    }
    exit(0);
}

/**
 * @brief 主程序入口
 * @return 程序退出码
 */
int main() {
    try {
        // 设置信号处理
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        cout << "=== Word Learning Application v2.0 ===" << endl;
        cout << "Enterprise Edition - Modular C++ Architecture" << endl;
        cout << "===========================================" << endl;
        
        // 创建应用实例
        auto app = make_shared<WordApp>();
        cout << "✓ WordApp initialized successfully" << endl;
        
        // 创建HTTP服务器
        global_server = make_shared<HttpServer>(app);
        cout << "✓ HttpServer initialized successfully" << endl;
        
        // 启动服务器
        const string host = "0.0.0.0";
        const int port = 8080;
        
        cout << "✓ Starting server..." << endl;
        if (!global_server->start(host, port)) {
            cerr << "✗ Failed to start server" << endl;
            return 1;
        }
        
    } catch (const exception& e) {
        cerr << "✗ Fatal error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}

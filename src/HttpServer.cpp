#include "HttpServer.h"
#include "FileUtils.h"
#include <iostream>
#include <fstream>

HttpServer::HttpServer(std::shared_ptr<WordApp> word_app) : app(word_app) {
    setup_cors();
    setup_static_routes();
    setup_api_routes();
}

void HttpServer::setup_cors() {
    // 设置CORS
    server.set_pre_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    // 处理OPTIONS请求
    server.Options(".*", [](const httplib::Request&, httplib::Response&) {
        return;
    });
}

void HttpServer::setup_static_routes() {
    // 主页
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        string content = FileUtils::read_text_file("assets/html/index.html");
        if (!content.empty()) {
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });
    
    // 单词朗读器页面
    server.Get("/listen/?", [](const httplib::Request&, httplib::Response& res) {
        string content = FileUtils::read_text_file("listen/index.html");
        if (!content.empty()) {
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("Word Reader not found", "text/plain");
        }
    });
    
    // 静态文件服务
    server.Get(R"(/(.+\.(css|js|html|ico|png|jpg|jpeg|gif)))", [this](const httplib::Request& req, httplib::Response& res) {
        string filename = req.matches[1];
        string filepath;
        
        // 根据文件扩展名确定路径
        string ext = FileUtils::get_file_extension(filename);
        if (ext == "css") {
            filepath = "assets/css/" + filename;
        } else if (ext == "js") {
            filepath = "assets/js/" + filename;
        } else if (ext == "html") {
            filepath = "assets/html/" + filename;
        } else {
            filepath = "assets/" + filename;
        }
        
        string content = FileUtils::read_text_file(filepath);
        if (!content.empty()) {
            string content_type = get_content_type(filename);
            res.set_content(content, content_type.c_str());
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });
}

void HttpServer::setup_api_routes() {
    // API端点
    server.Get("/get_learn_words", [this](const httplib::Request& req, httplib::Response& res) {
        int page = 1;
        if (req.has_param("page")) {
            page = stoi(req.get_param_value("page"));
        }
        
        json result = app->get_learn_words(page);
        res.set_content(result.dump(), "application/json");
    });
    
    server.Get("/get_exam_words", [this](const httplib::Request&, httplib::Response& res) {
        json result = app->get_exam_words();
        res.set_content(result.dump(), "application/json");
    });
    
    server.Post("/update_mistakes_batch", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_data = json::parse(req.body);
            vector<string> words = request_data["words"];
            
            bool success = app->update_mistakes_batch(words);
            res.set_content(json{{"success", success}}.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Invalid data format"}}.dump(), "application/json");
        }
    });
    
    server.Get("/get_review_words", [this](const httplib::Request& req, httplib::Response& res) {
        int page = 1;
        if (req.has_param("page")) {
            page = stoi(req.get_param_value("page"));
        }
        
        json result = app->get_review_words(page);
        res.set_content(result.dump(), "application/json");
    });
    
    server.Get("/get_all_review_words", [this](const httplib::Request&, httplib::Response& res) {
        json result = app->get_all_review_words();
        res.set_content(result.dump(), "application/json");
    });
    
    server.Get("/get_stats", [this](const httplib::Request&, httplib::Response& res) {
        json result = app->get_stats();
        res.set_content(result.dump(), "application/json");
    });
    
    server.Get("/dictionary_search", [this](const httplib::Request& req, httplib::Response& res) {
        string word;
        if (req.has_param("word")) {
            word = req.get_param_value("word");
        }
        
        json result = app->dictionary_search(word);
        res.set_content(result.dump(), "application/json");
    });
    
    // ===== 用户认证 API =====
    
    // 用户登录
    server.Post("/login", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_data = json::parse(req.body);
            string username = request_data["username"];
            
            json result = app->login_user(username);
            res.set_content(result.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Invalid request format"}}.dump(), "application/json");
        }
    });
    
    // 用户注册
    server.Post("/register", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_data = json::parse(req.body);
            string username = request_data["username"];
            
            json result = app->register_user(username);
            res.set_content(result.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Invalid request format"}}.dump(), "application/json");
        }
    });
    
    // 获取当前用户信息
    server.Get("/current_user", [this](const httplib::Request&, httplib::Response& res) {
        json result = app->get_current_user();
        res.set_content(result.dump(), "application/json");
    });
    
    // 切换用户
    server.Post("/switch_user", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_data = json::parse(req.body);
            string username = request_data["username"];
            
            json result = app->switch_user(username);
            res.set_content(result.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Invalid request format"}}.dump(), "application/json");
        }
    });
    
    // 用户登出
    server.Post("/logout", [this](const httplib::Request&, httplib::Response& res) {
        json result = app->logout_user();
        res.set_content(result.dump(), "application/json");
    });
    
    // 获取用户列表
    server.Get("/users", [this](const httplib::Request&, httplib::Response& res) {
        json result = app->get_user_list();
        res.set_content(result.dump(), "application/json");
    });
    
    // 删除用户
    server.Delete(R"(/users/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string username = req.matches[1];
        json result = app->delete_user(username);
        res.set_content(result.dump(), "application/json");
    });
    
    // ===== 用户数据管理 API =====
    
    // 重置用户进度
    server.Post("/reset_progress", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_data = json::parse(req.body);
            bool reset_mistakes = request_data.value("reset_mistakes", false);
            bool reset_position = request_data.value("reset_position", true);
            
            json result = app->reset_user_progress(reset_mistakes, reset_position);
            res.set_content(result.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Invalid request format"}}.dump(), "application/json");
        }
    });
    
    // ===== 单词朗读器 API =====
    
    // 单个单词朗读API
    server.Post("/speak", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_data = json::parse(req.body);
            string word = request_data["word"];
            
            if (word.empty()) {
                res.status = 400;
                res.set_content(json{{"status", "error"}, {"message", "No word provided"}}.dump(), "application/json");
                return;
            }
            
            // 调用C++朗读程序
            string command = "cd listen && ./word_reader \"" + word + "\"";
            int result = system(command.c_str());
            
            if (result == 0) {
                res.set_content(json{{"status", "success"}, {"message", "Spoke word: " + word}}.dump(), "application/json");
            } else {
                res.set_content(json{{"status", "error"}, {"message", "Failed to speak word: " + word}}.dump(), "application/json");
            }
            
        } catch (const exception& e) {
            res.status = 500;
            res.set_content(json{{"status", "error"}, {"message", string("Internal server error: ") + e.what()}}.dump(), "application/json");
        }
    });
}

string HttpServer::get_content_type(const string& filename) {
    string ext = FileUtils::get_file_extension(filename);
    
    if (ext == "css") return "text/css";
    else if (ext == "js") return "application/javascript";
    else if (ext == "html") return "text/html";
    else if (ext == "png") return "image/png";
    else if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    else if (ext == "gif") return "image/gif";
    else if (ext == "ico") return "image/x-icon";
    else return "text/plain";
}

bool HttpServer::start(const string& host, int port) {
    cout << "Starting C++ Word Learning Server on " << host << ":" << port << "..." << endl;
    cout << "Open your browser and visit: http://localhost:" << port << endl;
    cout << "Press Ctrl+C to stop the server." << endl;
    
    if (!server.listen(host.c_str(), port)) {
        cerr << "Error: Failed to start server on port " << port << endl;
        cerr << "The port might be already in use." << endl;
        return false;
    }
    
    return true;
}

void HttpServer::stop() {
    server.stop();
}

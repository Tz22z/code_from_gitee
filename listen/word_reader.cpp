#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

class WordReader {
private:
    std::string tts_command;
    int speed;
    int pause_duration; // milliseconds between words
    
public:
    WordReader(int speech_speed = 150, int word_pause = 800) 
        : speed(speech_speed), pause_duration(word_pause) {
        // 检查可用的TTS引擎
        if (system("which espeak > /dev/null 2>&1") == 0) {
            tts_command = "espeak";
        } else if (system("which festival > /dev/null 2>&1") == 0) {
            tts_command = "festival --tts";
        } else {
            std::cerr << "Warning: No TTS engine found. Please install espeak or festival." << std::endl;
            tts_command = "echo"; // fallback for testing
        }
    }
    
    // 清理和分割单词
    std::vector<std::string> splitWords(const std::string& text) {
        std::vector<std::string> words;
        std::stringstream ss(text);
        std::string word;
        
        // 按空格分割
        while (ss >> word) {
            // 移除标点符号
            word.erase(std::remove_if(word.begin(), word.end(), 
                [](char c) { return std::ispunct(c); }), word.end());
            
            // 转换为小写
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            
            if (!word.empty()) {
                words.push_back(word);
            }
        }
        
        return words;
    }
    
    // 朗读单个单词
    void speakWord(const std::string& word) {
        std::string command;
        
        if (tts_command == "espeak") {
            command = "espeak -s " + std::to_string(speed) + " \"" + word + "\"";
        } else if (tts_command.find("festival") != std::string::npos) {
            command = "echo \"" + word + "\" | festival --tts";
        } else {
            command = "echo \"Speaking: " + word + "\"";
        }
        
        system(command.c_str());
    }
    
    // 依次朗读所有单词
    void readWords(const std::string& text) {
        std::vector<std::string> words = splitWords(text);
        
        std::cout << "开始朗读 " << words.size() << " 个单词..." << std::endl;
        
        for (size_t i = 0; i < words.size(); i++) {
            std::cout << "[" << (i + 1) << "/" << words.size() << "] " << words[i] << std::endl;
            speakWord(words[i]);
            
            // 单词间暂停
            if (i < words.size() - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(pause_duration));
            }
        }
        
        std::cout << "朗读完成！" << std::endl;
    }
    
    // 设置语速
    void setSpeed(int new_speed) {
        speed = new_speed;
    }
    
    // 设置单词间暂停时间
    void setPauseDuration(int pause_ms) {
        pause_duration = pause_ms;
    }
};

// HTTP服务器处理函数
void handleHttpRequest() {
    // 简单的HTTP服务器实现
    std::cout << "Starting HTTP server on port 8080..." << std::endl;
    
    // 这里使用Python的http.server作为简单的解决方案
    system("cd /opt/listen && python3 -m http.server 8080 > /dev/null 2>&1 &");
}

int main(int argc, char* argv[]) {
    WordReader reader;
    
    if (argc > 1) {
        // 命令行模式
        if (std::string(argv[1]) == "--server") {
            handleHttpRequest();
            
            std::cout << "HTTP server started. Visit http://localhost:8080" << std::endl;
            std::cout << "Press Ctrl+C to stop the server." << std::endl;
            
            // 保持程序运行
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            // 直接朗读命令行参数
            std::string text;
            for (int i = 1; i < argc; i++) {
                text += argv[i];
                if (i < argc - 1) text += " ";
            }
            reader.readWords(text);
        }
    } else {
        // 交互模式
        std::cout << "=== 单词朗读器 ===" << std::endl;
        std::cout << "请输入要朗读的单词（用空格分隔），输入 'quit' 退出：" << std::endl;
        
        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "exit") {
                break;
            }
            
            if (!input.empty()) {
                reader.readWords(input);
                std::cout << std::endl;
            }
        }
    }
    
    return 0;
}

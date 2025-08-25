#include "WordApp.h"
#include "FileUtils.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <filesystem>
#include <httplib.h>

namespace fs = std::filesystem;

WordApp::WordApp() {
    // 企业版初始化 - 使用模块化组件
    cout << "✓ WordApp core initialized with enterprise modules" << endl;
}

void WordApp::initialize_data() {
    // 企业版使用模块化数据管理，此方法保留以兼容接口
    cout << "✓ Enterprise data management modules ready" << endl;
}

json WordApp::get_learn_words(int page, int words_per_page) {
    // 使用 UserDataManager 获取学习单词，支持断点续传
    return data_manager.get_learn_words(page, words_per_page);
}

json WordApp::get_exam_words() {
    return data_manager.get_exam_words(20);
}

bool WordApp::update_mistakes_batch(const vector<string>& words_to_update) {
    return data_manager.update_mistakes_batch(words_to_update);
}

json WordApp::get_review_words(int page, int words_per_page) {
    return data_manager.get_review_words(page, words_per_page);
}

json WordApp::get_all_review_words() {
    return data_manager.get_all_review_words();
}

json WordApp::get_stats() {
    return data_manager.get_stats();
}

map<string, json> WordApp::get_local_dictionary() {
    // 简化的本地词典，仅作为金山词霸API的备用
    return {
        {"hello", {{"en", "A greeting or expression of good will"}, {"zh", "你好；问候"}}},
        {"world", {{"en", "The earth and all its inhabitants"}, {"zh", "世界；地球"}}},
        {"computer", {{"en", "An electronic device for processing data"}, {"zh", "计算机；电脑"}}},
        {"study", {{"en", "To learn about something by reading, memorizing facts, attending school, etc."}, {"zh", "学习；研究"}}},
        {"learn", {{"en", "To gain knowledge or skill by studying, practicing, being taught, or experiencing something"}, {"zh", "学习；学会"}}},
        {"beautiful", {{"en", "Having beauty; pleasing to the senses or mind"}, {"zh", "美丽的；漂亮的"}}},
        {"important", {{"en", "Of great significance or value; likely to have a profound effect"}, {"zh", "重要的；重大的"}}},
        {"knowledge", {{"en", "Facts, information, and skills acquired through experience or education"}, {"zh", "知识；学问"}}},
        {"education", {{"en", "The process of receiving or giving systematic instruction"}, {"zh", "教育；培养"}}},
        {"progress", {{"en", "Forward movement toward a destination or goal"}, {"zh", "进步；进展"}}}
    };
}

json WordApp::dictionary_search(const string& word) {
    if (word.empty()) {
        return json{{"success", false}, {"error", "No word provided"}};
    }
    
    try {
        // 首先尝试金山词霸API
        cout << "[DEBUG] Searching for word: " << word << endl;
        json iciba_result = query_iciba_api(word);
        
        // 打印调试信息
        if (iciba_result.contains("success")) {
            cout << "[DEBUG] Iciba API result success: " << iciba_result["success"].get<bool>() << endl;
            if (iciba_result.contains("error")) {
                cout << "[DEBUG] Iciba API error: " << iciba_result["error"].get<string>() << endl;
            }
        }
        
        if (iciba_result["success"].get<bool>()) {
            cout << "[DEBUG] Using Iciba API result" << endl;
            return iciba_result;
        }
        
        cout << "[DEBUG] Iciba API failed, falling back to local dictionary" << endl;
        
        // 如果金山词霸API失败，使用本地词典作为备选
        auto local_dict = get_local_dictionary();
        string lower_word = word;
        transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
        
        if (local_dict.find(lower_word) != local_dict.end()) {
            cout << "[DEBUG] Found in local dictionary" << endl;
            return json{
                {"success", true},
                {"word", word},
                {"data", {
                    {"word", word},
                    {"phonetics", json::array()},
                    {"meanings", json::array({
                        {
                            {"partOfSpeech", "noun/verb"},
                            {"definitions", json::array({
                                {{"definition", local_dict[lower_word]["en"]}}
                            })}
                        }
                    })},
                    {"chinese_translation", local_dict[lower_word]["zh"]},
                    {"source", "Local Dictionary (Backup)"}
                }}
            };
        } else {
            cout << "[DEBUG] Word not found in any dictionary" << endl;
            // 返回未找到的信息，但标明API错误原因
            string error_msg = iciba_result.contains("error") ? 
                iciba_result["error"].get<string>() : "API unavailable";
            
            return json{
                {"success", true},
                {"word", word},
                {"data", {
                    {"word", word},
                    {"phonetics", json::array()},
                    {"meanings", json::array({
                        {
                            {"partOfSpeech", ""},
                            {"definitions", json::array({
                                {{"definition", "Dictionary service error: " + error_msg}}
                            })}
                        }
                    })},
                    {"chinese_translation", "未找到 \"" + word + "\" 的翻译"},
                    {"source", "Not Found"}
                }}
            };
        }
    } catch (const exception& e) {
        return json{
            {"success", false},
            {"error", "Translation service unavailable: " + string(e.what())}
        };
    }
}

// ===== 用户认证相关方法实现 =====

json WordApp::login_user(const string& username) {
    json result = auth_manager.login(username);
    if (result["success"].get<bool>()) {
        // 登录成功，切换数据管理器到该用户
        data_manager.set_current_user(username);
    }
    return result;
}

json WordApp::register_user(const string& username) {
    return auth_manager.register_user(username);
}

json WordApp::get_current_user() {
    return auth_manager.get_current_user_info();
}

json WordApp::switch_user(const string& username) {
    json result = auth_manager.switch_user(username);
    if (result["success"].get<bool>()) {
        // 切换成功，更新数据管理器
        data_manager.set_current_user(username);
    }
    return result;
}

json WordApp::logout_user() {
    json result = auth_manager.logout();
    if (result["success"].get<bool>()) {
        // 登出成功，清空数据管理器
        data_manager.set_current_user("");
    }
    return result;
}

json WordApp::get_user_list() {
    return auth_manager.get_user_list();
}

json WordApp::delete_user(const string& username) {
    return auth_manager.delete_user(username);
}

// ===== 词典API集成 =====

string WordApp::get_chinese_translation(const string& word, const vector<string>& definitions) {
    // 内置常用词汇的中文翻译词典
    static map<string, string> chinese_dict = {
        {"hello", "你好；问候"},
        {"world", "世界；地球"},
        {"ammunition", "弹药；军火"},
        {"annexation", "吞并；合并；附加"},
        {"apple", "苹果"},
        {"book", "书；书籍"},
        {"computer", "计算机；电脑"},
        {"develop", "发展；开发；研制"},
        {"education", "教育；培养"},
        {"friend", "朋友；友人"},
        {"good", "好的；优良的"},
        {"happy", "快乐的；幸福的"},
        {"important", "重要的；重大的"},
        {"knowledge", "知识；学问"},
        {"language", "语言；语言文字"},
        {"learn", "学习；学会"},
        {"love", "爱；热爱"},
        {"money", "金钱；货币"},
        {"network", "网络；网状系统"},
        {"opportunity", "机会；时机"},
        {"people", "人；人们"},
        {"progress", "进步；进展"},
        {"question", "问题；疑问"},
        {"research", "研究；调查"},
        {"science", "科学；自然科学"},
        {"technology", "技术；科技"},
        {"understand", "理解；明白"},
        {"value", "价值；价值观"},
        {"work", "工作；劳动"},
        {"year", "年；年度"},
        {"zero", "零；零度"},
        {"abandon", "放弃；抛弃"},
        {"ability", "能力；才能"},
        {"accept", "接受；承认"},
        {"achieve", "实现；达到"},
        {"action", "行动；行为"},
        {"address", "地址；演说"},
        {"advance", "前进；进步"},
        {"advantage", "优势；有利条件"},
        {"advice", "建议；忠告"},
        {"affect", "影响；感动"},
        {"afraid", "害怕的；担心的"},
        {"after", "在...之后"},
        {"again", "再次；又"},
        {"against", "反对；违反"},
        {"age", "年龄；时代"},
        {"agree", "同意；赞成"},
        {"aid", "援助；帮助"},
        {"aim", "目标；目的"},
        {"air", "空气；大气"},
        {"allow", "允许；准许"},
        {"almost", "几乎；差不多"},
        {"alone", "独自；单独"},
        {"along", "沿着；一起"},
        {"already", "已经；早已"},
        {"also", "也；而且"},
        {"although", "虽然；尽管"},
        {"always", "总是；永远"},
        {"among", "在...之中"},
        {"amount", "数量；总额"},
        {"analysis", "分析；解析"},
        {"ancient", "古代的；古老的"},
        {"anger", "愤怒；生气"},
        {"animal", "动物；兽类"},
        {"announce", "宣布；公布"},
        {"annual", "每年的；年度的"},
        {"another", "另一个；再一个"},
        {"answer", "回答；答案"},
        {"anxiety", "焦虑；忧虑"},
        {"any", "任何；一些"},
        {"appear", "出现；显得"},
        {"application", "应用；申请"},
        {"apply", "申请；应用"},
        {"approach", "方法；接近"},
        {"appropriate", "适当的；恰当的"},
        {"approve", "批准；赞成"},
        {"area", "地区；面积"},
        {"argue", "争论；辩论"},
        {"arise", "出现；发生"},
        {"arm", "手臂；武器"},
        {"army", "军队；陆军"},
        {"around", "在...周围"},
        {"arrange", "安排；整理"},
        {"arrest", "逮捕；阻止"},
        {"arrive", "到达；抵达"},
        {"art", "艺术；美术"},
        {"article", "文章；物品"},
        {"artist", "艺术家；画家"},
        {"ask", "问；请求"},
        {"aspect", "方面；外观"},
        {"assess", "评估；评定"},
        {"assist", "帮助；协助"},
        {"associate", "联系；关联"},
        {"assume", "假设；承担"},
        {"assure", "保证；确保"},
        {"atmosphere", "大气；气氛"},
        {"attach", "附加；依恋"},
        {"attack", "攻击；进攻"},
        {"attempt", "尝试；企图"},
        {"attend", "出席；照料"},
        {"attention", "注意；关注"},
        {"attitude", "态度；看法"},
        {"attract", "吸引；引起"},
        {"audience", "观众；听众"},
        {"author", "作者；作家"},
        {"authority", "权威；当局"},
        {"available", "可用的；可获得的"},
        {"average", "平均的；普通的"},
        {"avoid", "避免；回避"},
        {"aware", "意识到的；知道的"},
        {"away", "离开；远离"}
    };
    
    // 转换为小写进行查找
    string lower_word = word;
    transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
    
    // 查找内置词典
    auto it = chinese_dict.find(lower_word);
    if (it != chinese_dict.end()) {
        return it->second;
    }
    
    // 如果没有找到，尝试使用简单的规则翻译
    if (!definitions.empty()) {
        // 根据定义中的关键词提供简单的中文提示
        string first_def = definitions[0];
        transform(first_def.begin(), first_def.end(), first_def.begin(), ::tolower);
        
        if (first_def.find("weapon") != string::npos) {
            return "武器相关";
        } else if (first_def.find("military") != string::npos) {
            return "军事相关";
        } else if (first_def.find("government") != string::npos) {
            return "政府相关";
        } else if (first_def.find("territory") != string::npos) {
            return "领土相关";
        } else if (first_def.find("incorporate") != string::npos) {
            return "合并；纳入";
        } else if (first_def.find("attach") != string::npos) {
            return "附加；连接";
        } else if (first_def.find("add") != string::npos) {
            return "添加；增加";
        } else if (first_def.find("join") != string::npos) {
            return "加入；连接";
        } else if (first_def.find("supply") != string::npos) {
            return "供应；补给";
        } else if (first_def.find("projectile") != string::npos) {
            return "弹药；投射物";
        }
    }
    
    return "";
}

json WordApp::query_iciba_api(const string& word) {
    try {
        cout << "[DEBUG] Querying Iciba API for: " << word << endl;
        
        // 使用有道词典API（备用方案）和金山词霸
        string url = "https://fanyi.youdao.com/openapi.do?keyfrom=dict&key=null&type=data&doctype=json&version=1.1&q=" + word;
        cout << "[DEBUG] Request URL: " << url << endl;
        
        // 使用curl命令获取数据，设置用户代理和较短超时
        string cmd = "curl -s -L --connect-timeout 5 --max-time 8 -X GET \"" + url + "\" -H \"Accept: application/json\" -H \"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\" 2>/dev/null";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            cout << "[DEBUG] Failed to execute curl command" << endl;
            return json{{"success", false}, {"error", "Failed to execute curl command"}};
        }
        
        // 读取curl输出
        string response_body;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            response_body += buffer;
        }
        
        int status = pclose(pipe);
        
        if (status != 0) {
            cout << "[DEBUG] Curl command failed with status: " << status << endl;
            return json{{"success", false}, {"error", "Network request failed"}};
        }
        
        // 检查响应体
        if (response_body.empty()) {
            cout << "[DEBUG] Empty response body" << endl;
            return json{{"success", false}, {"error", "Empty response from API"}};
        }
        
        cout << "[DEBUG] Response body length: " << response_body.length() << endl;
        if (response_body.length() > 500) {
            cout << "[DEBUG] Response body (first 500 chars): " << response_body.substr(0, 500) << endl;
        } else {
            cout << "[DEBUG] Response body: " << response_body << endl;
        }
        
        // 解析JSON响应
        json api_response;
        try {
            api_response = json::parse(response_body);
            cout << "[DEBUG] JSON parsed successfully" << endl;
        } catch (const json::parse_error& e) {
            cout << "[DEBUG] JSON parse error: " << e.what() << endl;
            return json{{"success", false}, {"error", "Invalid JSON response from API"}};
        }
        
        // 有道词典API返回对象格式
        if (api_response.is_object()) {
            cout << "[DEBUG] Converting API response..." << endl;
            return convert_iciba_response(word, api_response);
        } else if (api_response.is_array() && !api_response.empty()) {
            // 备用：如果是其他API的数组格式
            cout << "[DEBUG] Converting array format response..." << endl;
            return convert_free_dictionary_response(word, api_response[0]);
        }
        
        cout << "[DEBUG] Unexpected API response format, using local dictionary" << endl;
        // 如果API失败，直接返回失败让主函数使用本地词典
        return json{{"success", false}, {"error", "API format error"}};
        
    } catch (const exception& e) {
        cout << "[DEBUG] Exception in query_iciba_api: " << e.what() << endl;
        // API调用失败，返回失败状态让系统使用备用方案
        return json{{"success", false}, {"error", string("API Error: ") + e.what()}};
    }
}

json WordApp::convert_free_dictionary_response(const string& word, const json& api_data) {
    try {
        json result = {
            {"success", true},
            {"word", word},
            {"data", {
                {"word", word},
                {"phonetics", json::array()},
                {"meanings", json::array()},
                {"chinese_translation", ""},
                {"source", "Free Dictionary API"}
            }}
        };
        
        auto& data = result["data"];
        
        // 解析音标
        if (api_data.contains("phonetics") && api_data["phonetics"].is_array()) {
            for (const auto& phonetic : api_data["phonetics"]) {
                if (phonetic.contains("text") && !phonetic["text"].get<string>().empty()) {
                    data["phonetics"].push_back({
                        {"text", phonetic["text"].get<string>()},
                        {"audio", phonetic.contains("audio") ? phonetic["audio"].get<string>() : ""}
                    });
                }
            }
        }
        
        // 解析词义
        vector<string> all_definitions;
        if (api_data.contains("meanings") && api_data["meanings"].is_array()) {
            for (const auto& meaning : api_data["meanings"]) {
                json meaning_obj = {
                    {"partOfSpeech", meaning.contains("partOfSpeech") ? meaning["partOfSpeech"].get<string>() : ""},
                    {"definitions", json::array()}
                };
                
                if (meaning.contains("definitions") && meaning["definitions"].is_array()) {
                    for (const auto& def : meaning["definitions"]) {
                        json def_obj = {
                            {"definition", def.contains("definition") ? def["definition"].get<string>() : ""},
                            {"example", def.contains("example") ? def["example"].get<string>() : ""},
                            {"synonyms", def.contains("synonyms") ? def["synonyms"] : json::array()}
                        };
                        meaning_obj["definitions"].push_back(def_obj);
                        
                        // 收集定义用于简单的中文翻译提示
                        if (def.contains("definition")) {
                            all_definitions.push_back(def["definition"].get<string>());
                        }
                    }
                }
                
                if (!meaning_obj["definitions"].empty()) {
                    data["meanings"].push_back(meaning_obj);
                }
            }
        }
        
        // 添加简单的中文翻译
        data["chinese_translation"] = get_chinese_translation(word, all_definitions);
        
        // 如果没有中文翻译，至少显示第一个英文定义
        if (data["chinese_translation"].get<string>().empty() && !all_definitions.empty()) {
            data["chinese_translation"] = all_definitions[0];
        }
        
        return result;
        
    } catch (const exception& e) {
        return json{
            {"success", false}, 
            {"error", string("解析词典响应时出错: ") + e.what()}
        };
    }
}

json WordApp::convert_iciba_response(const string& word, const json& iciba_data) {
    try {
        json result = {
            {"success", true},
            {"word", word},
            {"data", {
                {"word", word},
                {"phonetics", json::array()},
                {"meanings", json::array()},
                {"chinese_translation", ""},
                {"source", "金山词霸 iciba.com"}
            }}
        };
        
        auto& data = result["data"];
        
        // 解析音标和词义
        if (iciba_data.contains("symbols") && iciba_data["symbols"].is_array() && !iciba_data["symbols"].empty()) {
            auto& symbol = iciba_data["symbols"][0];
            
            // 解析音标
            if (symbol.contains("ph_en") && !symbol["ph_en"].get<string>().empty()) {
                data["phonetics"].push_back({
                    {"text", "英 [" + symbol["ph_en"].get<string>() + "]"},
                    {"audio", symbol.contains("ph_en_mp3") ? symbol["ph_en_mp3"].get<string>() : ""}
                });
            }
            
            if (symbol.contains("ph_am") && !symbol["ph_am"].get<string>().empty()) {
                data["phonetics"].push_back({
                    {"text", "美 [" + symbol["ph_am"].get<string>() + "]"},
                    {"audio", symbol.contains("ph_am_mp3") ? symbol["ph_am_mp3"].get<string>() : ""}
                });
            }
            
            // 解析词义
            if (symbol.contains("parts") && symbol["parts"].is_array()) {
                vector<string> all_meanings; // 收集所有释义用于中文翻译
                
                for (const auto& part : symbol["parts"]) {
                    json meaning = {
                        {"partOfSpeech", part.contains("part") ? part["part"].get<string>() : ""},
                        {"definitions", json::array()}
                    };
                    
                    if (part.contains("means") && part["means"].is_array()) {
                        for (const auto& mean : part["means"]) {
                            if (!mean.get<string>().empty()) {
                            meaning["definitions"].push_back({
                                {"definition", mean.get<string>()},
                                    {"example", ""},
                                    {"synonyms", json::array()}
                            });
                                all_meanings.push_back(mean.get<string>());
                        }
                        }
                    }
                    
                    // 只添加有定义的词性
                    if (!meaning["definitions"].empty()) {
                    data["meanings"].push_back(meaning);
                    }
                }
                
                // 设置中文翻译为所有释义的组合
                if (!all_meanings.empty()) {
                    string combined_translation;
                    for (size_t i = 0; i < all_meanings.size() && i < 3; ++i) { // 最多取前3个
                        if (i > 0) combined_translation += "; ";
                        combined_translation += all_meanings[i];
                    }
                    data["chinese_translation"] = combined_translation;
                }
            }
        }
        
        // 如果没有通过symbols找到数据，尝试其他字段
        if (data["meanings"].empty()) {
            // 检查是否有直接的翻译信息
            if (iciba_data.contains("exchange") && iciba_data["exchange"].is_object()) {
                // 可能包含词形变化等信息
            }
            
            // 如果仍然没有找到信息，返回基本信息
        if (data["chinese_translation"].get<string>().empty()) {
                data["chinese_translation"] = "未找到详细释义，建议使用其他词典查询";
                data["meanings"].push_back({
                    {"partOfSpeech", ""},
                    {"definitions", json::array({
                        {{"definition", "词典中暂无该词的详细信息"}}
                    })}
                });
            }
        }
        
        return result;
        
    } catch (const exception& e) {
        return json{
            {"success", false}, 
            {"error", string("解析金山词霸响应时出错: ") + e.what()}
        };
    }
}

// ===== 用户数据管理方法实现 =====

json WordApp::reset_user_progress(bool reset_mistakes, bool reset_position) {
    return data_manager.reset_progress(reset_mistakes, reset_position);
}


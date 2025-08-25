#include "FileUtils.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool FileUtils::file_exists(const string& filepath) {
    return fs::exists(filepath);
}

string FileUtils::read_text_file(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    string content((istreambuf_iterator<char>(file)), 
                   istreambuf_iterator<char>());
    file.close();
    return content;
}

bool FileUtils::write_text_file(const string& filepath, const string& content) {
    ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

string FileUtils::get_file_extension(const string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == string::npos) {
        return "";
    }
    return filename.substr(dot_pos + 1);
}

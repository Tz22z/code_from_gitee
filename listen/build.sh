#!/bin/bash

echo "=== 单词朗读器构建脚本 ==="

# 检查必要工具
echo "检查构建环境..."

# 检查g++
if ! command -v g++ &> /dev/null; then
    echo "错误: 未找到 g++ 编译器"
    echo "请安装: sudo apt-get install build-essential"
    exit 1
fi

# 检查Python3
if ! command -v python3 &> /dev/null; then
    echo "错误: 未找到 python3"
    echo "请安装: sudo apt-get install python3"
    exit 1
fi

echo "✓ 构建环境检查完成"

# 编译C++程序
echo "编译C++程序..."
g++ -std=c++11 -pthread -o word_reader word_reader.cpp

if [ $? -eq 0 ]; then
    echo "✓ C++程序编译成功"
    chmod +x word_reader
else
    echo "✗ C++程序编译失败"
    exit 1
fi

# 设置Python服务器权限
chmod +x server.py

# 检查TTS引擎
echo "检查TTS引擎..."
if command -v espeak &> /dev/null; then
    echo "✓ 检测到 espeak"
elif command -v festival &> /dev/null; then
    echo "✓ 检测到 festival"
else
    echo "⚠ 警告: 未检测到TTS引擎"
    echo "建议安装 espeak: sudo apt-get install espeak"
    echo "或安装 festival: sudo apt-get install festival"
fi

echo ""
echo "=== 构建完成 ==="
echo "运行方式："
echo "1. 启动服务器: ./run.sh"
echo "2. 访问网页: http://localhost:8080"
echo "3. 或命令行使用: ./word_reader \"hello world apple\""

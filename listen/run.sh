#!/bin/bash

echo "=== 启动单词朗读器 ==="

# 检查程序是否已编译
if [ ! -f "word_reader" ]; then
    echo "C++程序未编译，正在编译..."
    ./build.sh
    if [ $? -ne 0 ]; then
        echo "编译失败，请检查错误信息"
        exit 1
    fi
fi

# 启动服务器
echo "启动HTTP服务器..."
echo "网页地址: http://localhost:8080"
echo "按 Ctrl+C 停止服务器"
echo ""

python3 server.py

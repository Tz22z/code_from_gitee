#!/bin/bash

echo "=== 安装单词朗读器依赖 ==="

# 检测操作系统
if [ -f /etc/debian_version ]; then
    # Debian/Ubuntu
    echo "检测到 Debian/Ubuntu 系统"
    
    echo "更新包列表..."
    sudo apt-get update
    
    echo "安装构建工具..."
    sudo apt-get install -y build-essential
    
    echo "安装Python3..."
    sudo apt-get install -y python3
    
    echo "安装TTS引擎..."
    sudo apt-get install -y espeak espeak-data
    
    echo "✓ 依赖安装完成"
    
elif [ -f /etc/redhat-release ]; then
    # CentOS/RHEL/Fedora
    echo "检测到 RedHat 系列系统"
    
    if command -v dnf &> /dev/null; then
        # Fedora
        echo "安装构建工具..."
        sudo dnf install -y gcc-c++ make
        
        echo "安装Python3..."
        sudo dnf install -y python3
        
        echo "安装TTS引擎..."
        sudo dnf install -y espeak espeak-devel
        
    elif command -v yum &> /dev/null; then
        # CentOS/RHEL
        echo "安装构建工具..."
        sudo yum install -y gcc-c++ make
        
        echo "安装Python3..."
        sudo yum install -y python3
        
        echo "安装TTS引擎..."
        sudo yum install -y espeak espeak-devel
    fi
    
    echo "✓ 依赖安装完成"
    
elif [ -f /etc/arch-release ]; then
    # Arch Linux
    echo "检测到 Arch Linux 系统"
    
    echo "安装构建工具..."
    sudo pacman -S --noconfirm base-devel
    
    echo "安装Python3..."
    sudo pacman -S --noconfirm python
    
    echo "安装TTS引擎..."
    sudo pacman -S --noconfirm espeak-ng
    
    echo "✓ 依赖安装完成"
    
else
    echo "⚠ 未识别的操作系统"
    echo "请手动安装以下依赖："
    echo "- C++ 编译器 (g++)"
    echo "- Python 3"
    echo "- TTS引擎 (espeak 或 festival)"
fi

echo ""
echo "=== 依赖安装完成 ==="
echo "接下来运行: ./build.sh"

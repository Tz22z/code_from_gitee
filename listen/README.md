# 🔊 单词朗读器

一个基于C++后端和网页前端的单词朗读工具，支持依次朗读复制的单词列表。

## ✨ 功能特点

- 📝 **智能单词分割**: 自动分割文本中的单词，去除标点符号
- 🗣️ **依次朗读**: 按顺序朗读每个单词，支持暂停和继续
- ⚡ **双重TTS支持**: 
  - 网页端：使用浏览器内置的Web Speech API
  - 后端：使用系统TTS引擎 (espeak/festival)
- 🎛️ **可调参数**: 语速调节、单词间暂停时间调节
- 📊 **实时反馈**: 进度条、当前朗读单词高亮显示
- 🎨 **现代化界面**: 响应式设计，支持移动设备
- ⌨️ **键盘快捷键**: Ctrl+Enter开始，Ctrl+Space暂停，Esc停止

## 🚀 快速开始

### 1. 安装依赖

```bash
# 自动安装依赖（推荐）
chmod +x install_deps.sh
./install_deps.sh

# 或手动安装
# Ubuntu/Debian:
sudo apt-get install build-essential python3 espeak

# CentOS/RHEL:
sudo yum install gcc-c++ python3 espeak

# Arch Linux:
sudo pacman -S base-devel python espeak-ng
```

### 2. 构建程序

```bash
chmod +x build.sh
./build.sh
```

### 3. 启动服务器

```bash
chmod +x run.sh
./run.sh
```

### 4. 使用网页界面

打开浏览器访问: http://localhost:8080

## 📖 使用方法

### 网页界面使用

1. **粘贴单词**: 在文本框中粘贴或输入要朗读的单词
2. **调节参数**: 
   - 语速：80-250 words per minute
   - 暂停时间：200-2000 毫秒
3. **开始朗读**: 点击"开始朗读"按钮
4. **控制播放**: 使用暂停/继续、停止按钮控制播放

### 命令行使用

```bash
# 朗读单个或多个单词
./word_reader "hello world apple banana"

# 交互模式
./word_reader
> hello world
> quit

# 启动HTTP服务器模式
./word_reader --server
```

### 键盘快捷键

- `Ctrl + Enter`: 开始朗读
- `Ctrl + Space`: 暂停/继续
- `Esc`: 停止朗读

## 🔧 技术架构

```
┌─────────────────┐    HTTP API    ┌─────────────────┐
│   网页前端      │ ◄─────────────► │   Python服务器   │
│  (HTML/JS/CSS)  │                │   (server.py)   │
└─────────────────┘                └─────────────────┘
                                           │
                                           ▼
                                   ┌─────────────────┐
                                   │   C++后端程序    │
                                   │ (word_reader)   │
                                   └─────────────────┘
                                           │
                                           ▼
                                   ┌─────────────────┐
                                   │   系统TTS引擎    │
                                   │ (espeak/festival)│
                                   └─────────────────┘
```

## 📁 项目结构

```
/opt/listen/
├── word_reader.cpp      # C++后端程序源码
├── word_reader          # 编译后的可执行文件
├── server.py           # Python HTTP服务器
├── index.html          # 网页前端界面
├── build.sh           # 构建脚本
├── run.sh             # 启动脚本
├── install_deps.sh    # 依赖安装脚本
└── README.md          # 使用说明
```

## 🛠️ 开发说明

### C++后端 (word_reader.cpp)

- 使用系统TTS引擎进行语音合成
- 支持单词分割和清理
- 提供命令行和HTTP服务器模式
- 多线程支持，避免阻塞

### Python服务器 (server.py)

- 提供HTTP API接口
- 处理CORS跨域请求
- 后台线程执行朗读任务
- 静态文件服务

### 网页前端 (index.html)

- 现代化响应式设计
- Web Speech API支持
- 实时进度显示
- 键盘快捷键支持

## 🔍 故障排除

### TTS引擎问题

如果朗读没有声音：

1. 检查TTS引擎安装：
   ```bash
   espeak "test"  # 应该能听到声音
   ```

2. 安装音频驱动：
   ```bash
   sudo apt-get install alsa-utils pulseaudio
   ```

3. 检查音量设置：
   ```bash
   alsamixer  # 调节音量
   ```

### 编译问题

如果编译失败：

1. 检查g++版本：
   ```bash
   g++ --version  # 需要支持C++11
   ```

2. 安装完整的构建工具：
   ```bash
   sudo apt-get install build-essential
   ```

### 网络问题

如果无法访问网页：

1. 检查端口占用：
   ```bash
   netstat -tlnp | grep 8080
   ```

2. 更换端口（修改server.py中的PORT变量）

3. 检查防火墙设置

## 📝 许可证

MIT License - 自由使用和修改

## 🤝 贡献

欢迎提交Issue和Pull Request！

## 📞 联系方式

如有问题请创建GitHub Issue或联系开发者。

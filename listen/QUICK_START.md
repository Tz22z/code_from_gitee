# 🚀 快速启动指南

## 当前状态
✅ C++程序已编译完成  
✅ HTTP服务器已启动  
✅ 网页界面可用  
⚠️ 系统TTS引擎未安装（但网页版可正常使用）

## 立即使用

### 方法1: 网页版（推荐）
1. 打开浏览器访问: **http://localhost:8080**
2. 在文本框中粘贴要朗读的单词
3. 点击"开始朗读"
4. 浏览器会使用内置的Web Speech API进行朗读

### 方法2: 命令行版
```bash
# 测试单词分割功能（会显示但不会发声）
./word_reader "apple banana cherry dog elephant"
```

## 功能说明

### 网页版功能
- ✅ 智能单词分割
- ✅ 依次朗读（使用浏览器TTS）
- ✅ 语速调节
- ✅ 暂停/继续/停止
- ✅ 进度显示
- ✅ 单词高亮

### 使用示例
1. 复制这段文本到网页：`hello world apple banana cherry dog elephant fish`
2. 调节语速和暂停时间
3. 点击开始朗读
4. 程序会依次朗读每个单词，单词间有暂停

## 浏览器兼容性
- ✅ Chrome/Edge (推荐)
- ✅ Firefox  
- ✅ Safari
- ⚠️ 需要现代浏览器支持Web Speech API

## 如果需要系统TTS支持
如果以后想要安装系统TTS引擎，可以尝试：
```bash
# 更换软件源后安装
sudo apt-get install espeak
# 或者
sudo apt-get install festival
```

## 停止服务器
按 `Ctrl+C` 停止HTTP服务器

---
**当前服务器地址**: http://localhost:8080  
**端口**: 8080  
**状态**: 运行中 🟢


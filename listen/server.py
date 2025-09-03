#!/usr/bin/env python3
"""
简单的HTTP服务器，用于连接C++后端和网页前端
"""

import http.server
import socketserver
import json
import subprocess
import urllib.parse
import os
import threading
import time

class WordReaderHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="/opt/listen", **kwargs)

    def do_POST(self):
        """处理POST请求"""
        if self.path == '/speak':
            self.handle_speak_request()
        elif self.path == '/speak_words':
            self.handle_speak_words_request()
        else:
            self.send_error(404, "Not Found")

    def handle_speak_request(self):
        """处理单个单词朗读请求"""
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            
            word = data.get('word', '').strip()
            if not word:
                self.send_error(400, "No word provided")
                return
            
            # 调用C++程序朗读单词
            result = subprocess.run(
                ['/opt/listen/word_reader', word],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                response = {"status": "success", "message": f"Spoke word: {word}"}
            else:
                response = {"status": "error", "message": f"Failed to speak word: {word}"}
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Internal server error: {str(e)}")

    def handle_speak_words_request(self):
        """处理多个单词依次朗读请求"""
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            
            text = data.get('text', '').strip()
            speed = data.get('speed', 150)
            pause = data.get('pause', 800)
            
            if not text:
                self.send_error(400, "No text provided")
                return
            
            # 在后台线程中执行朗读，避免阻塞HTTP响应
            def speak_in_background():
                try:
                    # 调用C++程序朗读文本
                    subprocess.run(
                        ['/opt/listen/word_reader', text],
                        timeout=300  # 5分钟超时
                    )
                except subprocess.TimeoutExpired:
                    print("Speaking timeout")
                except Exception as e:
                    print(f"Speaking error: {e}")
            
            threading.Thread(target=speak_in_background, daemon=True).start()
            
            response = {"status": "success", "message": "Started speaking"}
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Internal server error: {str(e)}")

    def do_OPTIONS(self):
        """处理CORS预检请求"""
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def end_headers(self):
        # 添加CORS头
        self.send_header('Access-Control-Allow-Origin', '*')
        super().end_headers()

def main():
    PORT = 8088
    
    print("=== 单词朗读器服务器 ===")
    print(f"启动HTTP服务器，端口: {PORT}")
    print(f"网页地址: http://localhost:{PORT}")
    print("按 Ctrl+C 停止服务器")
    
    # 检查C++程序是否存在
    cpp_program = "/opt/listen/word_reader"

    
    if not os.path.exists(cpp_program):
        print(f"警告: C++程序 {cpp_program} 不存在")
        print("请先编译C++程序: g++ -o word_reader word_reader.cpp")
    
    # 检查TTS引擎
    tts_available = False
    if subprocess.run("which espeak", shell=True, capture_output=True).returncode == 0:
        print("✓ 检测到 espeak TTS引擎")
        tts_available = True
    elif subprocess.run("which festival", shell=True, capture_output=True).returncode == 0:
        print("✓ 检测到 festival TTS引擎")
        tts_available = True
    else:
        print("⚠ 未检测到TTS引擎，请安装 espeak 或 festival:")
        print("  Ubuntu/Debian: sudo apt-get install espeak")
        print("  CentOS/RHEL: sudo yum install espeak")
    
    try:
        with socketserver.TCPServer(("", PORT), WordReaderHandler) as httpd:
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n服务器已停止")
    except Exception as e:
        print(f"服务器错误: {e}")

if __name__ == "__main__":
    main()

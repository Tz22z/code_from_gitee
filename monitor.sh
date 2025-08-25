#!/bin/bash

# Word Learning Application 监控脚本

check_service() {
    if systemctl is-active --quiet word-learning; then
        echo "✅ 服务运行正常"
    else
        echo "❌ 服务异常，尝试重启..."
        systemctl restart word-learning
        sleep 5
        if systemctl is-active --quiet word-learning; then
            echo "✅ 服务重启成功"
        else
            echo "❌ 服务重启失败"
        fi
    fi
}

check_port() {
    if netstat -tulpn | grep -q ":8080"; then
        echo "✅ 端口 8080 正常监听"
    else
        echo "❌ 端口 8080 未监听"
    fi
}

check_nginx() {
    if systemctl is-active --quiet nginx; then
        echo "✅ Nginx运行正常"
    else
        echo "❌ Nginx异常"
    fi
}

check_disk_space() {
    local usage=$(df /opt/word-learning | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ $usage -gt 80 ]; then
        echo "⚠️  磁盘使用率: ${usage}% (警告: >80%)"
    else
        echo "✅ 磁盘使用率: ${usage}%"
    fi
}

echo "==============================================="
echo "Word Learning Application 健康检查"
echo "时间: $(date)"
echo "==============================================="

check_service
check_port
check_nginx
check_disk_space

echo "==============================================="

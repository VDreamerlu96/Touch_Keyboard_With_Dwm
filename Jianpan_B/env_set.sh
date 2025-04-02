#!/bin/bash

# 检查是否以 root 用户运行
if [ "$(id -u)" -ne 0 ]; then
    echo "请以 root 用户或使用 sudo 运行此脚本"
    exit 1
fi

# 加载 uinput 模块
echo "加载 uinput 模块..."
modprobe uinput

# 创建 udev 规则
echo "创建 udev 规则..."
echo 'KERNEL=="uinput", GROUP="uinput", MODE="0660", OPTIONS+="static_node=uinput"' > /etc/udev/rules.d/99-uinput.rules

# 创建 uinput 组（如果它不存在）
if ! getent group uinput > /dev/null; then
    echo "创建 uinput 组..."
    groupadd uinput
else
    echo "uinput 组已经存在"
fi

# 将用户添加到 uinput 组
read -p "请输入要添加到 uinput 组的用户名: " username
usermod -aG uinput "$username"

# 重新加载 udev 规则并应用
echo "重新加载 udev 规则..."
udevadm control --reload-rules
udevadm trigger

echo "配置完成！请注销并重新登录以使更改生效。"

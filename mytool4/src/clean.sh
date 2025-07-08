#!/bin/bash
# 清空 RuntimeAndError.json 文件内容

TARGET_FILE="/home/zjn/mytool4/src/RuntimeAndError.json"

# 检查文件是否存在
if [ -f "$TARGET_FILE" ]; then
    # 清空文件内容
    > "$TARGET_FILE"
    echo "[SUCCESS] 已清空文件内容：$TARGET_FILE"
else
    echo "[ERROR] 文件未找到：$TARGET_FILE"
    exit 1
fi
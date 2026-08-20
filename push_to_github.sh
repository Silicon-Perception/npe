#!/bin/bash
# NPP SDK - 推送到GitHub脚本
# 使用方法: bash push_to_github.sh <你的GitHub令牌>

set -e

if [ -z "$1" ]; then
    echo "错误: 请提供GitHub Personal Access Token"
    echo "用法: bash push_to_github.sh ghp_xxxxxxxxxxxx"
    exit 1
fi

TOKEN="$1"
REPO_URL="https://Mofinest:${TOKEN}@github.com/Silicon-Perception/npe.git"

echo "=== NPP SDK GitHub推送脚本 ==="
echo ""

# 进入脚本所在目录
cd "$(dirname "$0")"

# 检查git是否已初始化
if [ ! -d .git ]; then
    echo "[1/5] 初始化Git仓库..."
    git init
    git add .
    git commit -m "NPP SDK v2.0 - Initial Release"
else
    echo "[1/5] Git仓库已存在 ✓"
fi

# 添加远程仓库
echo "[2/5] 配置GitHub远程仓库..."
if git remote | grep -q "^github$"; then
    git remote set-url github "$REPO_URL"
else
    git remote add github "$REPO_URL"
fi
echo "  远程仓库: https://github.com/Silicon-Perception/npe"

# 推送
echo "[3/5] 推送到GitHub..."
git push -u github main

# 清理令牌（安全）
echo "[4/5] 清理令牌（安全考虑）..."
git remote set-url github https://github.com/Silicon-Perception/npe.git

# 验证
echo "[5/5] 验证..."
git remote -v
echo ""
echo "=== 推送完成！ ==="
echo ""
echo "下一步："
echo "1. 访问 https://github.com/Silicon-Perception/npe"
echo "2. 进入 Settings → Secrets and variables → Actions"
echo "3. 添加 Secret:"
echo "   Name:  GITEE_TOKEN"
echo "   Value: f18023c083064a2f934a49d2a8328a09"
echo ""
echo "配置完成后，GitHub将每6小时自动从Gitee同步！"

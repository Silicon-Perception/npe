# NPP SDK 贡献指南

> 感谢你对NPP SDK的关注！我们欢迎社区开发者参与开源部分的建设。

---

## 🤝 贡献范围

### ✅ 我们接受贡献

| 目录 | 说明 | 示例 |
|------|------|------|
| `include/` | 头文件改进 | 修复注释、添加文档 |
| `tests/` | 测试用例 | 新增单元测试、集成测试 |
| `examples/` | 示例代码 | 新场景Demo、最佳实践 |
| `tools/` | 迁移工具 | 支持更多协议转换 |
| `docs/` | 文档改进 | 教程、FAQ、翻译 |

### 🔒 不接受贡献

| 目录 | 原因 |
|------|------|
| `src/` | 核心闭源实现，仅维护者可修改 |
| `lib/` | 预编译二进制，自动生成 |

---

## 📝 贡献流程

### 1. 准备工作

```bash
# Fork 仓库
# 克隆你的Fork
git clone https://github.com/YOUR_USERNAME/npp-sdk.git
cd npp-sdk

# 添加上游仓库
git remote add upstream https://gitee.com/perception-engine/npp-sdk.git

# 创建分支
git checkout -b feature/your-feature-name
```

### 2. 开发规范

#### C代码规范
- 遵循C99标准
- 使用4空格缩进
- 函数命名：`npp_xxx_xxx()`
- 宏定义：`NPP_XXX_XXX`
- 注释使用英文或中文均可

#### 提交规范
```
feat: 添加新特性
fix: 修复Bug
docs: 文档更新
test: 测试用例
refactor: 代码重构
```

### 3. 提交PR

```bash
# 提交更改
git add .
git commit -m "feat: 添加XXX功能"

# 推送到你的Fork
git push origin feature/your-feature-name

# 在GitHub创建Pull Request
```

### 4. PR要求

- [ ] 描述清晰（做了什么、为什么）
- [ ] 仅修改开源部分
- [ ] 所有测试通过
- [ ] 代码风格一致
- [ ] 已更新相关文档

---

## 🐛 Bug报告

### 开源部分Bug

通过 [Gitee Issues](https://gitee.com/perception-engine/npp-sdk/issues) 报告：

```markdown
**描述**: 简述问题

**复现步骤**:
1. 步骤1
2. 步骤2

**期望行为**: 应该怎样

**实际行为**: 实际怎样

**环境**:
- OS: macOS 14 / Ubuntu 22.04
- 编译器: GCC 13 / Clang 15
- SDK版本: v1.4.0
```

### 闭源部分Bug

发送邮件至：alphache@163.com

---

## 💡 功能请求

欢迎通过GitHub Issues提交功能请求：

```markdown
**功能描述**: 简述功能

**使用场景**: 解释为什么需要

**可能的实现**: 可选，提供思路
```

---

## 📚 文档贡献

### 文档类型

- **教程**: 新手指南、最佳实践
- **FAQ**: 常见问题解答
- **翻译**: 中英文互译
- **API文档**: 函数说明、参数解释

### 文档规范

- Markdown格式
- 代码示例可运行
- 截图/GIF辅助说明

---

## 🧪 测试指南

### 运行测试

```bash
mkdir build && cd build
cmake ..
make
make test
```

### 添加测试

在 `tests/` 目录下添加测试文件：

```c
// tests/test_your_feature.c
#include <assert.h>
#include "npp.h"

void test_your_feature() {
    // 测试代码
    assert(1 + 1 == 2);
}

int main() {
    test_your_feature();
    return 0;
}
```

---

## 🏆 贡献者荣誉

所有贡献者将被列入CONTRIBUTORS.md（如果存在）。

---

## 📜 行为准则

- 尊重他人
-  constructive feedback
- 专注技术讨论
- 遵守开源协议

---

*感谢你的贡献！*

*最后更新: 2026-08-20*

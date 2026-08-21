# NPP SDK 版本管理策略

## 版本号格式

```
MAJOR.MINOR.YYMMDD.N
```

| 部分 | 含义 | 示例 |
|------|------|------|
| MAJOR | 主版本号，重大架构变更时递增 | 1 |
| MINOR | 次版本号，功能更新时递增 | 0 |
| YYMMDD | 发布日期，6位日期 | 260821 |
| N | 当日迭代编号，从1开始 | 1 |

## 示例

- `1.0.260821.1` — 2026年8月21日第一次发布
- `1.0.260821.2` — 2026年8月21日第二次修复
- `1.0.260822.1` — 2026年8月22日第一次发布
- `1.1.260825.1` — 2026年8月25日功能更新

## 版本号递增规则

### MAJOR（主版本号）
- 不兼容的 API 变更
- 协议格式重大变更
- 架构重构

### MINOR（次版本号）
- 新增功能（向后兼容）
- 性能优化
- 新平台支持

### YYMMDD（日期）
- 每次发布时使用当天日期
- 格式：YYMMDD（6位）

### N（迭代编号）
- 同一天内的第 N 次发布
- 从 1 开始，每次发布 +1
- 跨天重置为 1

## 版本号文件

仓库根目录的 `VERSION` 文件是版本号的单一来源（Single Source of Truth）。

所有其他文件（npp.h、CMakeLists.txt、README badge 等）必须与此文件保持一致。

## 发布流程

### 标准发布检查清单

1. 更新代码和功能
2. 更新 `VERSION` 文件
3. 同步更新以下文件：
   - `include/npp.h` — NPP_VERSION_STRING
   - `CMakeLists.txt` — project(VERSION)
   - `lib/README.md` — Version 章节
   - `README.md` / `README_CN.md` — Version badge
4. 更新 `CHANGELOG.md`
5. **编译混淆版本**（见下方混淆流程）
6. 提交代码并创建 Git tag

### 代码混淆流程

**每次发布预编译库时必须执行混淆编译。**

#### 混淆级别

| 级别 | 标志 | 说明 |
|------|------|------|
| 基础 | `-mllvm -flatten` | 控制流平坦化 |
| 中级 | `-mllvm -bcf` | 虚假控制流 |
| 高级 | `-mllvm -sub` | 指令替换 |
| 字符串 | `-mllvm -sobf` | 字符串加密 |

#### 编译命令

```bash
# 使用 Clang + O-LLVM 编译混淆版本
mkdir build && cd build
cmake .. \
    -DCMAKE_C_COMPILER=clang \
    -DNPP_OBFUSCATE=ON \
    -DCMAKE_BUILD_TYPE=Release
cmake --build .

# 验证混淆效果
nm libnpp.a | head -20  # 检查符号是否被混淆
```

#### 混淆验证清单

- [ ] 控制流平坦化已启用
- [ ] 虚假控制流已启用
- [ ] 指令替换已启用
- [ ] 字符串加密已启用（如可用）
- [ ] 符号表已剥离（`strip -S`）
- [ ] 调试信息已移除（`-g0`）
- [ ] 混淆后库文件大小正常
- [ ] 混淆后功能测试通过

#### 注意事项

1. **O-LLVM 安装**：混淆需要安装 O-LLVM 编译器
   ```bash
   # 安装 O-LLVM（示例）
   git clone https://github.com/obfuscator-llvm/obfuscator.git
   cd obfuscator && mkdir build && cd build
   cmake .. && make -j$(nproc)
   ```

2. **备份原始库**：混淆前保留原始库文件备份

3. **测试验证**：混淆后必须运行完整测试套件

4. **版本记录**：记录每次混淆使用的参数和编译器版本

## Git Tag 格式

```bash
git tag -a v1.0.260821.1 -m "Release 1.0.260821.1"
```

## 协议版本 vs SDK 版本

**注意**：NPP 协议版本（wire format version）与 SDK 版本是独立的。

- 协议版本：表示网络协议的兼容性
- SDK 版本：表示软件构建的版本

两者可以独立演进。

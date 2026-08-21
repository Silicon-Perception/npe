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

1. 更新代码和功能
2. 更新 `VERSION` 文件
3. 同步更新以下文件：
   - `include/npp.h` — NPP_VERSION_STRING
   - `CMakeLists.txt` — project(VERSION)
   - `lib/README.md` — Version 章节
   - `README.md` / `README_CN.md` — Version badge
4. 更新 `CHANGELOG.md`
5. 提交代码并创建 Git tag

## Git Tag 格式

```bash
git tag -a v1.0.260821.1 -m "Release 1.0.260821.1"
```

## 协议版本 vs SDK 版本

**注意**：NPP 协议版本（wire format version）与 SDK 版本是独立的。

- 协议版本：表示网络协议的兼容性
- SDK 版本：表示软件构建的版本

两者可以独立演进。

# lib/ 目录说明

本目录包含NPP SDK闭源部分的预编译二进制库。

## 获取方式

### 方式1：从GitHub Release下载（推荐）

访问：https://gitee.com/Silicon-Perception/npp-sdk/releases

下载对应版本的附件包，解压后覆盖本目录。

### 方式2：运行下载脚本

```bash
./scripts/download_libs.sh
```

## 文件说明

| 文件 | 说明 |
|------|------|
| `libnpp.a` | Linux/macOS静态库 |
| `libnpp.so` | Linux动态库 |
| `libnpp.dylib` | macOS动态库 |
| `libnpp.dll` | Windows动态库 |
| `checksums.sha256` | SHA256校验和 |

## 版本匹配

确保下载的二进制版本与 `include/npp.h` 中定义的版本号一致：

```c
#define NPP_VERSION_MAJOR 1
#define NPP_VERSION_MINOR 4
#define NPP_VERSION_PATCH 0
```

## 链接方式

### CMake
```cmake
target_link_libraries(your_app ${NPP_SDK_DIR}/lib/libnpp.a)
```

### Makefile
```make
LDFLAGS += -L$(NPP_SDK_DIR)/lib -lnpp
```

## 许可证

本目录下的预编译二进制受商业授权保护。
个人/学术使用免费，商业使用需联系 alphache@163.com 获取授权。

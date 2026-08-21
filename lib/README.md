# lib/ Pre-compiled Libraries

This directory contains pre-compiled libraries for NPP SDK closed-source components.

## Available Libraries

| File | Platform | Type | Size |
|------|----------|------|------|
| `libnpp.a` | Linux/macOS | Static | 66K |
| `libnpe.a` | Linux/macOS | Static | 99K |
| `libnpe.4.1.0.dylib` | macOS | Dynamic | 75K |
| `libnpe_mcu.a` | MCU (Embedded) | Static | 7.3K |

## Windows Support

Pre-compiled Windows libraries (`.lib`/`.dll`) are not included in this release.

**To build on Windows:**

1. Install Visual Studio 2019+ with C++ desktop development workload
2. Open Developer Command Prompt for VS
3. Run:
   ```cmd
   mkdir build && cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release
   ```
4. Output: `build/Release/npp.lib`

**Requirements for Windows build:**
- MSVC 2019 or later
- Windows SDK 10.0.18362.0 or later

## Usage

### Linux/macOS - Static Library

```bash
gcc -o my_app my_app.c -I./include -L./lib -lnpp
```

### Windows - Static Library

```cmd
cl my_app.c /I./include /link /LIBPATH:./lib npp.lib
```

### CMake (Cross-Platform)

```cmake
target_link_libraries(your_app ${NPP_SDK_DIR}/lib/libnpp.a)
```

### Makefile

```make
LDFLAGS += -L$(NPP_SDK_DIR)/lib -lnpp
```

## Version

NPP SDK v1.0.260821.1

## License

Pre-compiled binaries are commercially licensed.
- Personal/Academic use: Free
- Commercial use: Contact alphache@163.com

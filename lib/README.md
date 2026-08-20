# lib/ Pre-compiled Libraries

This directory contains pre-compiled static libraries for NPP SDK closed-source components.

## Available Libraries

| File | Platform | Size |
|------|----------|------|
| `libnpp.a` | Linux/macOS Static | 66K |
| `libnpe.a` | Linux/macOS Static | 99K |
| `libnpe.4.1.0.dylib` | macOS Dynamic | 75K |
| `libnpe_mcu.a` | MCU (Embedded) | 7.3K |

## Usage

### Link Static Library

```bash
gcc -o my_app my_app.c -I./include -L./lib -lnpp
```

### CMake
```cmake
target_link_libraries(your_app ${NPP_SDK_DIR}/lib/libnpp.a)
```

### Makefile
```make
LDFLAGS += -L$(NPP_SDK_DIR)/lib -lnpp
```

## Version

NPP SDK v2.0.2

## License

Pre-compiled binaries are commercially licensed.
- Personal/Academic use: Free
- Commercial use: Contact alphache@163.com

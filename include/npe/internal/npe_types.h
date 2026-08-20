/**
 * NPE 4.0 — 通用类型定义
 */

#ifndef NPE_TYPES_H
#define NPE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPE_VERSION_MAJOR 4
#define NPE_VERSION_MINOR 0
#define NPE_VERSION_PATCH 0
#define NPE_VERSION_STRING "4.0.0"

#define NPE_MAX_WIDTH  1920
#define NPE_MAX_HEIGHT 1080
#define NPE_MAX_SIZE   (NPE_MAX_WIDTH * NPE_MAX_HEIGHT)

#define NPE_SYNC_WORD  0xAA55
#define NPE_FRAME_CONTROL  0x01
#define NPE_FRAME_DATA     0x02

typedef enum {
    NPE_PLATFORM_DESKTOP = 0,
    NPE_PLATFORM_MCU     = 1,
    NPE_PLATFORM_FPGA    = 2,
} NPEPlatform;

typedef enum {
    NPE_MODE_ZERO  = 0,
    NPE_MODE_FULL  = 1,
} NPEMode;

typedef enum {
    NPE_TARGET_QUALITY   = 0,
    NPE_TARGET_BALANCED  = 1,
    NPE_TARGET_BANDWIDTH = 2,
} NPETargetMode;

#ifdef __cplusplus
}
#endif

#endif

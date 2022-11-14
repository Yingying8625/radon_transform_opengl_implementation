/* Copyright:
 * ----------------------------------------------------------------------------
 * This confidential and proprietary software may be used only as authorized
 * by a licensing agreement from ARM Limited.
 *      (C) COPYRIGHT 2013, 2016, 2020-2021 ARM Limited, ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorized copies and
 * copies may only be made to the extent permitted by a licensing agreement
 * from ARM Limited.
 * ----------------------------------------------------------------------------
 */


/* Include the original header */
#include <khronos/EGL/eglext.h>

#ifndef EGL_ARM_surface_compression
#define EGL_ARM_surface_compression 1
#define EGL_ARM_surface_compression_fixed_rate_16 1
#define EGL_ARM_surface_compression_fixed_rate_24 1
#define EGL_ARM_surface_compression_fixed_rate_32 1
#define EGL_SURFACE_COMPRESSION_ARM 0x34B0
#define EGL_SURFACE_COMPRESSION_FIXED_RATE_NONE_ARM     0x34B1
#define EGL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_ARM  0x34B2
#define EGL_SURFACE_COMPRESSION_FIXED_RATE_16_ARM       0x34B4
#define EGL_SURFACE_COMPRESSION_FIXED_RATE_24_ARM       0x34B5
#define EGL_SURFACE_COMPRESSION_FIXED_RATE_32_ARM       0x34B6
#define EGL_SURFACE_COMPRESSION_CHROMA_ARM              0x34B7
#endif /* EGL_ARM_surface_compression */

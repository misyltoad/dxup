#pragma once

// GCC complains about the COM interfaces
// not having virtual destructors
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic ignored "-Wnonnull-compare" // Crysis work around.
#endif // __GNUC__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>

// GCC: -std options disable certain keywords
// https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html
#if defined(__WINE__) && !defined(typeof)
#define typeof __typeof
#endif

#include <d3d10_1.h>
#include <d3d11_1.h>

// This is not defined in the mingw headers
#ifndef D3D11_1_UAV_SLOT_COUNT
#define D3D11_1_UAV_SLOT_COUNT 64
#endif

#define D3D10_CREATE_DEVICE_DEBUG 1
#define D3D10_CREATE_DEVICE_DEBUGGABLE 8

#ifndef D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL
#define D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL 0xFFFFFFFF
#endif

#ifndef D3D11_KEEP_UNORDERED_ACCESS_VIEWS
#define D3D11_KEEP_UNORDERED_ACCESS_VIEWS 0xFFFFFFFF
#endif

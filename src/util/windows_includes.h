#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#pragma GCC diagnostic ignored "-Wswitch" // D3DFORMAT is not a complete enumeration. TODO: Eliminitate this warning.
#pragma GCC diagnostic ignored "-Wenum-compare" // D3DFORMAT is not a complete enumeration. TODO: Eliminitate this warning, #define D3DFORMAT?

#pragma GCC diagnostic ignored "-Wreorder" // TODO: fix orders.
#pragma GCC diagnostic ignored "-Wstrict-aliasing" // TODO: fix this (memcpy maybe?).
#endif // __GNUC__

#if defined(__WINE__) && !defined(typeof)
#define typeof __typeof
#endif
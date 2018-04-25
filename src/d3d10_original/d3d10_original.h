#pragma once

#include <d3d10_1.h>

#define D3D10_ORIGINAL_ARGS(returntype, name, ...)

#define D3D10_ORIGINAL_WRAP(returntype, name, ...) typedef returntype ( __stdcall * D3D10_ORIGINAL##name )(__VA_ARGS__);
#include "d3d10_original_interfaces.h"
#undef D3D10_ORIGINAL_WRAP

class D3D10OriginalInterface
{
public:
	D3D10OriginalInterface()
	{
		HMODULE d3d10OriginalModule = LoadLibraryA("d3d10_original.dll");

		if (d3d10OriginalModule)
		{
#define D3D10_ORIGINAL_WRAP(returntype, name, ...) name = D3D10_ORIGINAL ## name (GetProcAddress(d3d10OriginalModule, "D3D10" #name ));
#include "d3d10_original_interfaces.h"
#undef D3D10_ORIGINAL_WRAP
		}
	}

	static D3D10OriginalInterface& Get()
	{
		if (!s_d3d10OriginalInterface)
			s_d3d10OriginalInterface = new D3D10OriginalInterface();

		return *s_d3d10OriginalInterface;
	}

#define D3D10_ORIGINAL_WRAP(returntype, name, ...) D3D10_ORIGINAL ## name name;
#include "d3d10_original_interfaces.h"
#undef D3D10_ORIGINAL_WRAP

private:
	static D3D10OriginalInterface* s_d3d10OriginalInterface;
};

extern "C"
{
#undef D3D10_ORIGINAL_ARGS
#define D3D10_ORIGINAL_WRAP(returntype, name, ...) returntype __stdcall D3D10##name ( __VA_ARGS__ )
#define D3D10_ORIGINAL_ARGS(returntype, name, ...) { return D3D10OriginalInterface::Get().##name (__VA_ARGS__); }
#include "d3d10_original_interfaces.h"
}
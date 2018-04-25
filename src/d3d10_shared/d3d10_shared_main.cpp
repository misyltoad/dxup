#include "../d3d10_original/d3d10_original.h"

D3D10OriginalInterface* D3D10OriginalInterface::s_d3d10OriginalInterface = nullptr;

extern "C"
{
	const char* __stdcall D3D10GetVertexShaderProfile(ID3D10Device *device)
	{
		return "vs_4_0";
	}

	const char* __stdcall D3D10GetGeometryShaderProfile(ID3D10Device *device)
	{
		return "gs_4_0";
	}

	const char* __stdcall D3D10GetPixelShaderProfile(ID3D10Device *device)
	{
		return "ps_4_0";
	}

	// Unknown params & returntype
	void __stdcall D3D10GetVersion()
	{

	}

	void __stdcall D3D10RegisterLayers()
	{

	}

	void __stdcall RevertToOldImplementation()
	{

	}
}
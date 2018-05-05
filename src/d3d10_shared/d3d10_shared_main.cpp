#include "../d3d10_original/d3d10_original.h"
#include "../d3d10_1/d3d10_1_base.h"

FILE* g_LogFile = nullptr;

D3D10OriginalInterface* D3D10OriginalInterface::s_d3d10OriginalInterface = nullptr;

extern "C"
{
	const char* __stdcall D3D10GetVertexShaderProfile(ID3D10Device *device)
	{
		return "vs_4_1";
	}

	const char* __stdcall D3D10GetGeometryShaderProfile(ID3D10Device *device)
	{
		return "gs_4_1";
	}

	const char* __stdcall D3D10GetPixelShaderProfile(ID3D10Device *device)
	{
		return "ps_4_1";
	}

	bool __stdcall IsDXUP()
	{
		return true;
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

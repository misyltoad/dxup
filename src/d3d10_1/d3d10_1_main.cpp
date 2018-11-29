#include <array>

#include "d3d10_1_device.h"
#include "../dxgi/dxgi_swapchain.h"

#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d10effect.h>

extern "C"
{
	using namespace dxup;

	static D3D_FEATURE_LEVEL featureLevel[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	typedef void (__stdcall *DXUPWrapSwapChain)(IDXGISwapChain** ppSwapChain);

	DXUPWrapSwapChain SwapChainWrapFunc = nullptr;
	void DXGISwapchainWrapper(IDXGISwapChain** ppSwapchain)
	{
		if (!SwapChainWrapFunc)
		{
			HMODULE dxgiModule = LoadLibraryA("dxgi.dll");
			SwapChainWrapFunc = (DXUPWrapSwapChain) GetProcAddress(dxgiModule, "DXUPWrapSwapChain");
		}

		SwapChainWrapFunc(ppSwapchain);
	}

	HRESULT STDMETHODCALLTYPE D3D10CreateDevice1(
		IDXGIAdapter         *pAdapter,
		D3D10_DRIVER_TYPE    DriverType,
		HMODULE              Software,
		UINT                 Flags,
		D3D10_FEATURE_LEVEL1 HardwareLevel,
		UINT                 SDKVersion,
		ID3D10Device1        **ppDevice
	)
	{
		ID3D11Device* pDX11Device = nullptr;
		ID3D11Device1* pDX11Device1 = nullptr;
		D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;

		Flags &= ~D3D10_CREATE_DEVICE_ALLOW_NULL_FROM_MAP;
		Flags &= ~D3D10_CREATE_DEVICE_STRICT_VALIDATION;
		Flags &= ~D3D10_CREATE_DEVICE_DEBUGGABLE;
		Flags |= D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
#ifdef _DEBUG
		//Flags |= D3D11_CREATE_DEVICE_DEBUG;
		//Flags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

		HRESULT result = D3D11CreateDevice(pAdapter, pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags, featureLevel, 6, D3D11_SDK_VERSION, &pDX11Device, &level, nullptr);

		pDX11Device->QueryInterface(__uuidof(ID3D11Device1), (void **)&pDX11Device1);

		*ppDevice = new D3D10Device(pDX11Device1);

		return result;
	}

	HRESULT STDMETHODCALLTYPE D3D10CreateDeviceAndSwapChain1(
		IDXGIAdapter *pAdapter,
		D3D10_DRIVER_TYPE DriverType,
		HMODULE Software,
		UINT Flags,
		D3D10_FEATURE_LEVEL1 HardwareLevel,
		UINT SDKVersion,
		DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
		IDXGISwapChain **ppSwapChain,
		ID3D10Device1 **ppDevice)
	{
		ID3D11Device* pDX11Device = nullptr;
		ID3D11Device1* pDX11Device1 = nullptr;
		IDXGISwapChain* pSwapChain = nullptr;
		D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;

		Flags &= ~D3D10_CREATE_DEVICE_ALLOW_NULL_FROM_MAP;
		Flags &= ~D3D10_CREATE_DEVICE_STRICT_VALIDATION;
		Flags &= ~D3D10_CREATE_DEVICE_DEBUGGABLE;
		Flags |= D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
#ifdef _DEBUG
		//Flags |= D3D11_CREATE_DEVICE_DEBUG;
		//Flags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

		HRESULT result = D3D11CreateDeviceAndSwapChain(pAdapter, pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags, featureLevel, 6, D3D11_SDK_VERSION, pSwapChainDesc, &pSwapChain, &pDX11Device, &level, nullptr);

		pDX11Device->QueryInterface(__uuidof(ID3D11Device1), (void **)&pDX11Device1);

		DXGISwapchainWrapper(&pSwapChain);

		*ppSwapChain = pSwapChain;

		*ppDevice = new D3D10Device(pDX11Device1);

		return result;
	}

	const char* STDMETHODCALLTYPE D3D10GetVertexShaderProfile(ID3D10Device *device)
	{
		return "vs_4_1";
	}

	const char* STDMETHODCALLTYPE D3D10GetGeometryShaderProfile(ID3D10Device *device)
	{
		return "gs_4_1";
	}

	const char* STDMETHODCALLTYPE D3D10GetPixelShaderProfile(ID3D10Device *device)
	{
		return "ps_4_1";
	}

	bool STDMETHODCALLTYPE IsDXUP()
	{
		return true;
	}

	HRESULT STDMETHODCALLTYPE D3D10CreateBlob(SIZE_T size, LPD3D10BLOB *ppBuffer)
	{
		return D3DCreateBlob(size, ppBuffer);
	}

	HRESULT STDMETHODCALLTYPE D3D10GetInputSignatureBlob(const void* pShaderBytecode, SIZE_T bytecodeLength, ID3D10Blob** ppSignatureBlob)
	{
		return D3DGetInputSignatureBlob(pShaderBytecode, bytecodeLength, ppSignatureBlob);
	}

	HRESULT STDMETHODCALLTYPE D3D10GetOutputSignatureBlob(const void *pShaderBytecode, SIZE_T BytecodeLength, ID3D10Blob **ppSignatureBlob)
	{
		return D3DGetOutputSignatureBlob(pShaderBytecode, BytecodeLength, ppSignatureBlob);
	}

	HRESULT STDMETHODCALLTYPE D3D10ReflectShader(const void *pShaderBytecode, SIZE_T BytecodeLength, ID3D10ShaderReflection **ppReflector)
	{
		// Work around dodgy mingw headers.
		const GUID ID3D10ShaderReflectionGUID = {0xc530ad7d, 0x9b16, 0x4395, {0xa9, 0x79, 0xba, 0x2e, 0xcf, 0xf8, 0x3a, 0xdd}};
		return D3DReflect(pShaderBytecode, BytecodeLength, /*__uuidof(ID3D10ShaderReflection)*/ ID3D10ShaderReflectionGUID, (void**)ppReflector);
	}

	HRESULT STDMETHODCALLTYPE D3D10CompileShader(LPCSTR pSrcData, SIZE_T SrcDataSize, LPCSTR pFileName, const D3D10_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags, ID3D10Blob** ppShader, ID3D10Blob** ppErrorMsgs)
	{
		return D3DCompile(pSrcData, SrcDataSize, pFileName, pDefines, pInclude, pFunctionName, pProfile, Flags, 0, ppShader, ppErrorMsgs);
	}

	HRESULT STDMETHODCALLTYPE D3D10CreateEffectFromMemory(void *data, SIZE_T data_size, UINT flags, ID3D10Device *device, ID3D10EffectPool *effect_pool, ID3D10Effect **effect)
	{
		DXUP_Log(Warn, "Stub: D3D10CreateEffectFromMemory");
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE D3D10CompileEffectFromMemory(
		void                     *pData,
		SIZE_T                   DataLength,
		LPCSTR                   pSrcFileName,
		CONST D3D10_SHADER_MACRO *pDefines,
		ID3D10Include            *pInclude,
		UINT                     HLSLFlags,
		UINT                     FXFlags,
		ID3D10Blob               **ppCompiledEffect,
		ID3D10Blob               **ppErrors
	)
	{
		DXUP_Log(Warn, "Stub: D3D10CompileEffectFromMemory");
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE D3D10CreateEffectPoolFromMemory(void *data, SIZE_T data_size, UINT fx_flags, ID3D10Device *device, ID3D10EffectPool **effect_pool)
	{
		DXUP_Log(Warn, "Stub: D3D10CreateEffectPoolFromMemory");
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE D3D10DisassembleEffect(ID3D10Effect *pEffect, BOOL EnableColorCode, ID3D10Blob** ppDisassembly)
	{
		#ifndef __GNUC__
			return D3DDisassemble10Effect(pEffect, 0, ppDisassembly);
		#else
			DXUP_Log(Warn, "Stub: D3D10DisassembleEffect [Linux]");
			return E_NOTIMPL;
		#endif
	}

	HRESULT STDMETHODCALLTYPE D3D10DisassembleShader(const void *pShader, SIZE_T BytecodeLength, BOOL EnableColorCode, LPCSTR pComments, ID3D10Blob **ppDisassembly)
	{
		return D3DDisassemble(pShader, BytecodeLength, 0, pComments, ppDisassembly);
	}

	HRESULT STDMETHODCALLTYPE D3D10PreprocessShader(
		LPCSTR             pSrcData,
		SIZE_T             SrcDataSize,
		LPCSTR             pFileName,
		const D3D10_SHADER_MACRO *pDefines,
		LPD3D10INCLUDE     pInclude,
		ID3D10Blob         **ppShaderText,
		ID3D10Blob         **ppErrorMsgs)
	{
		return D3DPreprocess(pSrcData, SrcDataSize, pFileName, pDefines, pInclude, ppShaderText, ppErrorMsgs);
	}

	// Unknown params & returntype
	void STDMETHODCALLTYPE D3D10GetVersion()
	{
		DXUP_Log(Warn, "Stub: D3D10GetVersion");
	}

	void STDMETHODCALLTYPE D3D10RegisterLayers()
	{
		DXUP_Log(Warn, "Stub: D3D10RegisterLayers");
	}

	void STDMETHODCALLTYPE RevertToOldImplementation()
	{
		DXUP_Log(Warn, "Stub: RevertToOldImplementation");
	}

	HRESULT __stdcall D3D10CreateDevice
	(
		IDXGIAdapter      *pAdapter,
		D3D10_DRIVER_TYPE DriverType,
		HMODULE           Software,
		UINT              Flags,
		UINT              SDKVersion,
		ID3D10Device      **ppDevice
	)
	{
		return D3D10CreateDevice1(pAdapter, DriverType, Software, Flags, D3D10_FEATURE_LEVEL_10_0, SDKVersion, (ID3D10Device1**)ppDevice);
	}

	HRESULT __stdcall D3D10CreateDeviceAndSwapChain(
		IDXGIAdapter         *pAdapter,
		D3D10_DRIVER_TYPE    DriverType,
		HMODULE              Software,
		UINT                 Flags,
		UINT                 SDKVersion,
		DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
		IDXGISwapChain       **ppSwapChain,
		ID3D10Device         **ppDevice
	)
	{
		return D3D10CreateDeviceAndSwapChain1(pAdapter, DriverType, Software, Flags, D3D10_FEATURE_LEVEL_10_0, SDKVersion, pSwapChainDesc, ppSwapChain, (ID3D10Device1**)ppDevice);
	}
}
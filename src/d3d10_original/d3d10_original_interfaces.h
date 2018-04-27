D3D10_ORIGINAL_WRAP(HRESULT, CreateEffectFromMemory, void *data, SIZE_T data_size, UINT flags, ID3D10Device *device, ID3D10EffectPool *effect_pool, ID3D10Effect **effect)
D3D10_ORIGINAL_ARGS(HRESULT, CreateEffectFromMemory, data, data_size, flags, device, effect_pool, effect)

D3D10_ORIGINAL_WRAP(HRESULT, CompileEffectFromMemory, void *data, SIZE_T data_size, const char *filename, const D3D10_SHADER_MACRO *defines, ID3D10Include *include, UINT hlsl_flags, UINT fx_flags, ID3D10Blob **effect, ID3D10Blob **errors)
D3D10_ORIGINAL_ARGS(HRESULT, CompileEffectFromMemory, data, data_size, filename, defines, include, hlsl_flags, fx_flags, effect, errors)

D3D10_ORIGINAL_WRAP(HRESULT, CompileShader, LPCSTR pSrcData, SIZE_T SrcDataSize, LPCSTR pFileName, const D3D10_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags, ID3D10Blob** ppShader, ID3D10Blob** ppErrorMsgs)
D3D10_ORIGINAL_ARGS(HRESULT, CompileShader, pSrcData, SrcDataSize, pFileName, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs)

D3D10_ORIGINAL_WRAP(HRESULT, CreateBlob, SIZE_T size, LPD3D10BLOB *ppBuffer)
D3D10_ORIGINAL_ARGS(HRESULT, CreateBlob, size, ppBuffer)

D3D10_ORIGINAL_WRAP(HRESULT, GetInputSignatureBlob, const void* pShaderBytecode, SIZE_T bytecodeLength, ID3D10Blob** ppSignatureBlob)
D3D10_ORIGINAL_ARGS(HRESULT, GetInputSignatureBlob, pShaderBytecode, bytecodeLength, ppSignatureBlob)

D3D10_ORIGINAL_WRAP(HRESULT, ReflectShader, const void *pShaderBytecode, SIZE_T BytecodeLength, ID3D10ShaderReflection **ppReflector)
D3D10_ORIGINAL_ARGS(HRESULT, ReflectShader, pShaderBytecode, BytecodeLength, ppReflector)\

D3D10_ORIGINAL_WRAP(HRESULT, DisassembleEffect, ID3D10Effect *pEffect, BOOL EnableColorCode, ID3D10Blob** ppDisassembly)
D3D10_ORIGINAL_ARGS(HRESULT, DisassembleEffect, pEffect, EnableColorCode, ppDisassembly)

D3D10_ORIGINAL_WRAP(HRESULT, DisassembleShader, const void       *pShader, SIZE_T     BytecodeLength, BOOL       EnableColorCode, LPCSTR     pComments, ID3D10Blob **ppDisassembly)
D3D10_ORIGINAL_ARGS(HRESULT, DisassembleShader, pShader, BytecodeLength, EnableColorCode, pComments, ppDisassembly)

D3D10_ORIGINAL_WRAP(HRESULT, PreprocessShader, LPCSTR             pSrcData,
	SIZE_T             SrcDataSize,
	LPCSTR             pFileName,
	const D3D10_SHADER_MACRO *pDefines,
	LPD3D10INCLUDE     pInclude,
	ID3D10Blob         **ppShaderText,
	ID3D10Blob         **ppErrorMsgs)

D3D10_ORIGINAL_ARGS(HRESULT, PreprocessShader, pSrcData, SrcDataSize, pFileName, pDefines, pInclude, ppShaderText, ppErrorMsgs)

D3D10_ORIGINAL_WRAP(HRESULT, CreateEffectPoolFromMemory, void *data, SIZE_T data_size, UINT fx_flags,
	ID3D10Device *device, ID3D10EffectPool **effect_pool)
D3D10_ORIGINAL_ARGS(HRESULT, CreateEffectPoolFromMemory, data, data_size, fx_flags,
	device, effect_pool)

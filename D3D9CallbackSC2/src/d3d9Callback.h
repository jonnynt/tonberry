/*
d3dCallback.h
Written by Matthew Fisher

Header for d3d9Callback exports.
*/

//
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the D3DCALLBACK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// D3D9CALLBACK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
//
#ifdef D3DCALLBACK_EXPORTS
#define D3D9CALLBACK_API __declspec(dllexport)
#else
#define D3D9CALLBACK_API __declspec(dllimport)
#endif

D3D9CALLBACK_API void D3D9CallbackInitialize();
D3D9CALLBACK_API void D3D9CallbackFreeMemory();
D3D9CALLBACK_API void ReportCreateVertexShader(CONST DWORD* pFunction, HANDLE Shader);
D3D9CALLBACK_API void ReportCreatePixelShader(CONST DWORD* pFunction, HANDLE Shader);
D3D9CALLBACK_API bool ReportCreateTexture(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle, D3D9Base::IDirect3DTexture9** ppTexture);	//ADDED-OMZY
D3D9CALLBACK_API bool ReportUnlockTexture(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle);
D3D9CALLBACK_API bool ReportUnlockRect(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle);	//ADDED-OMZY
D3D9CALLBACK_API bool ReportUpdateSurface(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle);	//ADDED-OMZY
D3D9CALLBACK_API void ReportLockVertexBuffer(BufferLockData &Data, D3DVERTEXBUFFER_DESC &Desc);
D3D9CALLBACK_API void ReportLockIndexBuffer(BufferLockData &Data, D3DINDEXBUFFER_DESC &Desc);
D3D9CALLBACK_API void ReportDestroy(HANDLE Handle);
D3D9CALLBACK_API bool ReportSetTexture(DWORD Stage, HANDLE *SurfaceHandles, UINT SurfaceHandleCount);	//MODIFIED TO bool-OMZY
D3D9CALLBACK_API void ReportSetViewport(CONST D3DVIEWPORT9 *pViewport);
D3D9CALLBACK_API void ReportSetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
D3D9CALLBACK_API void ReportSetVertexDeclaration(D3DVERTEXELEMENT9 *Elements, UINT ElementCount);
D3D9CALLBACK_API void ReportSetFVF(DWORD FVF);
D3D9CALLBACK_API void ReportSetStreamSource(UINT StreamNumber, HANDLE VBufferHandle, UINT OffsetInBytes, UINT Stride);
D3D9CALLBACK_API void ReportSetStreamSourceFreq(UINT StreamNumber, UINT FrequencyParameter);
D3D9CALLBACK_API void ReportSetLight(DWORD Index, CONST D3DLIGHT9* pLight);
D3D9CALLBACK_API void ReportLightEnable(DWORD Index, BOOL Enable);
D3D9CALLBACK_API void ReportSetMaterial(CONST D3DMATERIAL9* pMaterial);
D3D9CALLBACK_API void ReportSetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
D3D9CALLBACK_API void ReportSetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
D3D9CALLBACK_API void ReportSetIndices(HANDLE IBufferHandle);
D3D9CALLBACK_API void ReportSetVertexShader(HANDLE Shader);
D3D9CALLBACK_API void ReportSetPixelShader(HANDLE Shader);
D3D9CALLBACK_API void ReportSetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount);
D3D9CALLBACK_API void ReportSetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
D3D9CALLBACK_API void ReportSetVertexShaderConstantI(UINT StartRegister, CONST UINT* pConstantData, UINT Vector4iCount);
D3D9CALLBACK_API void ReportSetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount);
D3D9CALLBACK_API void ReportSetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
D3D9CALLBACK_API void ReportSetPixelShaderConstantI(UINT StartRegister, CONST UINT* pConstantData, UINT Vector4iCount);
D3D9CALLBACK_API void ReportSetRenderTarget(DWORD RenderTargetIndex, HANDLE Surface);
D3D9CALLBACK_API bool PreRenderQuery(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount);
D3D9CALLBACK_API bool ReportDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
D3D9CALLBACK_API bool ReportDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
D3D9CALLBACK_API bool ReportDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
D3D9CALLBACK_API bool ReportDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
D3D9CALLBACK_API void ReportPresent(CONST RECT* pSourceRect,CONST RECT* pDestRect);
D3D9CALLBACK_API void ReportClear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
D3D9CALLBACK_API void ReportBeginScene();
D3D9CALLBACK_API void ReportEndScene();
D3D9CALLBACK_API void ReportCreateDevice(D3D9Base::LPDIRECT3DDEVICE9 Device, ID3D9DeviceOverlay *Overlay);
D3D9CALLBACK_API void ReportFreeDevice();

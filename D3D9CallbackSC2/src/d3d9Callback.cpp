#include "Main.h"
#include "d3d9Callback.h"

#pragma data_seg(".HOOKDATA") //Shared data among all instances.
HHOOK hook = NULL;
bool keydown = false;
ofstream hook_debug;
#pragma data_seg()

bool enabled;

extern "C"
{
	IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
	{
		SignalError("Unexpected call to Direct3DCreate9");
		return NULL;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	{
		//g_Context = NULL;
	}
	break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void SafeDLLInitialize()
{
	if (g_Context == NULL) {
		g_Context = new GlobalContext;
		g_Context->Init();
		hook_debug.open("hook.log");
		installhook();
		enabled = true;
	}
}

D3D9CALLBACK_API void D3D9CallbackInitialize() {}
D3D9CALLBACK_API void D3D9CallbackFreeMemory()
{
	if (g_Context != NULL) {
		delete g_Context;
		g_Context = NULL;
		removehook();
		hook_debug.close();
	}
}
D3D9CALLBACK_API void ReportCreateVertexShader(CONST DWORD* pFunction, HANDLE Shader) {}
D3D9CALLBACK_API void ReportCreatePixelShader(CONST DWORD* pFunction, HANDLE Shader) {}
D3D9CALLBACK_API bool ReportCreateTexture(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle, D3D9Base::IDirect3DTexture9** ppTexture) //ADDED-OMZY
{
	g_Context->CreateTexture(Desc, Bmp, Handle, ppTexture);
	return true;
}
D3D9CALLBACK_API bool ReportUnlockTexture(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle)
{
	//g_Context->UnlockTexture(Desc, Bmp, Handle);
	return true;
}
D3D9CALLBACK_API bool ReportUnlockRect(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle) //ADDED-OMZY
{
	g_Context->UnlockRect(Desc, Bmp, Handle);
	return true;
}
D3D9CALLBACK_API bool ReportUpdateSurface(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle) //ADDED-OMZY
{
	g_Context->UpdateSurface(Desc, Bmp, Handle);
	return true;
}
D3D9CALLBACK_API void ReportLockVertexBuffer(BufferLockData &Data, D3DVERTEXBUFFER_DESC &Desc) {}
D3D9CALLBACK_API void ReportLockIndexBuffer(BufferLockData &Data, D3DINDEXBUFFER_DESC &Desc) {}
D3D9CALLBACK_API void ReportDestroy(HANDLE Handle)
{
	g_Context->Destroy(Handle);
}
D3D9CALLBACK_API bool ReportSetTexture(DWORD Stage, HANDLE *SurfaceHandles, UINT SurfaceHandleCount)	//MODIFIED-OMZY
{
	return g_Context->SetTexture(Stage, SurfaceHandles, SurfaceHandleCount);
}
D3D9CALLBACK_API void ReportSetViewport(CONST D3DVIEWPORT9 *pViewport) {}
D3D9CALLBACK_API void ReportSetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {}
D3D9CALLBACK_API void ReportSetVertexDeclaration(D3DVERTEXELEMENT9 *Elements, UINT ElementCount) {}
D3D9CALLBACK_API void ReportSetFVF(DWORD FVF) {}
D3D9CALLBACK_API void ReportSetStreamSource(UINT StreamNumber, HANDLE VBufferHandle, UINT OffsetInBytes, UINT Stride) {}
D3D9CALLBACK_API void ReportSetStreamSourceFreq(UINT StreamNumber, UINT FrequencyParameter) {}
D3D9CALLBACK_API void ReportSetLight(DWORD Index, CONST D3DLIGHT9* pLight) {}
D3D9CALLBACK_API void ReportLightEnable(DWORD Index, BOOL Enable) {}
D3D9CALLBACK_API void ReportSetMaterial(CONST D3DMATERIAL9* pMaterial) {}
D3D9CALLBACK_API void ReportSetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {}
D3D9CALLBACK_API void ReportSetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {}
D3D9CALLBACK_API void ReportSetIndices(HANDLE IBufferHandle) {}
D3D9CALLBACK_API void ReportSetVertexShader(HANDLE Shader) {}
D3D9CALLBACK_API void ReportSetPixelShader(HANDLE Shader) {}
D3D9CALLBACK_API void ReportSetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {}
D3D9CALLBACK_API void ReportSetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {}
D3D9CALLBACK_API void ReportSetVertexShaderConstantI(UINT StartRegister, CONST UINT* pConstantData, UINT Vector4iCount) {}
D3D9CALLBACK_API void ReportSetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {}
D3D9CALLBACK_API void ReportSetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {}
D3D9CALLBACK_API void ReportSetPixelShaderConstantI(UINT StartRegister, CONST UINT* pConstantData, UINT Vector4iCount) {}
D3D9CALLBACK_API void ReportSetRenderTarget(DWORD RenderTargetIndex, HANDLE Surface) {}
D3D9CALLBACK_API bool ReportDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) { return true; }
D3D9CALLBACK_API bool ReportDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount) { return true; }
D3D9CALLBACK_API bool PreRenderQuery(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount) { return false; }
D3D9CALLBACK_API bool ReportDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) { return true; }
D3D9CALLBACK_API bool ReportDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) { return true; }
D3D9CALLBACK_API void ReportPresent(CONST RECT* pSourceRect, CONST RECT* pDestRect) {}
D3D9CALLBACK_API void ReportClear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {}
D3D9CALLBACK_API void ReportBeginScene()
{
	g_Context->BeginScene();
}
D3D9CALLBACK_API void ReportEndScene() {}
D3D9CALLBACK_API void ReportFreeDevice() {}
D3D9CALLBACK_API void ReportCreateDevice(D3D9Base::LPDIRECT3DDEVICE9 Device, ID3D9DeviceOverlay *Overlay)
{
	SafeDLLInitialize();
	g_Context->Graphics.SetDevice(Device);
}

D3D9CALLBACK_API void installhook()
{
	hook = NULL;
	hook = SetWindowsHookEx(WH_GETMESSAGE, GetMessageProc, NULL, GetCurrentThreadId());
	if (hook == NULL) {
		hook_debug << "Unable to install hook" << endl;
	}
}

D3D9CALLBACK_API void removehook()
{
	UnhookWindowsHookEx(hook);
}

D3D9CALLBACK_API LRESULT CALLBACK GetMessageProc(int ncode, WPARAM wparam, LPARAM lparam)
{
	// if ncode < 0 pass control to the next hook
	if (ncode < 0) return CallNextHookEx(hook, ncode, wparam, lparam);

	MSG* msg = (MSG*)lparam;
	if (msg->message == WM_KEYDOWN) {
		switch (msg->wParam) {
		case VK_F9: g_Context->ToggleSetTexture(); break;
		case VK_TAB:
			if (GetKeyState(VK_LSHIFT) & 0x8000)
				g_Context->ToggleBoth();
			break;
		}
	}

	return 0;
}

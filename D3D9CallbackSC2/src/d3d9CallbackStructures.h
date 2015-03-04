//
// d3dCallback.h
//
// Header for d3dCallback exports
//

#pragma once

interface ID3D9DeviceOverlay
{
public:
    virtual void WriteLine(const String &Text, RGBColor Color, UINT PanelIndex) = 0;
    virtual void ClearPanel(UINT PanelIndex) = 0;
    
    virtual void AddMesh(const Mesh &M) = 0;
    virtual void SetMeshTransform(const Matrix4 &Transform) = 0;
    virtual void ClearMeshes() = 0;

    virtual void RenderMeshes() = 0;
};

struct BufferLockData
{
    UINT OffsetToLock;
    UINT SizeToLock;
    DWORD Flags;
    HANDLE Handle;
    VOID* pRAMBuffer;
    VOID* pVideoBuffer;
    bool Create;
};

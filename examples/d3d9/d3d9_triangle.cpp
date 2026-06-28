// Minimal DXVK Native D3D9 example: draws a colored triangle using the
// fixed-function pipeline (pre-transformed XYZRHW vertices with a diffuse
// color, so no shaders or transformation matrices are needed).

#include <cstdio>
#include <cstring>

#include <d3d9.h>

#include "../common/example_window.h"

using namespace dxvk::example;

namespace {
  struct Vertex {
    float    x, y, z, rhw;
    D3DCOLOR color;
  };

  constexpr DWORD VertexFvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
}

int main() {
  Window window = createWindow("DXVK Native - D3D9 triangle", 1280, 720);

  if (!window.hwnd) {
    std::fprintf(stderr, "Failed to create window\n");
    return 1;
  }

  IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

  if (!d3d9) {
    std::fprintf(stderr, "Direct3DCreate9 failed\n");
    return 1;
  }

  D3DPRESENT_PARAMETERS pp = { };
  pp.Windowed         = TRUE;
  pp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
  pp.hDeviceWindow    = window.hwnd;
  pp.BackBufferWidth  = window.width;
  pp.BackBufferHeight = window.height;
  pp.BackBufferFormat = D3DFMT_X8R8G8B8;
  pp.BackBufferCount  = 1;

  IDirect3DDevice9* device = nullptr;

  HRESULT hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
    window.hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device);

  if (FAILED(hr)) {
    std::fprintf(stderr, "CreateDevice failed (0x%08x)\n", (unsigned) hr);
    d3d9->Release();
    return 1;
  }

  const float w = float(window.width);
  const float h = float(window.height);

  const Vertex vertices[] = {
    { 0.50f * w, 0.10f * h, 0.0f, 1.0f, D3DCOLOR_XRGB(255,   0,   0) },
    { 0.85f * w, 0.85f * h, 0.0f, 1.0f, D3DCOLOR_XRGB(  0, 255,   0) },
    { 0.15f * w, 0.85f * h, 0.0f, 1.0f, D3DCOLOR_XRGB(  0,   0, 255) },
  };

  IDirect3DVertexBuffer9* vbo = nullptr;
  device->CreateVertexBuffer(sizeof(vertices), 0, VertexFvf,
    D3DPOOL_MANAGED, &vbo, nullptr);

  void* data = nullptr;
  vbo->Lock(0, sizeof(vertices), &data, 0);
  std::memcpy(data, vertices, sizeof(vertices));
  vbo->Unlock();

  device->SetRenderState(D3DRS_LIGHTING, FALSE);
  device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  while (processEvents(window)) {
    device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(32, 32, 32), 1.0f, 0);

    device->BeginScene();
    device->SetFVF(VertexFvf);
    device->SetStreamSource(0, vbo, 0, sizeof(Vertex));
    device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    device->EndScene();

    device->Present(nullptr, nullptr, nullptr, nullptr);
  }

  vbo->Release();
  device->Release();
  d3d9->Release();
  destroyWindow(window);
  return 0;
}

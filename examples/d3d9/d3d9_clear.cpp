// Minimal DXVK Native D3D9 example: creates a device and a swap chain, then
// clears the back buffer to an animated colour every frame.

#include <cmath>
#include <cstdio>

#include <d3d9.h>

#include "../common/example_window.h"

using namespace dxvk::example;

int main(int argc, char** argv) {
  Window window = createWindow("DXVK Native - D3D9", 1280, 720);

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

  float t = 0.0f;

  while (processEvents(window)) {
    t += 0.02f;
    BYTE r = BYTE(127.5f * (std::sin(t) + 1.0f));
    BYTE g = BYTE(127.5f * (std::sin(t + 2.0f) + 1.0f));
    BYTE b = BYTE(127.5f * (std::sin(t + 4.0f) + 1.0f));

    device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(r, g, b), 1.0f, 0);
    device->Present(nullptr, nullptr, nullptr, nullptr);
  }

  device->Release();
  d3d9->Release();
  destroyWindow(window);
  return 0;
}

// Minimal DXVK Native D3D10 example. DXVK only ships d3d10core (the low-level
// core entry point), so unlike D3D11 there is no D3D10CreateDeviceAndSwapChain
// helper. We build the device and swap chain manually:
//   1. create a DXGI factory and pick the first adapter
//   2. create an ID3D10Device via D3D10CoreCreateDevice
//   3. create a swap chain for the device through the DXGI factory
// then clear the back buffer to an animated colour every frame.

#include <cmath>
#include <cstdio>

#include <d3d10.h>
#include <dxgi.h>

#include "../common/example_window.h"

using namespace dxvk::example;

// Exported by libdxvk_d3d10core but not declared in the public headers.
extern "C" HRESULT __stdcall D3D10CoreCreateDevice(
        IDXGIFactory*     pFactory,
        IDXGIAdapter*     pAdapter,
        UINT              Flags,
        D3D_FEATURE_LEVEL FeatureLevel,
        ID3D10Device**    ppDevice);

int main(int argc, char** argv) {
  Window window = createWindow("DXVK Native - D3D10", 1280, 720);

  if (!window.hwnd) {
    std::fprintf(stderr, "Failed to create window\n");
    return 1;
  }

  IDXGIFactory* factory = nullptr;

  HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory),
    reinterpret_cast<void**>(&factory));

  if (FAILED(hr)) {
    std::fprintf(stderr, "CreateDXGIFactory failed (0x%08x)\n", (unsigned) hr);
    return 1;
  }

  IDXGIAdapter* adapter = nullptr;
  factory->EnumAdapters(0, &adapter);

  ID3D10Device* device = nullptr;
  hr = D3D10CoreCreateDevice(factory, adapter, 0, D3D_FEATURE_LEVEL_10_0, &device);

  if (FAILED(hr)) {
    std::fprintf(stderr, "D3D10CoreCreateDevice failed (0x%08x)\n", (unsigned) hr);
    return 1;
  }

  DXGI_SWAP_CHAIN_DESC scd = { };
  scd.BufferCount       = 2;
  scd.BufferDesc.Width  = window.width;
  scd.BufferDesc.Height = window.height;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow      = window.hwnd;
  scd.SampleDesc.Count  = 1;
  scd.Windowed          = TRUE;
  scd.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  IDXGISwapChain* swapChain = nullptr;
  hr = factory->CreateSwapChain(device, &scd, &swapChain);

  if (FAILED(hr)) {
    std::fprintf(stderr, "CreateSwapChain failed (0x%08x)\n", (unsigned) hr);
    return 1;
  }

  ID3D10Texture2D* backBuffer = nullptr;
  swapChain->GetBuffer(0, __uuidof(ID3D10Texture2D),
    reinterpret_cast<void**>(&backBuffer));

  ID3D10RenderTargetView* rtv = nullptr;
  device->CreateRenderTargetView(backBuffer, nullptr, &rtv);

  float t = 0.0f;

  while (processEvents(window)) {
    t += 0.02f;
    float color[4] = {
      0.5f * (std::sin(t)        + 1.0f),
      0.5f * (std::sin(t + 2.0f) + 1.0f),
      0.5f * (std::sin(t + 4.0f) + 1.0f),
      1.0f,
    };

    device->ClearRenderTargetView(rtv, color);
    swapChain->Present(1, 0);
  }

  rtv->Release();
  backBuffer->Release();
  swapChain->Release();
  device->Release();
  if (adapter)
    adapter->Release();
  factory->Release();
  destroyWindow(window);
  return 0;
}

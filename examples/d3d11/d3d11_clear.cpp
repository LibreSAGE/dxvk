// Minimal DXVK Native D3D11 example: creates a device + swap chain with
// D3D11CreateDeviceAndSwapChain, then clears the back buffer to an animated
// colour every frame.

#include <cmath>
#include <cstdio>

#include <d3d11.h>

#include "../common/example_window.h"

using namespace dxvk::example;

int main() {
  Window window = createWindow("DXVK Native - D3D11", 1280, 720);

  if (!window.hwnd) {
    std::fprintf(stderr, "Failed to create window\n");
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

  IDXGISwapChain*      swapChain = nullptr;
  ID3D11Device*        device    = nullptr;
  ID3D11DeviceContext* context   = nullptr;

  HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
    nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd,
    &swapChain, &device, nullptr, &context);

  if (FAILED(hr)) {
    std::fprintf(stderr, "D3D11CreateDeviceAndSwapChain failed (0x%08x)\n", (unsigned) hr);
    return 1;
  }

  ID3D11Texture2D* backBuffer = nullptr;
  swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
    reinterpret_cast<void**>(&backBuffer));

  ID3D11RenderTargetView* rtv = nullptr;
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

    context->ClearRenderTargetView(rtv, color);
    swapChain->Present(1, 0);
  }

  rtv->Release();
  backBuffer->Release();
  context->Release();
  device->Release();
  swapChain->Release();
  destroyWindow(window);
  return 0;
}

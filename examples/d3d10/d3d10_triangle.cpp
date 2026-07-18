// Minimal DXVK Native D3D10 example: draws a colored triangle. As in
// d3d10_clear.cpp the device is built manually (DXGI factory -> adapter ->
// D3D10CoreCreateDevice -> swap chain). The vertex and pixel shaders are
// precompiled DXBC (see examples/common/triangle.hlsl) embedded as byte
// arrays, since DXVK Native has no runtime HLSL compiler.

#include <cstdio>

#include <d3d10.h>
#include <dxgi.h>

#include "../common/example_window.h"
#include "../common/triangle_vs_dxbc.h"
#include "../common/triangle_ps_dxbc.h"

using namespace dxvk::example;

// Exported by libdxvk_d3d10core but not declared in the public headers.
extern "C" HRESULT __stdcall D3D10CoreCreateDevice(
        IDXGIFactory*     pFactory,
        IDXGIAdapter*     pAdapter,
        UINT              Flags,
        D3D_FEATURE_LEVEL FeatureLevel,
        ID3D10Device**    ppDevice);

namespace {
  struct Vertex {
    float pos[2];
    float color[3];
  };
}

int main(int argc, char** argv) {
  Window window = createWindow("DXVK Native - D3D10 triangle", 1280, 720);

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

  const Vertex vertices[] = {
    { {  0.0f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
  };

  D3D10_BUFFER_DESC bd = { };
  bd.ByteWidth = sizeof(vertices);
  bd.Usage     = D3D10_USAGE_IMMUTABLE;
  bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;

  D3D10_SUBRESOURCE_DATA srd = { };
  srd.pSysMem = vertices;

  ID3D10Buffer* vbo = nullptr;
  device->CreateBuffer(&bd, &srd, &vbo);

  const D3D10_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 0,                            D3D10_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA, 0 },
  };

  ID3D10InputLayout* inputLayout = nullptr;
  device->CreateInputLayout(layout, 2, g_triangle_vs, sizeof(g_triangle_vs), &inputLayout);

  ID3D10VertexShader* vs = nullptr;
  ID3D10PixelShader*  ps = nullptr;
  device->CreateVertexShader(g_triangle_vs, sizeof(g_triangle_vs), &vs);
  device->CreatePixelShader(g_triangle_ps, sizeof(g_triangle_ps), &ps);

  D3D10_VIEWPORT viewport = { };
  viewport.Width    = window.width;
  viewport.Height   = window.height;
  viewport.MaxDepth = 1.0f;

  const float clearColor[4] = { 0.125f, 0.125f, 0.125f, 1.0f };
  const UINT  stride = sizeof(Vertex);
  const UINT  offset = 0;

  while (processEvents(window)) {
    device->ClearRenderTargetView(rtv, clearColor);

    device->OMSetRenderTargets(1, &rtv, nullptr);
    device->RSSetViewports(1, &viewport);
    device->IASetInputLayout(inputLayout);
    device->IASetVertexBuffers(0, 1, &vbo, &stride, &offset);
    device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device->VSSetShader(vs);
    device->PSSetShader(ps);
    device->Draw(3, 0);

    swapChain->Present(1, 0);
  }

  ps->Release();
  vs->Release();
  inputLayout->Release();
  vbo->Release();
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

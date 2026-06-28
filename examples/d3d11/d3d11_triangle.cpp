// Minimal DXVK Native D3D11 example: draws a colored triangle. The vertex and
// pixel shaders are precompiled DXBC (see examples/common/triangle.hlsl),
// embedded as byte arrays because DXVK Native has no runtime HLSL compiler.

#include <cstdio>

#include <d3d11.h>

#include "../common/example_window.h"
#include "../common/triangle_vs_dxbc.h"
#include "../common/triangle_ps_dxbc.h"

using namespace dxvk::example;

namespace {
  struct Vertex {
    float pos[2];
    float color[3];
  };
}

int main() {
  Window window = createWindow("DXVK Native - D3D11 triangle", 1280, 720);

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

  const Vertex vertices[] = {
    { {  0.0f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
  };

  D3D11_BUFFER_DESC bd = { };
  bd.ByteWidth = sizeof(vertices);
  bd.Usage     = D3D11_USAGE_IMMUTABLE;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA srd = { };
  srd.pSysMem = vertices;

  ID3D11Buffer* vbo = nullptr;
  device->CreateBuffer(&bd, &srd, &vbo);

  const D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  ID3D11InputLayout* inputLayout = nullptr;
  device->CreateInputLayout(layout, 2, g_triangle_vs, sizeof(g_triangle_vs), &inputLayout);

  ID3D11VertexShader* vs = nullptr;
  ID3D11PixelShader*  ps = nullptr;
  device->CreateVertexShader(g_triangle_vs, sizeof(g_triangle_vs), nullptr, &vs);
  device->CreatePixelShader(g_triangle_ps, sizeof(g_triangle_ps), nullptr, &ps);

  D3D11_VIEWPORT viewport = { };
  viewport.Width    = float(window.width);
  viewport.Height   = float(window.height);
  viewport.MaxDepth = 1.0f;

  const float clearColor[4] = { 0.125f, 0.125f, 0.125f, 1.0f };
  const UINT  stride = sizeof(Vertex);
  const UINT  offset = 0;

  while (processEvents(window)) {
    context->ClearRenderTargetView(rtv, clearColor);

    context->OMSetRenderTargets(1, &rtv, nullptr);
    context->RSSetViewports(1, &viewport);
    context->IASetInputLayout(inputLayout);
    context->IASetVertexBuffers(0, 1, &vbo, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(vs, nullptr, 0);
    context->PSSetShader(ps, nullptr, 0);
    context->Draw(3, 0);

    swapChain->Present(1, 0);
  }

  ps->Release();
  vs->Release();
  inputLayout->Release();
  vbo->Release();
  rtv->Release();
  backBuffer->Release();
  context->Release();
  device->Release();
  swapChain->Release();
  destroyWindow(window);
  return 0;
}

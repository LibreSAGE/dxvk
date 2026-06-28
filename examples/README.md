# DXVK Native examples

Minimal example programs that drive each Direct3D version supported by DXVK
Native through a windowing toolkit. There are two examples per API:

* **clear** — creates a window, device and swap chain and clears the back
  buffer to an animated colour every frame.
* **triangle** — additionally draws a colored triangle.

| API   | clear                                          | triangle                                              | Entry point(s)                                |
|-------|------------------------------------------------|-------------------------------------------------------|-----------------------------------------------|
| D3D8  | [d3d8/d3d8_clear.cpp](d3d8/d3d8_clear.cpp)     | [d3d8/d3d8_triangle.cpp](d3d8/d3d8_triangle.cpp)      | `Direct3DCreate8`                             |
| D3D9  | [d3d9/d3d9_clear.cpp](d3d9/d3d9_clear.cpp)     | [d3d9/d3d9_triangle.cpp](d3d9/d3d9_triangle.cpp)      | `Direct3DCreate9`                             |
| D3D10 | [d3d10/d3d10_clear.cpp](d3d10/d3d10_clear.cpp) | [d3d10/d3d10_triangle.cpp](d3d10/d3d10_triangle.cpp)  | `CreateDXGIFactory` + `D3D10CoreCreateDevice` |
| D3D11 | [d3d11/d3d11_clear.cpp](d3d11/d3d11_clear.cpp) | [d3d11/d3d11_triangle.cpp](d3d11/d3d11_triangle.cpp)  | `D3D11CreateDeviceAndSwapChain`               |

The D3D8/D3D9 triangles use the fixed-function pipeline; the D3D10/D3D11
triangles use shaders (see [Shaders](#shaders) below).

Each example is built once per available windowing backend, producing the
executables `example_<api>_<example>_<backend>` (e.g. `example_d3d9_triangle_sdl2`):

* **SDL2** (`-DEXAMPLE_WSI_SDL2`)
* **SDL3** (`-DEXAMPLE_WSI_SDL3`)
* **GLFW** (`-DEXAMPLE_WSI_GLFW`)

The shared windowing glue lives in
[common/example_window.h](common/example_window.h).

## How windowing works

DXVK Native does not create windows. The application creates a window with the
toolkit, passes the toolkit's window pointer to the D3D APIs reinterpreted as an
`HWND`, and tells DXVK which WSI backend to use at runtime through the
`DXVK_WSI_DRIVER` environment variable (`SDL2`, `SDL3` or `GLFW`). The window
helper sets this variable for you to match the backend it was compiled with.

## Building

The examples are not built by default. Enable them with `-Dbuild_examples=true`
on a DXVK Native (non-Windows) build. At least one of SDL2, SDL3 or GLFW must be
available:

```sh
meson setup build -Dbuild_examples=true
ninja -C build
```

The resulting binaries are placed in `build/examples/`.

## Shaders

DXVK consumes compiled DXBC bytecode and DXVK Native has no runtime HLSL
compiler, so the D3D10/D3D11 triangle shaders are compiled offline and embedded
as byte arrays in [common/triangle_vs_dxbc.h](common/triangle_vs_dxbc.h) and
[common/triangle_ps_dxbc.h](common/triangle_ps_dxbc.h). The HLSL source is
[common/triangle.hlsl](common/triangle.hlsl) (profiles `vs_4_0` / `ps_4_0`,
accepted by both D3D10 and D3D11).

To regenerate the headers you need a D3DCompile implementation (e.g. Windows
`fxc`, or `d3dcompiler_47.dll` under wine). [common/compile_hlsl.c](common/compile_hlsl.c)
is a small helper that does this:

```sh
# cross-compile with mingw, then run under wine where d3dcompiler_47.dll exists
x86_64-w64-mingw32-gcc compile_hlsl.c -o compile_hlsl.exe
wine compile_hlsl.exe triangle.hlsl vs_main vs_4_0 g_triangle_vs triangle_vs_dxbc.h
wine compile_hlsl.exe triangle.hlsl ps_main ps_4_0 g_triangle_ps triangle_ps_dxbc.h
```

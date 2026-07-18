# DXVK examples

Minimal example programs that drive each Direct3D version supported by DXVK
through a windowing toolkit. They build on both DXVK Native (Linux, macOS) and
Windows. There are two examples per API:

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

DXVK does not create windows — the application does, and hands the resulting
`HWND` to D3D. How that handle is obtained differs per platform, which is the
only part of the examples that is not portable:

* **DXVK Native** — the WSI backends define an `HWND` to *be* the toolkit's
  window pointer, so the helper just reinterprets it. The backend is chosen at
  runtime via the `DXVK_WSI_DRIVER` environment variable (`SDL2`, `SDL3` or
  `GLFW`), which the helper sets to match the toolkit it was compiled against.
* **Windows** — `HWND` is a real OS handle, so the helper asks the toolkit for
  the native handle of the window it created (`SDL_GetWindowWMInfo`,
  `SDL_PROP_WINDOW_WIN32_HWND_POINTER`, `glfwGetWin32Window`). DXVK only has the
  Win32 WSI there, so `DXVK_WSI_DRIVER` is deliberately left unset.

All of this is confined to [common/example_window.h](common/example_window.h);
the example programs themselves are identical across platforms.

## Running

By default an example runs until you close its window. Set
`DXVK_EXAMPLE_FRAMES=<n>` to render `n` frames and exit with code 0 instead —
this is how CI runs them non-interactively:

```sh
DXVK_EXAMPLE_FRAMES=10 ./build/examples/example_d3d9_triangle_sdl2
```

On Windows the DXVK DLLs must sit next to the executable, otherwise the loader
picks Microsoft's `d3d9.dll`/`dxgi.dll` from `System32` instead.

## Building

The examples are not built by default. Enable them with `-Dbuild_examples=true`.
At least one of SDL2, SDL3 or GLFW must be available:

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

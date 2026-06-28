/* Offline HLSL -> DXBC compiler. Runs under wine, uses d3dcompiler_47.dll.
 * Usage: compile_hlsl.exe <in.hlsl> <entry> <profile> <arrayname> <out.h>
 * Emits a C header with `static const unsigned char <arrayname>[] = {...};`
 */
#define COBJMACROS
#include <windows.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <stdlib.h>

typedef HRESULT (WINAPI *PFN_D3DCOMPILE)(
  LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*,
  LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);

int main(int argc, char** argv) {
  if (argc != 6) {
    fprintf(stderr, "usage: %s <in.hlsl> <entry> <profile> <arrayname> <out.h>\n", argv[0]);
    return 2;
  }

  const char* inPath  = argv[1];
  const char* entry   = argv[2];
  const char* profile = argv[3];
  const char* name    = argv[4];
  const char* outPath = argv[5];

  FILE* in = fopen(inPath, "rb");
  if (!in) { fprintf(stderr, "cannot open %s\n", inPath); return 1; }
  fseek(in, 0, SEEK_END);
  long srcLen = ftell(in);
  fseek(in, 0, SEEK_SET);
  char* src = malloc(srcLen + 1);
  fread(src, 1, srcLen, in);
  src[srcLen] = 0;
  fclose(in);

  HMODULE lib = LoadLibraryA("d3dcompiler_47.dll");
  if (!lib) lib = LoadLibraryA("d3dcompiler_43.dll");
  if (!lib) { fprintf(stderr, "cannot load d3dcompiler\n"); return 1; }

  PFN_D3DCOMPILE pD3DCompile = (PFN_D3DCOMPILE) GetProcAddress(lib, "D3DCompile");
  if (!pD3DCompile) { fprintf(stderr, "no D3DCompile export\n"); return 1; }

  ID3DBlob* code = NULL;
  ID3DBlob* errors = NULL;
  /* D3DCOMPILE_OPTIMIZATION_LEVEL3 = (1<<15) */
  HRESULT hr = pD3DCompile(src, srcLen, inPath, NULL, NULL,
    entry, profile, (1 << 15), 0, &code, &errors);

  if (errors) {
    fprintf(stderr, "%.*s\n", (int) ID3D10Blob_GetBufferSize(errors),
      (char*) ID3D10Blob_GetBufferPointer(errors));
  }
  if (FAILED(hr)) { fprintf(stderr, "D3DCompile failed 0x%08lx\n", (unsigned long) hr); return 1; }

  const unsigned char* bytes = (const unsigned char*) ID3D10Blob_GetBufferPointer(code);
  SIZE_T len = ID3D10Blob_GetBufferSize(code);

  FILE* out = fopen(outPath, "wb");
  if (!out) { fprintf(stderr, "cannot write %s\n", outPath); return 1; }
  fprintf(out, "// Generated from %s (%s, %s). Do not edit.\n", inPath, entry, profile);
  fprintf(out, "#pragma once\n");
  fprintf(out, "static const unsigned char %s[] = {", name);
  for (SIZE_T i = 0; i < len; i++) {
    if (i % 16 == 0) fprintf(out, "\n  ");
    fprintf(out, "0x%02x,", bytes[i]);
  }
  fprintf(out, "\n};\n");
  fclose(out);

  fprintf(stderr, "wrote %s (%lu bytes)\n", outPath, (unsigned long) len);
  return 0;
}

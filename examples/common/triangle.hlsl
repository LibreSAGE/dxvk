// Shaders for the D3D10/D3D11 colored-triangle examples.
//
// DXVK consumes compiled DXBC bytecode and there is no HLSL compiler available
// at runtime on DXVK Native, so this is compiled offline and embedded as byte
// arrays in triangle_vs_dxbc.h / triangle_ps_dxbc.h (profiles vs_4_0 / ps_4_0,
// which both D3D10 and D3D11 accept). Regenerate with examples/common/compile_hlsl.c.

struct VSInput {
  float2 pos   : POSITION;
  float3 color : COLOR;
};

struct VSOutput {
  float4 pos   : SV_POSITION;
  float3 color : COLOR;
};

VSOutput vs_main(VSInput input) {
  VSOutput output;
  output.pos   = float4(input.pos, 0.0f, 1.0f);
  output.color = input.color;
  return output;
}

float4 ps_main(VSOutput input) : SV_TARGET {
  return float4(input.color, 1.0f);
}

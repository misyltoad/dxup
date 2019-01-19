struct PixelShaderInput {
  float4 position : SV_POSITION;
  float2 texcoord : TEXCOORD;
};

Texture2D blitTexture : register(t0);
SamplerState defaultSampler : register(s0);

PixelShaderInput blit_vs(uint vertexID : SV_VertexID) {
  PixelShaderInput vout;

	if(vertexID == 3) {
		vout.texcoord = float2(1.0, -1.0);
		vout.position = float4(1.0, 3.0, 0.0, 1.0);
	}
	else if(vertexID == 1) {
		vout.texcoord = float2(-1.0, 1.0);
		vout.position = float4(-3.0, -1.0, 0.0, 1.0);
	}
	else {
		vout.texcoord = float2(1.0, 1.0);
		vout.position = float4(1.0, -1.0, 0.0, 1.0);
	}
	return vout;
}

float4 blit_ps(PixelShaderInput pin) : SV_Target {
  return blitTexture.Sample(defaultSampler, pin.texcoord);
}
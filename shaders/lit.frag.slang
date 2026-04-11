struct VSOutput {
    float4 pos : SV_POSITION;
    float3 color;
    float2 uv;
};

[[vk::binding(0, 1)]]
Texture2D texture;

[[vk::binding(1, 1)]]
SamplerState texSampler;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
   return float4(0.5, 0.5, 0.5, 1.0);
}
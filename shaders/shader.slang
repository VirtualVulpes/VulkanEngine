struct FrameData {
    float4x4 view;
    float4x4 proj;
};

struct ObjectData {
    float4x4 model;
    uint texture_id;
};

[[vk::binding(0, 0)]]
ConstantBuffer<FrameData> frameData;

[[vk::binding(0, 1)]]
ConstantBuffer<ObjectData> objectData;

struct VSInput {
    float3 pos;
    float3 color;
    float2 uv;
}

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 color;
    float2 uv;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    float4 worldPos = mul(objectData.model, float4(input.pos, 1.0));
    float4 viewPos = mul(frameData.view, worldPos);
    output.pos = mul(frameData.proj, viewPos);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

Sampler2D textures[];

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
   return textures[objectData.texture_id].Sample(vertIn.uv) * float4(vertIn.color, 1.0);
}
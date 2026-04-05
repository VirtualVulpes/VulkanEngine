struct FrameData {
    float4x4 view;
    float4x4 proj;
};

struct TransformData {
    float4x4 model;
};

[[vk::binding(0, 0)]]
ConstantBuffer<FrameData> frameData;

[[vk::binding(0, 1)]]
ConstantBuffer<TransformData> transformData;

struct VSInput {
    float3 pos;
    float3 color;
}

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 color;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    float4 worldPos = mul(transformData.model, float4(input.pos, 1.0));
    float4 viewPos = mul(frameData.view, worldPos);
    output.pos = mul(frameData.proj, viewPos);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
    return float4(vertIn.color, 1.0);
}

#include <metal_stdlib>


struct VertexIn {
    float2 position;
    float2 uv;
};


struct VertexFragData {
    float4 position [[position]];
    float2 uv;
    float opacity;
};


struct RenderData {
    uint2 textureSize;
};


vertex VertexFragData VertexFn(
    constant const VertexIn *vertices [[buffer(0)]],
    constant const RenderData *d [[buffer(1)]],
    uint vid [[vertex_id]])
{
    VertexFragData out;

    const float tsx = float(d->textureSize.x) * 0.5;
    const float tsy = float(d->textureSize.y) * 0.5;

    out.position = float4(
       -1.0 + float(vertices[vid].position.x) / tsx,
        1.0 - float(vertices[vid].position.y) / tsy, 0, 1);

    out.uv = vertices[vid].uv;

    return out;
}


fragment half4 FragmentFn(
    VertexFragData in [[stage_in]],
    metal::texture2d<half, metal::access::sample> texture [[texture(0)]])
{
    constexpr metal::sampler s;

    return texture.sample(s, in.uv);
}

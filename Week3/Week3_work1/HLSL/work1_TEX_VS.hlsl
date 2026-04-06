#include "work1_TEX.hlsli"

// 顶点着色器
VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.tex = vIn.tex;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);

    vOut.posH = mul(posW, viewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    return vOut;
}

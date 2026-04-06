#include "work3_TEX.hlsli"
// 这是什么啊？寄存器？
Texture2D g_Tex : register(t0);
SamplerState g_SamLinear : register(s0);
// 像素着色器
float4 PS(VertexOut pIn) : SV_Target
{
    float4 A1, D1, S1;
    float4 A2, D2, S2;
    float4 A3, D3, S3;
    
    // 这是总的光照
    float4 A, D, S;
    A = D = S = float4(0, 0, 0, 0);
    // 获取世界坐标法线
    float3 N = normalize(pIn.normalW);
    // 获取视线的方向向量
    float3 toEyeW = -normalize(g_EyePosW - pIn.posW);
    // 获取光照的反方向向量
    float3 lightDirect = normalize(-g_DirLight.direction);
    
    ComputeDirectionalLight(g_Material, g_DirLight, N, toEyeW, A1, D1, S1);
    A += A1;
    D += D1;
    S += S1;

    ComputePointLight(g_Material, g_PointLight, pIn.posW, N, toEyeW, A2, D2, S2);
    A += A2;
    D += D2;
    S += S2;

    ComputeSpotLight(g_Material, g_SpotLight, pIn.posW, N, toEyeW, A3, D3, S3);
    A += A3;
    D += D3;
    S += S3;
    

    // 获取UV坐标对应的颜色
    float4 texColor = g_Tex.Sample(g_SamLinear, pIn.tex);
    // 采样纹理
    // float4 texColor = g_Tex.Sample(g_SamLinear, rotatedUV);
    
    // 最后根据光源进行相加
    // 镜面反射一般不会收纹理影响，只有另外两个要乘上纹理
    float3 finalColor = texColor.rgb * (A + D).rgb + S.rgb;
    float finalAlpha = g_Material.diffuse.a * (g_DirLight.diffuse.a + g_PointLight.diffuse.a + g_SpotLight.diffuse.a) * 0.5f;
    //finalAlpha = 0.5f * g_DirLight.diffuse.a;
    
    return float4(finalColor, finalAlpha);
}
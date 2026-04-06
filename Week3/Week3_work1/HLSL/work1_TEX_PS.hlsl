#include "work1_TEX.hlsli"
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
    
    // 纹理旋转（绕每个面的中心）
    // 1. 平移到 [-0.5, 0.5] 范围
    float2 uv = float2(pIn.tex.x - 0.5f, pIn.tex.y - 0.5f);
    // 2. 扩展为 4D 齐次坐标，应用 4x4 旋转矩阵，取前两分量
    uv = (mul(uv, g_RotMat).xy);
    // 3. 平移回 [0, 1] 范围
    uv.x += 0.5f;
    uv.y += 0.5f;
    
    
    // 获取UV坐标对应的颜色
    float4 texColor = g_Tex.Sample(g_SamLinear, uv);
    
    // 最后根据光源进行相加
    // 镜面反射一般不会收纹理影响，只有另外两个要乘上纹理
    float3 finalColor = texColor.rgb * (A + D).rgb + S.rgb;
    float finalAlpha = g_Material.diffuse.a * (g_DirLight.diffuse.a + g_PointLight.diffuse.a + g_SpotLight.diffuse.a) * 1.0f;
  
    //return float4(finalColor.rgb, finalAlpha);
    //return float4(uv.x, uv.y, 0, 1);
    //return float4(pIn.tex.x, pIn.tex.y, 0, 1);
    return float4(g_RotMat[0][0], g_RotMat[0][1], g_RotMat[1][0], 1);
}
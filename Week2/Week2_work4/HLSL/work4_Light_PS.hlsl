#include "work4_Light.hlsli"

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
    
    
    // 最后根据光源进行相加
    // 我去，那些函数怎么写啊
    float3 finalColor = pIn.color.rgb * (A + D).rgb + S.rgb;
    float finalAlpha = g_Material.diffuse.a * g_DirLight.diffuse.a;
    
    return float4(finalColor, finalAlpha);
}
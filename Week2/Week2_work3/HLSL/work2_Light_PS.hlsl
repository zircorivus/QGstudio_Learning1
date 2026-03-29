#include "work2_Light.hlsli"

// 像素着色器
float4 PS(VertexOut pIn) : SV_Target
{
    // 这是总的光照
    float4 A, D, S;
    A = D = S = float4(0, 0, 0, 0);
    // 获取世界坐标法线
    float3 N = normalize(pIn.normalW);
    // 获取视线的方向向量
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);
    // 获取光照的反方向向量
    float3 lightDirect = normalize(-g_DirLight.direction);
    
    // 环境光：直接用参数相乘
    float4 Ambient1 = g_Material.ambient * g_DirLight.ambient;
    // 漫反射：取光照向量和法线的点积
    float4 Diffuse1 = max(dot(lightDirect, N), 0.0f) * g_Material.diffuse * g_DirLight.diffuse;
    // 镜面反射
    // 使用pilln-phong模型
    // 先是用归一化计算理想向量
    float3 idealNormal1 = normalize(lightDirect + toEyeW);
    // 计算理想向量和实际向量的点积
    float H1 = max(dot(idealNormal1, N), 0.0f);
    // 得到镜面反射因子
    float specularFactor1 = pow(H1, g_Material.specular.w);
    // 最后才得到镜面反射强度
    float4 Specular1 = specularFactor1 * g_Material.specular * g_DirLight.specular;
    A += Ambient1;
    D += Diffuse1;
    S += Specular1;
    
    // 点光的计算
    // 环境光：直接用参数相乘
    float4 Ambient2 = g_Material.ambient * g_PointLight.ambient;
    // 漫反射：取光照向量和法线的点积
    float3 lightDirect2 = normalize(g_PointLight.position - pIn.posW);
    float4 Diffuse2 = max(dot(lightDirect2, N), 0.0f) * g_Material.diffuse * g_PointLight.diffuse;
    // 镜面反射
    // 使用pilln-phong模型
    // 先是用归一化计算理想向量
    float3 idealNormal2 = normalize(lightDirect2 + toEyeW);
    // 计算理想向量和实际向量的点积
    float H2 = max(dot(idealNormal2, N), 0.0f);
    // 得到镜面反射因子
    float specularFactor2 = pow(H2, g_Material.specular.w);
    // 最后才得到镜面反射强度
    float4 Specular2 = specularFactor2 * g_Material.specular * g_PointLight.specular;
    float d = length(g_PointLight.position - pIn.posW);
    float pointFactor = 1 / (1 + 0.01 * d + 0.01 * d * d);
    A += Ambient2 * pointFactor;
    D += Diffuse2 * pointFactor;
    S += Specular2 * pointFactor;
    
    // 聚光灯的计算
    // 环境光：直接用参数相乘
    float4 Ambient3 = g_Material.ambient * g_SpotLight.ambient;
    // 漫反射：取光照向量和法线的点积
    float3 lightDirect3 = normalize(g_SpotLight.position - pIn.posW);
    float4 Diffuse3 = max(dot(lightDirect3, N), 0.0f) * g_Material.diffuse * g_SpotLight.diffuse;
    // 镜面反射
    // 使用pilln-phong模型
    // 先是用归一化计算理想向量
    float3 idealNormal3 = normalize(lightDirect3 + toEyeW);
    // 计算理想向量和实际向量的点积
    float H3 = max(dot(idealNormal3, N), 0.0f);
    // 得到镜面反射因子
    float specularFactor3 = pow(H3, g_Material.specular.w);
    // 最后才得到镜面反射强度
    float4 Specular3 = specularFactor3 * g_Material.specular * g_SpotLight.specular;
    // 然后计算聚光的光锥因子和衰弱系数
    d = length(g_SpotLight.position - pIn.posW);
    // 光锥因子
    float spot = pow(max(dot(-normalize(g_SpotLight.position - pIn.posW), g_SpotLight.direction), 0.0f), g_SpotLight.Spot);
    // 衰弱因子
    float att = spot / dot(g_SpotLight.att, float3(1.0f, d, d * d));
    A += Ambient3 * att;
    D += Diffuse3 * att;
    S += Specular3 * att;
    
    // 最后根据光源进行相加
    // 我去，那些函数怎么写啊
    float3 finalColor = pIn.color.rgb * (A + D).rgb + S.rgb;
    float finalAlpha = g_Material.diffuse.a * g_DirLight.diffuse.a;
    
    return float4(finalColor, finalAlpha);
}
#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include <d3d11.h>
using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight), m_CBuffer(), m_PyramidWorld(), m_CubeWorld()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    if (!InitEffect())
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    static float phi = 0.0f, theta = 0.0f;
    phi += 0.3f * dt;
    theta += 0.37f * dt;

    // 棱锥：先旋转，再平移到 (-2, 0, 0)
    XMMATRIX pyramidRot = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
    XMMATRIX pyramidTrans = XMMatrixTranslation(-2.0f, 0.0f, 0.0f);
    // 再让世界矩阵等于旋转矩阵和平移矩阵相乘的结果？
    m_PyramidWorld = XMMatrixTranspose(pyramidRot * pyramidTrans);

    // 立方体：先旋转，再平移到 (2, 0, 0)
    XMMATRIX cubeRot = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
    XMMATRIX cubeTrans = XMMatrixTranslation(2.0f, 0.0f, 0.0f);
    // 再让世界矩阵等于旋转矩阵和平移矩阵相乘的结果？
    m_CubeWorld = XMMatrixTranspose(cubeRot * cubeTrans);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    //这里是设置背景色
    static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,0,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&black));
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 准备常量缓冲区的公共部分（view 和 proj 不变）
    // 新建一个常量缓冲区，在绘制两个不同图形前，调用各自不同的世界矩阵，剩下两个矩阵是公用的
    ConstantBuffer cb;
    cb.view = m_CBuffer.view;   // 保留之前计算的 view 和 proj
    cb.proj = m_CBuffer.proj;

    D3D11_MAPPED_SUBRESOURCE mappedData;

    // 绘制棱锥，赋值世界矩阵
    cb.world = m_PyramidWorld;
    // 设置常量缓冲区的映射关系
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    // 上传常量缓冲区
    memcpy_s(mappedData.pData, sizeof(ConstantBuffer), &cb, sizeof(ConstantBuffer));
    // 解除内存到常量缓冲区的映射
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
    // 绘制
    m_pd3dImmediateContext->DrawIndexed(18, 0, 0);   // 棱锥 18 个索引



    // 绘制立方体，赋值世界矩阵
    cb.world = m_CubeWorld;
    // 设置常量缓冲区的映射关系
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    // 上传常量缓冲区
    memcpy_s(mappedData.pData, sizeof(ConstantBuffer), &cb, sizeof(ConstantBuffer));
    // 解除内存到常量缓冲区的映射
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
    //绘制
    m_pd3dImmediateContext->DrawIndexed(36, 18, 5); // 立方体 36 个索引，起始索引 18，顶点偏移 5

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    // 创建顶点着色器
    HR(CreateShaderFromFile(L"HLSL\\work5_VS.cso", L"HLSL\\work5_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
    // 创建并绑定顶点布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // 创建像素着色器
    HR(CreateShaderFromFile(L"HLSL\\work5_PS.cso", L"HLSL\\work5_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

    return true;
}

/// <summary>
/// 初始化渲染所需资源 执行了从创建顶点缓冲区到绑定着色器的完整流程
/// </summary>
/// <returns></returns>
bool GameApp::InitResource()
{
    // 设置三角形顶点 float3和float4分别是位置和颜色
    VertexPosColor vertices[] =
    {
        // 四棱锥
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, // 0
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // 1
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }, // 2
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // 3
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }, // 4

        // 立方体
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }, // 5
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // 6
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }, // 7
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // 8
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, // 9
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }, // 10
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }, // 11
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) } // 12
    };
    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    // 顶点缓冲区的成员：缓冲区内容在创建后不可变（只读），适合静态几何体。这要求初始化时必须提供数据
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    // 顶点缓冲区的成员：缓冲区大小（字节数），刚好容纳三个顶点
    vbd.ByteWidth = sizeof vertices;
    // 顶点缓冲区的成员：指明此缓冲区将用作顶点缓冲区 vertexBuffer
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // 顶点缓冲区的成员：CPU 无需访问（因为不可变）
    vbd.CPUAccessFlags = 0;


    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    // 将初始数据指向刚才新建好的顶点数组
    InitData.pSysMem = vertices;
    // HR() 是一个宏，通常用于检查返回值（若失败则抛出异常或记录错误）
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // 索引数组 1~4
    DWORD indices[] = 
    {
        //四棱锥
        // 正面
        4, 0, 1,
        // 左面
        4, 3, 0,
        // 背面
        4, 2, 3,
        // 右面
        4, 1, 2,
        // 底面
        2, 1, 0,
        3, 2, 0,

        // 立方体
         // 正面
        0, 1, 2,
        2, 3, 0,
        // 左面
        4, 5, 1,
        1, 0, 4,
        // 顶面
        1, 5, 6,
        6, 2, 1,
        // 背面
        7, 6, 5,
        5, 4, 7,
        // 右面
        3, 2, 6,
        6, 7, 3,
        // 底面
        4, 0, 3,
        3, 7, 4
    };

    // 设置索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    // 这是初始化内存吗
    ZeroMemory(&ibd, sizeof(ibd));
    // 设置索引缓冲区的更新类型，创建后不可改变
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    // 将缓冲区宽度设置为索引数组的长度
    ibd.ByteWidth = sizeof indices;
    // 设置缓冲区类型，用作索引缓冲
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    // 这是啥，是说明CPU无需访问
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = indices;
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    // 输入装配阶段的索引缓冲区设置
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


    // ******************
    // 设置常量缓冲区描述
    //
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // 新建常量缓冲区，不使用初始数据
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));


    // 初始化常量缓冲区的值
    // 如果你不熟悉这些矩阵，可以先忽略，待读完第四章后再回头尝试修改
    m_CBuffer.world = XMMatrixIdentity();  // 这里 world 是临时的，之后每帧会覆盖
    m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

    // 上传一次 view 和 proj 即可（world 会在绘制时覆盖）上传常量缓冲区
    // ConstantBuffer是那个可以每帧更新的公用的世界矩阵，这里初始化的时候上传一下，但是上面的drawScene会每帧更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(ConstantBuffer), &m_CBuffer, sizeof(ConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);

    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(VertexPosColor);	// 跨越字节数
    UINT offset = 0;						// 起始偏移量

    //  将顶点缓冲区绑定到输入装配阶段的插槽 0
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    // 设置图元类型，设定输入布局，设置成三角形列表，每三个一组组成三角形
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // 绑定输入布局对象，该对象描述顶点数据如何映射到顶点着色器的输入参数，所以这个布局应该和顶点缓冲区的数据格式保持一致
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    // 将更新好的常量缓冲区绑定到顶点着色器 绑定常量缓冲区
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");
    

    return true;
}

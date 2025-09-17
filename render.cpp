
#include "render.h"
#include <vector>
#include <fstream>
#include <stdlib.h>

struct Vertex {
    float position[3];
    float texcoord[2];
};

bool D3D11YUV420PRenderer::Initialize(HWND hwnd, int width, int height) {

    width_ = width;
    height_ = height;

    DXGI_SWAP_CHAIN_DESC scd = { 0 };
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
        &scd, &swapChain_, &device_, nullptr, &context_);

    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
    if (FAILED(hr)) return false;

    hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_);
    if (FAILED(hr)) return false;

    context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport = { 0 };
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    context_->RSSetViewports(1, &viewport);

    if (!CreateShaders()) return false;
    if (!CreateTextures()) return false;
    if (!CreateVertexBuffer()) return false;

    return true;
}

bool D3D11YUV420PRenderer::CreateShaders() {
    const char* vsCode = R"(
        struct VS_INPUT {
            float3 pos : POSITION;
            float2 tex : TEXCOORD0;
        };
        struct VS_OUTPUT {
            float4 pos : SV_POSITION;
            float2 tex : TEXCOORD0;
        };
        VS_OUTPUT main(VS_INPUT input) {
            VS_OUTPUT output;
            output.pos = float4(input.pos, 1.0);
            output.tex = input.tex;
            return output;
        }
    )";

    const char* psCode = R"(
        Texture2D yTex : register(t0);
        Texture2D uTex : register(t1);
        Texture2D vTex : register(t2);
        SamplerState samp : register(s0);

        static const float3x3 yuv2rgb = float3x3(
            1.164383f,  0.000000f,  1.596027f,
            1.164383f, -0.391762f, -0.812968f,
            1.164383f,  2.017232f,  0.000000f
        );

        float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET {
            float y = yTex.Sample(samp, tex).r - 0.0625f;
            float u = uTex.Sample(samp, tex).r - 0.5f;
            float v = vTex.Sample(samp, tex).r - 0.5f;
            float3 rgb = mul(yuv2rgb, float3(y, u, v));
            return float4(rgb, 1.0f);
        }
    )";

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    HRESULT hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, &vertexShader_);
    if (FAILED(hr)) return false;

    hr = device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, &pixelShader_);
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = device_->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), &inputLayout_);
    if (FAILED(hr)) return false;

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device_->CreateSamplerState(&sampDesc, &samplerState_);
    if (FAILED(hr)) return false;

    return true;
}

bool D3D11YUV420PRenderer::CreateTextures() {
    D3D11_TEXTURE2D_DESC yDesc = { 0 };
    yDesc.Width = width_;
    yDesc.Height = height_;
    yDesc.MipLevels = 1;
    yDesc.ArraySize = 1;
    yDesc.Format = DXGI_FORMAT_R8_UNORM;
    yDesc.SampleDesc.Count = 1;
    yDesc.Usage = D3D11_USAGE_DYNAMIC;
    yDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    yDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


    D3D11_TEXTURE2D_DESC uvDesc = yDesc;
    uvDesc.Width = width_ / 2;
    uvDesc.Height = height_ / 2;

    HRESULT hr = S_OK;
    hr = device_->CreateTexture2D(&yDesc, nullptr, &texturePlanes_[0]);
    hr = device_->CreateTexture2D(&uvDesc, nullptr, &texturePlanes_[1]);
    hr = device_->CreateTexture2D(&uvDesc, nullptr, &texturePlanes_[2]);

    hr = device_->CreateShaderResourceView(texturePlanes_[0].Get(), nullptr,
        &resourceViewPlanes_[0]);
    hr = device_->CreateShaderResourceView(texturePlanes_[1].Get(), nullptr,
        &resourceViewPlanes_[1]);
    hr = device_->CreateShaderResourceView(texturePlanes_[2].Get(), nullptr,
        &resourceViewPlanes_[2]);


    return true;
}

bool D3D11YUV420PRenderer::CreateVertexBuffer() {
    Vertex vertices[] = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}}
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = device_->CreateBuffer(&bd, &initData, &vertexBuffer_);
    return SUCCEEDED(hr);
}

void D3D11YUV420PRenderer::Render(const uint8_t* yuvData) {
    // Update Y plane
    D3D11_MAPPED_SUBRESOURCE mapped;
    context_->Map(texturePlanes_[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    for (int i = 0; i < height_; i++) {
        memcpy((uint8_t*)mapped.pData + i * mapped.RowPitch, yuvData + i * width_, width_);
    }
    context_->Unmap(texturePlanes_[0].Get(), 0);

    // Update U plane
    context_->Map(texturePlanes_[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    const uint8_t* uData = yuvData + width_ * height_;
    for (int i = 0; i < height_ / 2; i++) {
        memcpy((uint8_t*)mapped.pData + i * mapped.RowPitch, uData + i * width_ / 2, width_ / 2);
    }
    context_->Unmap(texturePlanes_[1].Get(), 0);

    // Update V plane
    context_->Map(texturePlanes_[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    uData = yuvData + width_ * height_ + width_ * height_ / 4;
    for (int i = 0; i < height_ / 2; i++) {
        memcpy((uint8_t*)mapped.pData + i * mapped.RowPitch, uData + i * width_ / 2, width_ / 2);
    }
    context_->Unmap(texturePlanes_[2].Get(), 0);

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
    context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
    context_->IASetInputLayout(inputLayout_.Get());
    
    context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
    context_->PSSetShaderResources(0, 1, resourceViewPlanes_[0].GetAddressOf());
    context_->PSSetShaderResources(1, 1, resourceViewPlanes_[1].GetAddressOf());
    context_->PSSetShaderResources(2, 1, resourceViewPlanes_[2].GetAddressOf());
    context_->PSSetSamplers(0, 1, samplerState_.GetAddressOf());

    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context_->Draw(4, 0);
    swapChain_->Present(0, 0);
}

void D3D11YUV420PRenderer::Cleanup() {
    for (auto& view : resourceViewPlanes_) view.Reset();
    for (auto& tex : texturePlanes_) tex.Reset();
    vertexBuffer_.Reset();
    inputLayout_.Reset();
    pixelShader_.Reset();
    vertexShader_.Reset();
    samplerState_.Reset();
    renderTargetView_.Reset();
    swapChain_.Reset();
    context_.Reset();
    device_.Reset();
}

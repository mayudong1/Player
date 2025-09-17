#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <stdint.h>

class D3D11YUV420PRenderer {
public:
    bool Initialize(HWND hwnd, int width, int height);
    void Render(const uint8_t* yuvData);
    void Cleanup();

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

    // YUV textures
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texturePlanes_[3];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> resourceViewPlanes_[3];

    // Shader resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;

    int width_;
    int height_;

    bool CreateShaders();
    bool CreateTextures();
    bool CreateVertexBuffer();
};

#if !defined(NDEBUG) && !defined(_DEBUG)
#define NDEBUG
#endif

#include <stdio.h>
#include <time.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include "resource.h"
#include "SharedConst.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int windowWidth, windowHeight;
HINSTANCE hInstance;
HWND hWnd;

void InitWindows()
{
    // �������� ���������� � ��������� ������
    hInstance = GetModuleHandle(NULL);
    windowWidth = 800;
    windowHeight = 800;

    WNDCLASS wc = {};
    // ��� �������������� ����������
    wc.style = 0;
    // ������ ���������� �������
    wc.lpfnWndProc = &WndProc;
    // ��� �� ����� �������������� ��������� ������ � ��������� ���� � ��������� ������
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    // ������ (���������), �������� ����������� ���������� �������
    wc.hInstance = hInstance;
    // ��������� ����������� ������ � ����������� ������
    wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    // ���� ���� �� �����, ������� �� ������ "�����"
    wc.hbrBackground = NULL;
    // ���� ��� ����
    wc.lpszMenuName = NULL;
    // ������ �������� ������ ����
    wc.lpszClassName = _T("WindowClass1");

    // ������������ ����� ����
    ATOM result = RegisterClass(&wc);
    // ���������, ��� ����� ������� ���������������
    assert(result);

    // ����������� ���� -- ����� ���������, ����� �������� ������ � �.�.
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    RECT rect = {};
    // ���������� ������� (������ �����) �� ������ ������ � ��������� �������
    rect.left = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
    rect.top = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
    rect.right = rect.left + windowWidth;
    rect.bottom = rect.top + windowHeight;
    // �������� ������� ���� � ������. ��������� �������� -- ������� ����
    AdjustWindowRect(&rect, dwStyle, FALSE);

    hWnd = CreateWindow(
        _T("WindowClass1"),
        _T("WindowName1"),
        dwStyle,
        // ����� ������� ���� ����
        rect.left, rect.top,
        // ������ ����
        rect.right - rect.left,
        rect.bottom - rect.top,
        // ������������ ����
        // HWND_DESKTOP ������������ � NULL
        HWND_DESKTOP,
        // ����
        NULL,
        // ������ (���������), ������� ����������� ����
        hInstance,
        // �������������� ��������
        NULL);
    // ���������, ��� ���� ������� �������
    assert(hWnd);
}

void DisposeWindows()
{
    // ������� ����
    DestroyWindow(hWnd);
    // ������� �����
    UnregisterClass(_T("WindowClass1"), hInstance);
}

IDXGISwapChain* swapChain;
ID3D11Device* device;
ID3D11DeviceContext* deviceContext;

void InitSwapChain()
{
    HRESULT result;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    // ������� ��������� � �������� ���������� ����� ����
    swapChainDesc.BufferDesc.Width = windowWidth;
    swapChainDesc.BufferDesc.Height = windowHeight;
    // ����������� ���������� ������ � ������� �������� � ���� ������������� �����
    // �.�. ��� ����� ������������ ������� ������, ���������
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    // ������ ������ -- 32-������ RGBA
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // �� ������ ��������������� ��� ������
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    // �� ���������� �����������
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    // ���������� SwapChain ��� ������
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // ���� "������" (�� ������������) �����
    swapChainDesc.BufferCount = 1;
    // ������ ���� ��� ������
    swapChainDesc.OutputWindow = hWnd;
    // ������� �����
    swapChainDesc.Windowed = TRUE;
    // ����������� ������ ���������� �� ������ ��� ������ �� �����
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // ���������� DirectX 11.0, �.�. ��� ��� ����������
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    // � Debug-������ ������� ����� ������� DirectX
#ifndef NDEBUG
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT flags = 0;
#endif

    result = D3D11CreateDeviceAndSwapChain(
        // ���������� ������������ ��-���������
        NULL,
        // ���������� ���������� ����������
        D3D_DRIVER_TYPE_HARDWARE, NULL,
        // ��. ����
        flags,
        // ���������� ���� ������ DirectX
        &featureLevel, 1,
        // ������ SDK
        D3D11_SDK_VERSION,
        // �������� ��������� ����� ��������
        &swapChainDesc,
        // ���������, ���� �������� ���������
        &swapChain, &device, NULL, &deviceContext);
    // ���������, ��� �������� ������ �������
    assert(SUCCEEDED(result));
}

void DisposeSwapChain()
{
    deviceContext->Release();
    device->Release();
    swapChain->Release();
}

ID3D11RenderTargetView* renderTargetView;

void InitRenderTargetView()
{
    HRESULT result;
    ID3D11Texture2D* backBuffer = NULL;

    // ����� "������" ����� �� SwapChain
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    assert(SUCCEEDED(result));
    assert(backBuffer);

    // �������������� ������ � ������ ��� ������ � ��� ���������
    result = device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
    assert(SUCCEEDED(result));

    // ��������� �� ����� ������ ��� �� �����
    // ����� ��������, ��� ��� ����� ��� ���� �� ���������,
    // �.�. �� ���� �� ��� ��������� SwapChain,
    // Release() ���� ����������� ���������
    backBuffer->Release();

    // ���������� ��������� View ��� ���������
    deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);

    // ������ ������� ���������
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)windowWidth;
    viewport.Height = (FLOAT)windowHeight;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    deviceContext->RSSetViewports(1, &viewport);
}

void DisposeRenderTargetView()
{
    renderTargetView->Release();
}

ID3D11ComputeShader* computeShader;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

ID3D11InputLayout* inputLayout;

void InitShaders()
{
    HRESULT result;
    HRSRC src;
    HGLOBAL res;

    // ������������� ��������������� �������
    // ����� ���������� � ����������� ���� �������
    // �������� ������ �� ������������, �.�. ������� ���������� ������
    // ������������ ����� � �� ����� ���� �� ������
    src = FindResource(hInstance, MAKEINTRESOURCE(IDR_BYTECODE_COMPUTE), _T("ShaderObject"));
    res = LoadResource(hInstance, src);
    // �������������� ������
    result = device->CreateComputeShader(
        // ������� ������� � ��� ������
        res, SizeofResource(hInstance, src),
        // �������� ��� ������������. � ����� ������ �� ������������, �.�. ������ �� ������ �������
        NULL,
        // ��������� �� ������
        &computeShader);
    assert(SUCCEEDED(result));
    FreeResource(res);

    // ����������� �������� ��� ����������� �������
    src = FindResource(hInstance, MAKEINTRESOURCE(IDR_BYTECODE_PIXEL), _T("ShaderObject"));
    res = LoadResource(hInstance, src);
    result = device->CreatePixelShader(res, SizeofResource(hInstance, src),
        NULL, &pixelShader);
    assert(SUCCEEDED(result));
    FreeResource(res);

    // ���������� ��� ���������� �������
    src = FindResource(hInstance, MAKEINTRESOURCE(IDR_BYTECODE_VERTEX), _T("ShaderObject"));
    res = LoadResource(hInstance, src);
    result = device->CreateVertexShader(res, SizeofResource(hInstance, src),
        NULL, &vertexShader);
    assert(SUCCEEDED(result));

    // ������, ��� � ��������� ������ ����� ��������� ������
    // ������� ��� ��������� --- ������� � ���� �����
    D3D11_INPUT_ELEMENT_DESC inputDescs[2] = {};
    // �������� ������� ��������� �������
    // ������������� ��� ���������
    inputDescs[0].SemanticName = "POSITION";
    // ����� ������ � ������, ���� ��������� � ������ ������������� ������ ������ ������
    inputDescs[0].SemanticIndex = 0;
    // ��������� ������ �� 32-������ ������������ �����
    inputDescs[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    // ������ ��������
    inputDescs[0].InputSlot = 0;
    // �������������� ��������
    inputDescs[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    // ��� ������ �������
    inputDescs[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    // ���������� ������� ��� ���������
    inputDescs[0].InstanceDataStepRate = 0;

    // �������� ������� ��������� �������, ����������
    inputDescs[1].SemanticName = "COLOR";
    inputDescs[1].SemanticIndex = 0;
    // ���� --- ������ �� ��� 32-������ ������������ �����
    inputDescs[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    // ������ ��������
    inputDescs[1].InputSlot = 1;
    inputDescs[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    inputDescs[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputDescs[1].InstanceDataStepRate = 0;


    result = device->CreateInputLayout(
        // ������ �������� ���������� � ��� �����
        inputDescs, 2,
        // ������� � ��� �����
        res, SizeofResource(hInstance, src),
        // ��������� �����
        &inputLayout);
    assert(SUCCEEDED(result));

    FreeResource(res);
}

void DisposeShaders()
{
    inputLayout->Release();
    computeShader->Release();
    vertexShader->Release();
    pixelShader->Release();
}

ID3D11Buffer* positionBuffer;
ID3D11Buffer* colorBuffer;
ID3D11Buffer* velocityBuffer;

void InitBuffers()
{
    HRESULT result;

    float* data = new float[3 * PARTICLE_COUNT];
    // �������� �������, ������� ����� ������� � ����� ��� ��������
    D3D11_SUBRESOURCE_DATA subresource = {};
    // ��������� �� ������
    subresource.pSysMem = data;
    // ����� �������� ������ ��� �������
    subresource.SysMemPitch = 0;
    // ����� �������� ������ ��� ���������� �������
    subresource.SysMemSlicePitch = 0;

    // �������� ������
    D3D11_BUFFER_DESC desc = {};
    // ��� ������
    desc.ByteWidth = sizeof(float[2 * PARTICLE_COUNT]);
    // ������ �� ������ � ������
    desc.Usage = D3D11_USAGE_DEFAULT;
    // ����� ������� ���������� � ��� ���������, � ��� ����������� ��� ������
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
    // ������ � ���������� �� �����
    desc.CPUAccessFlags = 0;
    // �������������� ����� �� �����
    desc.MiscFlags = 0;
    // ������ ������ �������� ������
    desc.StructureByteStride = sizeof(float[2]);

    // �������������� ������ �������
    for (int i = 0; i < 2 * PARTICLE_COUNT; i++)
        data[i] = 2.0f * rand() / RAND_MAX - 1.0f;

    // ������� ����� �������
    result = device->CreateBuffer(&desc, &subresource, &positionBuffer);
    assert(SUCCEEDED(result));

    // ����� ��������� ������������ ������ ��� ����������� ��� ������
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

    // �������������� ������ ���������
    for (int i = 0; i < 2 * PARTICLE_COUNT; i++)
        data[i] = 0.0f;

    // ������� ����� ���������
    result = device->CreateBuffer(&desc, &subresource, &velocityBuffer);
    assert(SUCCEEDED(result));

    desc.ByteWidth = sizeof(float[3 * PARTICLE_COUNT]);
    // ����� ������������ ������ � ��������� �������
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // ����, � ������� �� ������� � �������� --- 3 �����
    desc.StructureByteStride = sizeof(float[3]);
    // �������������� ������ ������
    for (int i = 0; i < 3 * PARTICLE_COUNT; i++)
        data[i] = 1.0f * rand() / RAND_MAX;

    result = device->CreateBuffer(&desc, &subresource, &colorBuffer);
    assert(SUCCEEDED(result));

    // ����������� ������, �������������� ��� �������������
    delete[] data;
}

void DisposeBuffers()
{
    positionBuffer->Release();
    colorBuffer->Release();
    velocityBuffer->Release();
}

ID3D11UnorderedAccessView* positionUAV;
ID3D11UnorderedAccessView* velocityUAV;

void InitUAV()
{
    HRESULT result;

    // �������� ������� � ������ �� ������� ��� � �������
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
    // ��������� ������ �� 32-������ ������������ �����
    desc.Format = DXGI_FORMAT_R32G32_FLOAT;
    // ������ � ������, ���� ����� �������� � ����������
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    // ������ � ������� ��������
    desc.Buffer.FirstElement = 0;
    // ���������� ���������
    desc.Buffer.NumElements = PARTICLE_COUNT;
    // ��� �������������� ������
    desc.Buffer.Flags = 0;

    // ������������� ������� � ������ �������
    result = device->CreateUnorderedAccessView(positionBuffer, &desc,
        &positionUAV);
    assert(SUCCEEDED(result));
    // ������������� ������� � ������ ���������
    result = device->CreateUnorderedAccessView(velocityBuffer, &desc,
        &velocityUAV);
    assert(SUCCEEDED(result));
}

void DisposeUAV()
{
    positionUAV->Release();
    velocityUAV->Release();
}

void InitBindings()
{
    // ������������� ������������ �������
    // ��������������
    deviceContext->CSSetShader(computeShader, NULL, 0);
    // ���������
    deviceContext->VSSetShader(vertexShader, NULL, 0);
    // ����������
    deviceContext->PSSetShader(pixelShader, NULL, 0);

    // ������������� ������ � ������ ��������� � ��������������� �������
    deviceContext->CSSetUnorderedAccessViews(1, 1, &velocityUAV, NULL);
    // ������������� ������ ������ ���������� ���������� �������
    deviceContext->IASetInputLayout(inputLayout);
    // �������� ����� �����
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

const int FRAME_TIME_COUNT = 128;
clock_t frameTime[FRAME_TIME_COUNT];
int currentFrame = 0;

float AverageFrameTime()
{
    frameTime[currentFrame] = clock();
    int nextFrame = (currentFrame + 1) % FRAME_TIME_COUNT;
    clock_t delta = frameTime[currentFrame] - frameTime[nextFrame];
    currentFrame = nextFrame;
    return (float)delta / CLOCKS_PER_SEC / FRAME_TIME_COUNT;
}

void Frame()
{
    float frameTime = AverageFrameTime();

    // ������� ���������
    char buf[256];
    sprintf_s(buf, "average framerate: %.1f", 1.0f / frameTime);
    SetWindowTextA(hWnd, buf);

    // ������� ����� ������ ������
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);

    // ��������� ������� �� 32-������ ������������ ���� ������
    UINT strides[] = { sizeof(float[2]), sizeof(float[3]) };
    UINT offsets[] = { 0, 0 };

    ID3D11Buffer* nullBuffer = NULL;
    ID3D11UnorderedAccessView* nullUAV = NULL;

    // ������� ������ ���������� ������� � ������ �������
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, strides, offsets);
    // ������������� ������ ��������������� ������� � ������ �������
    deviceContext->CSSetUnorderedAccessViews(0, 1, &positionUAV, NULL);
    // �������� �������������� ������
    deviceContext->Dispatch(PARTICLE_COUNT / NUMTHREADS, 1, 1);

    // ������� ������ ��������������� ������� � ������ �������
    deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
    // ������������� ������ ���������� ������� � ������� ������� � ������
    ID3D11Buffer* vertBuffers[] = { positionBuffer, colorBuffer };
    deviceContext->IASetVertexBuffers(0, 2, vertBuffers, strides, offsets);
    // �������� ���������
    deviceContext->Draw(PARTICLE_COUNT, 0);

    // ������� ����������� �� �����
    swapChain->Present(1, 0);
}

void ResizeSwapChain()
{
    HRESULT result;

    RECT rect;
    // �������� ���������� ������� ����
    GetClientRect(hWnd, &rect);
    windowWidth = rect.right - rect.left;
    windowHeight = rect.bottom - rect.top;

    // ��� ����, ����� �������� ������ �����������, �����
    // ���������� ��� ��������� �� "������" �����
    DisposeRenderTargetView();
    // �������� ������ �����������
    result = swapChain->ResizeBuffers(
        // �������� ������ ���� �������
        0,
        // ����� �������
        windowWidth, windowHeight,
        // �� ������ ������ � �����
        DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(result));
    // ������� ����� ������ � "�������" ������
    InitRenderTargetView();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        break;

    case WM_SIZE:
        ResizeSwapChain();
        break;

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    return 0;
}

int main()
{
    InitWindows();
    InitSwapChain();
    InitRenderTargetView();
    InitShaders();
    InitBuffers();
    InitUAV();
    InitBindings();

    ShowWindow(hWnd, SW_SHOW);

    bool shouldExit = false;
    while (!shouldExit)
    {
        Frame();

        MSG msg;
        while (!shouldExit && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                shouldExit = true;
        }
    }

    DisposeUAV();
    DisposeBuffers();
    DisposeShaders();
    DisposeRenderTargetView();
    DisposeSwapChain();
    DisposeWindows();
}

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
    // Получаем информацию о локальном модуле
    hInstance = GetModuleHandle(NULL);
    windowWidth = 800;
    windowHeight = 800;

    WNDCLASS wc = {};
    // Без дополнительных параметров
    wc.style = 0;
    // Задаем обработчик событий
    wc.lpfnWndProc = &WndProc;
    // Нам не нужно дополнительное выделение памяти к структуре окна и структуре класса
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    // Модуль (программа), которому принадлежит обработчик событий
    wc.hInstance = hInstance;
    // Загружаем стандартный курсор и стандартную иконку
    wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    // Цвет фона не важен, поэтому не задаем "кисть"
    wc.hbrBackground = NULL;
    // Окно без меню
    wc.lpszMenuName = NULL;
    // Задаем название класса окна
    wc.lpszClassName = _T("WindowClass1");

    // Регистрируем класс окна
    ATOM result = RegisterClass(&wc);
    // Проверяем, что класс успешно зарегистрирован
    assert(result);

    // Стандартное окно -- имеет заголовок, можно изменить размер и т.д.
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    RECT rect = {};
    // Клиентская область (внутри рамки) по центру экрана и заданного размера
    rect.left = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
    rect.top = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
    rect.right = rect.left + windowWidth;
    rect.bottom = rect.top + windowHeight;
    // Вычислим область окна с рамкой. Последний параметр -- наличие меню
    AdjustWindowRect(&rect, dwStyle, FALSE);

    hWnd = CreateWindow(
        _T("WindowClass1"),
        _T("WindowName1"),
        dwStyle,
        // Левый верхний угол окна
        rect.left, rect.top,
        // Размер окна
        rect.right - rect.left,
        rect.bottom - rect.top,
        // Родительское окно
        // HWND_DESKTOP раскрывается в NULL
        HWND_DESKTOP,
        // Меню
        NULL,
        // Модуль (программа), которой принадлежит окно
        hInstance,
        // Дополнительные свойства
        NULL);
    // Проверяем, что окно успешно создано
    assert(hWnd);
}

void DisposeWindows()
{
    // Удаляем окно
    DestroyWindow(hWnd);
    // Удаляем класс
    UnregisterClass(_T("WindowClass1"), hInstance);
}

IDXGISwapChain* swapChain;
ID3D11Device* device;
ID3D11DeviceContext* deviceContext;

void InitSwapChain()
{
    HRESULT result;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    // Разрмер совпадает с размером клиентской части окна
    swapChainDesc.BufferDesc.Width = windowWidth;
    swapChainDesc.BufferDesc.Height = windowHeight;
    // Ограничение количества кадров в секунду задается в виде рационального числа
    // Т.к. нам нужна максимальная частота кадров, отключаем
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    // Формат вывода -- 32-битный RGBA
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // Не задаем масштабирования при выводе
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    // Не используем сглаживание
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    // Используем SwapChain для вывода
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // Один "задний" (не отображаемый) буфер
    swapChainDesc.BufferCount = 1;
    // Задаем окно для вывода
    swapChainDesc.OutputWindow = hWnd;
    // Оконный режим
    swapChainDesc.Windowed = TRUE;
    // Отбрасываем старую информацию из буфера при выводе на экран
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // Используем DirectX 11.0, т.к. его нам достаточно
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    // В Debug-версии создаем поток отладки DirectX
#ifndef NDEBUG
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT flags = 0;
#endif

    result = D3D11CreateDeviceAndSwapChain(
        // Используем видеоадаптер по-умолчанию
        NULL,
        // Используем аппаратную реализацию
        D3D_DRIVER_TYPE_HARDWARE, NULL,
        // См. выше
        flags,
        // Используем одну версию DirectX
        &featureLevel, 1,
        // Версия SDK
        D3D11_SDK_VERSION,
        // Передаем созданное ранее описание
        &swapChainDesc,
        // Указатели, куда записать результат
        &swapChain, &device, NULL, &deviceContext);
    // Проверяем, что операция прошла успешно
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

    // Берем "задний" буфер из SwapChain
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    assert(SUCCEEDED(result));
    assert(backBuffer);

    // Инициализируем доступ к буферу для записи и для отрисовки
    result = device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
    assert(SUCCEEDED(result));

    // Указатель на буфер больше нам не нужен
    // Стоит отметить, что сам буфер при этом не удаляется,
    // т.к. на него всё ещё указывает SwapChain,
    // Release() лишь освобождает указатель
    backBuffer->Release();

    // Используем созданный View для отрисовки
    deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);

    // Задаем область отрисовки
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

    // Инициализация вычислительного шейдера
    // Берем встроенные в исполняемый файл ресурсы
    // Проверка ошибок не производится, т.к. байткод расположен внутри
    // исполняемого файла и не может быть не найден
    src = FindResource(hInstance, MAKEINTRESOURCE(IDR_BYTECODE_COMPUTE), _T("ShaderObject"));
    res = LoadResource(hInstance, src);
    // Инициализируем шейдер
    result = device->CreateComputeShader(
        // Байткод шейдера и его размер
        res, SizeofResource(hInstance, src),
        // Свойства для компоновщика. В нашем случае не используется, т.к. шейдер из одного объекта
        NULL,
        // Указатель на шейдер
        &computeShader);
    assert(SUCCEEDED(result));
    FreeResource(res);

    // Аналогичные операции для пиксельного шейдера
    src = FindResource(hInstance, MAKEINTRESOURCE(IDR_BYTECODE_PIXEL), _T("ShaderObject"));
    res = LoadResource(hInstance, src);
    result = device->CreatePixelShader(res, SizeofResource(hInstance, src),
        NULL, &pixelShader);
    assert(SUCCEEDED(result));
    FreeResource(res);

    // Аналогично для вершинного шейдера
    src = FindResource(hInstance, MAKEINTRESOURCE(IDR_BYTECODE_VERTEX), _T("ShaderObject"));
    res = LoadResource(hInstance, src);
    result = device->CreateVertexShader(res, SizeofResource(hInstance, src),
        NULL, &vertexShader);
    assert(SUCCEEDED(result));

    // Задаем, как в вершинный шейдер будут вводиться данные
    // Зададим два аргумента --- позицию и цвет точки
    D3D11_INPUT_ELEMENT_DESC inputDescs[2] = {};
    // Описание первого аргумента функции
    // Семантическое имя аргумента
    inputDescs[0].SemanticName = "POSITION";
    // Нужно только в случае, если элементов с данным семантическим именем больше одного
    inputDescs[0].SemanticIndex = 0;
    // Двумерный вектор из 32-битных вещественных чисел
    inputDescs[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    // Первый параметр
    inputDescs[0].InputSlot = 0;
    // Необязательный аргумент
    inputDescs[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    // Для каждой вершины
    inputDescs[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    // Используем вершины для отрисовки
    inputDescs[0].InstanceDataStepRate = 0;

    // Описание второго аргумента функции, аналогично
    inputDescs[1].SemanticName = "COLOR";
    inputDescs[1].SemanticIndex = 0;
    // Цвет --- вектор из трёх 32-битных вещественных чисел
    inputDescs[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    // Второй параметр
    inputDescs[1].InputSlot = 1;
    inputDescs[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    inputDescs[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputDescs[1].InstanceDataStepRate = 0;


    result = device->CreateInputLayout(
        // Массив описаний аргументов и его длина
        inputDescs, 2,
        // Байткод и его длина
        res, SizeofResource(hInstance, src),
        // Структура ввода
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
    // Описание массива, который будет записан в буфер при создании
    D3D11_SUBRESOURCE_DATA subresource = {};
    // Указатель на массив
    subresource.pSysMem = data;
    // Имеет значение только для текстур
    subresource.SysMemPitch = 0;
    // Имеет значение только для трехмерных текстур
    subresource.SysMemSlicePitch = 0;

    // Описание буфера
    D3D11_BUFFER_DESC desc = {};
    // Его размер
    desc.ByteWidth = sizeof(float[2 * PARTICLE_COUNT]);
    // Доступ на чтение и запись
    desc.Usage = D3D11_USAGE_DEFAULT;
    // Буфер позиций используем и для отрисовки, и при вычислениях как массив
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
    // Доступ с процессора не нужен
    desc.CPUAccessFlags = 0;
    // Дополнительные флаги не нужны
    desc.MiscFlags = 0;
    // Размер одного элемента буфера
    desc.StructureByteStride = sizeof(float[2]);

    // Инициализируем массив позиций
    for (int i = 0; i < 2 * PARTICLE_COUNT; i++)
        data[i] = 2.0f * rand() / RAND_MAX - 1.0f;

    // Создаем буфер позиций
    result = device->CreateBuffer(&desc, &subresource, &positionBuffer);
    assert(SUCCEEDED(result));

    // Буфер скоростей используется только при вычислениях как массив
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

    // Инициализируем массив скоростей
    for (int i = 0; i < 2 * PARTICLE_COUNT; i++)
        data[i] = 0.0f;

    // Создаем буфер скоростей
    result = device->CreateBuffer(&desc, &subresource, &velocityBuffer);
    assert(SUCCEEDED(result));

    desc.ByteWidth = sizeof(float[3 * PARTICLE_COUNT]);
    // Цвета используются только в вершинном шейдере
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // Цвет, в отличие от позиции и скорости --- 3 числа
    desc.StructureByteStride = sizeof(float[3]);
    // Инициализируем массив цветов
    for (int i = 0; i < 3 * PARTICLE_COUNT; i++)
        data[i] = 1.0f * rand() / RAND_MAX;

    result = device->CreateBuffer(&desc, &subresource, &colorBuffer);
    assert(SUCCEEDED(result));

    // Освобождаем память, использованную для инициализации
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

    // Описание доступа к буферу из шейдера как к массиву
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
    // Двумерный вектор из 32-битных вещественных чисел
    desc.Format = DXGI_FORMAT_R32G32_FLOAT;
    // Доступ к буферу, есть также варианты с текстурами
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    // Доступ с первого элемента
    desc.Buffer.FirstElement = 0;
    // Количество элементов
    desc.Buffer.NumElements = PARTICLE_COUNT;
    // Без дополнительных флагов
    desc.Buffer.Flags = 0;

    // Инициализация доступа к буферу позиций
    result = device->CreateUnorderedAccessView(positionBuffer, &desc,
        &positionUAV);
    assert(SUCCEEDED(result));
    // Инициализация доступа к буферу скоростей
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
    // Устанавливаем используемые шейдеры
    // Вычислительный
    deviceContext->CSSetShader(computeShader, NULL, 0);
    // Вершинный
    deviceContext->VSSetShader(vertexShader, NULL, 0);
    // Пиксельный
    deviceContext->PSSetShader(pixelShader, NULL, 0);

    // Устанавливаем доступ к буферу скоростей у вычислительного шейдера
    deviceContext->CSSetUnorderedAccessViews(1, 1, &velocityUAV, NULL);
    // Устанавливаем способ записи аргументов вершинного шейдера
    deviceContext->IASetInputLayout(inputLayout);
    // Рисовать будем точки
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

    // Выводим фреймрейт
    char buf[256];
    sprintf_s(buf, "average framerate: %.1f", 1.0f / frameTime);
    SetWindowTextA(hWnd, buf);

    // Очищаем буфер черным цветом
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);

    // Двумерные вектора из 32-битных вещественных идут подряд
    UINT strides[] = { sizeof(float[2]), sizeof(float[3]) };
    UINT offsets[] = { 0, 0 };

    ID3D11Buffer* nullBuffer = NULL;
    ID3D11UnorderedAccessView* nullUAV = NULL;

    // Убираем доступ вершинного шейдера к буферу позиций
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, strides, offsets);
    // Устанавливаем доступ вычислительного шейдера к буферу позиций
    deviceContext->CSSetUnorderedAccessViews(0, 1, &positionUAV, NULL);
    // Вызываем вычислительный шейдер
    deviceContext->Dispatch(PARTICLE_COUNT / NUMTHREADS, 1, 1);

    // Убираем доступ вычислительного шейдера к буферу позиций
    deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
    // Устанавливаем доступ вершинного шейдера к буферам позиций и цветов
    ID3D11Buffer* vertBuffers[] = { positionBuffer, colorBuffer };
    deviceContext->IASetVertexBuffers(0, 2, vertBuffers, strides, offsets);
    // Вызываем отрисовку
    deviceContext->Draw(PARTICLE_COUNT, 0);

    // Выводим изображение на экран
    swapChain->Present(1, 0);
}

void ResizeSwapChain()
{
    HRESULT result;

    RECT rect;
    // Получаем актуальные размеры окна
    GetClientRect(hWnd, &rect);
    windowWidth = rect.right - rect.left;
    windowHeight = rect.bottom - rect.top;

    // Для того, чтобы изменить размер изображения, нужно
    // освободить все указатели на "задний" буфер
    DisposeRenderTargetView();
    // Изменяем размер изображения
    result = swapChain->ResizeBuffers(
        // Изменяем размер всех буферов
        0,
        // Новые размеры
        windowWidth, windowHeight,
        // Не меняем формат и флаги
        DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(result));
    // Создаем новый доступ к "заднему" буферу
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

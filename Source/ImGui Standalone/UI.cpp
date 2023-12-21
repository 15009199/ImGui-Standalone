#include "UI.h"
#include "Drawing.h"

ID3D11Device* UI::pd3dDevice = nullptr;
ID3D11DeviceContext* UI::pd3dDeviceContext = nullptr;
IDXGISwapChain* UI::pSwapChain = nullptr;
ID3D11RenderTargetView* UI::pMainRenderTargetView = nullptr;

HMODULE UI::hCurrentModule = nullptr;

bool UI::CreateDeviceD3D(HWND hWnd) {
    MyLogger::LogHRESULT(S_OK, "Creating D3D Device", __FILE__, __LINE__);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext) != S_OK) {
        MyLogger::LogHRESULT(E_FAIL, "Failed to create D3D Device", __FILE__, __LINE__);
        return false;
    }

    MyLogger::LogHRESULT(S_OK, "D3D Device and SwapChain created successfully", __FILE__, __LINE__);
    CreateRenderTarget();
    return true;
}

void UI::CreateRenderTarget() {
    MyLogger::LogHRESULT(S_OK, "Creating Render Target", __FILE__, __LINE__);

    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer != nullptr) {
        pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pMainRenderTargetView);
        pBackBuffer->Release();
    }

    MyLogger::LogHRESULT(S_OK, "Render Target created successfully", __FILE__, __LINE__);
}

void UI::CleanupRenderTarget() {
    MyLogger::LogHRESULT(S_OK, "Cleaning up Render Target", __FILE__, __LINE__);

    if (pMainRenderTargetView) {
        pMainRenderTargetView->Release();
        pMainRenderTargetView = nullptr;
    }

    MyLogger::LogHRESULT(S_OK, "Render Target cleaned up successfully", __FILE__, __LINE__);
}

void UI::CleanupDeviceD3D() {
    MyLogger::LogHRESULT(S_OK, "Cleaning up D3D Device", __FILE__, __LINE__);

    CleanupRenderTarget();
    if (pSwapChain) {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }

    if (pd3dDeviceContext) {
        pd3dDeviceContext->Release();
        pd3dDeviceContext = nullptr;
    }

    if (pd3dDevice) {
        pd3dDevice->Release();
        pd3dDevice = nullptr;
    }

    MyLogger::LogHRESULT(S_OK, "D3D Device cleaned up successfully", __FILE__, __LINE__);
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

LRESULT WINAPI UI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MyLogger::LogHRESULT(S_OK, "UI::WndProc called", __FILE__, __LINE__);

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        MyLogger::LogHRESULT(S_OK, "ImGui WndProcHandler handled the message", __FILE__, __LINE__);
        return true;
    }

    switch (msg) {
    case WM_SIZE:
        if (pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            MyLogger::LogHRESULT(S_OK, "WM_SIZE: Resizing buffers", __FILE__, __LINE__);
            CleanupRenderTarget();
            pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) {
            MyLogger::LogHRESULT(S_OK, "WM_SYSCOMMAND: SC_KEYMENU detected", __FILE__, __LINE__);
            return 0;
        }
        break;

    case WM_DESTROY:
        MyLogger::LogHRESULT(S_OK, "WM_DESTROY: Posting Quit Message", __FILE__, __LINE__);
        ::PostQuitMessage(0);
        return 0;

    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports) {
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            MyLogger::LogHRESULT(S_OK, "WM_DPICHANGED: Scaling viewports", __FILE__, __LINE__);
        }
        break;

    default:
        break;
    }

    MyLogger::LogHRESULT(S_OK, "UI::WndProc completed", __FILE__, __LINE__);
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void UI::Render() {
    try {
        MyLogger::LogHRESULT(S_OK, "UI::Render called", __FILE__, __LINE__);

        ImGui_ImplWin32_EnableDpiAwareness();
        MyLogger::LogHRESULT(S_OK, "DPI awareness enabled", __FILE__, __LINE__);

        const WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("ImGui Standalone"), nullptr };
        ::RegisterClassEx(&wc);
        MyLogger::LogHRESULT(S_OK, "Window class registered", __FILE__, __LINE__);

        const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("ImGui Standalone"), WS_OVERLAPPEDWINDOW, 100, 100, 50, 50, NULL, NULL, wc.hInstance, NULL);
        MyLogger::LogHRESULT(S_OK, "Window created", __FILE__, __LINE__);

        if (!CreateDeviceD3D(hwnd)) {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            MyLogger::LogHRESULT(E_FAIL, "Failed to create D3D device", __FILE__, __LINE__);
            return;
        }

        ::ShowWindow(hwnd, SW_HIDE);
        ::UpdateWindow(hwnd);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        MyLogger::LogHRESULT(S_OK, "ImGui context created", __FILE__, __LINE__);

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        MyLogger::LogHRESULT(S_OK, "ImGui configuration set", __FILE__, __LINE__);

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 4.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info = {};
        info.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(monitor, &info);
        const int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;

        if (monitor_height > 1080) {
            const float fScale = 2.0f;
            ImFontConfig cfg;
            cfg.SizePixels = 13 * fScale;
            ImGui::GetIO().Fonts->AddFontDefault(&cfg);
            MyLogger::LogHRESULT(S_OK, "High-DPI settings applied", __FILE__, __LINE__);
        }

        ImGui::GetIO().IniFilename = nullptr;

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);
        MyLogger::LogHRESULT(S_OK, "ImGui Win32 and DX11 initialized", __FILE__, __LINE__);

        const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        bool bDone = false;

        while (!bDone) {
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Start", __FILE__, __LINE__);

            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);

                if (msg.message == WM_QUIT) {
                    MyLogger::LogHRESULT(S_OK, "UI Message Loop: WM_QUIT received, exiting loop", __FILE__, __LINE__);
                    bDone = true;
                }
            }

            if (GetAsyncKeyState(VK_END) & 1) {
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: VK_END pressed, exiting loop", __FILE__, __LINE__);
                bDone = true;
            }

            if (bDone) {
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: Loop exited", __FILE__, __LINE__);
                break;
            }

            ImGui_ImplDX11_NewFrame();
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: DX11 NewFrame", __FILE__, __LINE__);
            ImGui_ImplWin32_NewFrame();
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Win32 NewFrame", __FILE__, __LINE__);
            ImGui::NewFrame();
            {
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: ImGui NewFrame", __FILE__, __LINE__);
                Drawing::Draw();
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: ImGui NewFrame Drawing completed", __FILE__, __LINE__);
            }
            ImGui::EndFrame();
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: ImGui EndFrame", __FILE__, __LINE__);
            ImGui::Render();
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: ImGui Render", __FILE__, __LINE__);

            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Colours Cleared", __FILE__, __LINE__);
            pd3dDeviceContext->OMSetRenderTargets(1, &pMainRenderTargetView, nullptr);
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Render Target Set", __FILE__, __LINE__);
            pd3dDeviceContext->ClearRenderTargetView(pMainRenderTargetView, clear_color_with_alpha);
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Render Target Cleared", __FILE__, __LINE__);

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            MyLogger::LogHRESULT(S_OK, "UI Message Loop: ImGui Draw Data Rendered", __FILE__, __LINE__);

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: Viewports Updated", __FILE__, __LINE__);

                ImGui::RenderPlatformWindowsDefault();
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: Viewports Rendered", __FILE__, __LINE__);
            }


            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Before Present", __FILE__, __LINE__);

            HRESULT presentResult = pSwapChain->Present(1, 0);

            if (FAILED(presentResult)) {
                MyLogger::LogHRESULT(presentResult, "UI Message Loop: Present failed", __FILE__, __LINE__);
                // You may want to handle the failure appropriately, depending on your application.
            }
            else {
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: Present succeeded", __FILE__, __LINE__);
            }

            MyLogger::LogHRESULT(S_OK, "UI Message Loop: After Present", __FILE__, __LINE__);

            #ifndef _WINDLL
            if (!Drawing::isActive()) {
                MyLogger::LogHRESULT(S_OK, "UI Message Loop: Drawing is not active, exiting loop", __FILE__, __LINE__);
                break;
            }
            #endif

            MyLogger::LogHRESULT(S_OK, "UI Message Loop: Iteration completed", __FILE__, __LINE__);
        }


        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        MyLogger::LogHRESULT(S_OK, "UI cleanup completed", __FILE__, __LINE__);

        #ifdef _WINDLL
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)FreeLibrary, hCurrentModule, NULL, nullptr);
        #endif

        MyLogger::LogHRESULT(S_OK, "UI::Render completed", __FILE__, __LINE__);
    }
    catch (const std::exception& e) {
        MyLogger::LogHRESULT(E_FAIL, e.what(), __FILE__, __LINE__);
    }
    catch (...) {
        MyLogger::LogHRESULT(E_FAIL, "Unknown exception occurred", __FILE__, __LINE__);
    }
}

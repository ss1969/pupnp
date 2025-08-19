#include "VolumeWindow.h"
#include <stdio.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

VolumeWindow::VolumeWindow() 
    : m_hwnd(nullptr), m_slider(nullptr), m_label(nullptr), m_valueLabel(nullptr),
      m_currentVolume(50), m_isClientMode(false) {
}

VolumeWindow::~VolumeWindow() {
    Destroy();
}

bool VolumeWindow::CreateServiceWindow(const char* title) {
    m_isClientMode = false;
    return CreateClientWindow(title);
}

bool VolumeWindow::CreateClientWindow(const char* title) {
    if (m_hwnd != nullptr) {
        return true; // 窗口已存在
    }
    
    m_isClientMode = (strcmp(title, "音量控制") == 0);
    
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    // 注册窗口类
    const wchar_t* className = L"VolumeWindowClass";
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    
    RegisterClassEx(&wc);
    
    // 转换title为宽字符
    std::wstring wTitle;
    if (title) {
        int len = MultiByteToWideChar(CP_UTF8, 0, title, -1, nullptr, 0);
        wTitle.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, title, -1, &wTitle[0], len);
    }
    
    // 创建窗口
    m_hwnd = CreateWindowEx(
        0,
        className,
        wTitle.c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    CreateControls();
    UpdateVolumeDisplay();
    
    return true;
}

void VolumeWindow::CreateControls() {
    // 创建标签
    const wchar_t* labelText = m_isClientMode ? L"拖动滑块调节音量:" : L"当前音量:";
    m_label = CreateWindow(
        L"STATIC", labelText,
        WS_VISIBLE | WS_CHILD,
        20, 20, 200, 20,
        m_hwnd, nullptr,
        GetModuleHandle(nullptr), nullptr
    );
    
    // 创建滑块
    DWORD sliderStyle = WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_TOOLTIPS;
    if (!m_isClientMode) {
        sliderStyle |= WS_DISABLED; // 服务端模式下禁用滑块
    }
    
    m_slider = CreateWindow(
        TRACKBAR_CLASS, L"",
        sliderStyle,
        20, 50, 200, 30,
        m_hwnd, (HMENU)SLIDER_ID,
        GetModuleHandle(nullptr), nullptr
    );
    
    // 设置滑块范围
    SendMessage(m_slider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessage(m_slider, TBM_SETPOS, TRUE, m_currentVolume);
    
    // 创建数值显示标签
    m_valueLabel = CreateWindow(
        L"STATIC", L"50%",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        230, 50, 50, 30,
        m_hwnd, nullptr,
        GetModuleHandle(nullptr), nullptr
    );
}

void VolumeWindow::Show() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
    }
}

void VolumeWindow::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

void VolumeWindow::SetVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    m_currentVolume = volume;
    
    if (m_slider) {
        SendMessage(m_slider, TBM_SETPOS, TRUE, volume);
    }
    
    UpdateVolumeDisplay();
}

int VolumeWindow::GetVolume() {
    if (m_slider) {
        return (int)SendMessage(m_slider, TBM_GETPOS, 0, 0);
    }
    return m_currentVolume;
}

void VolumeWindow::SetVolumeChangeCallback(std::function<void(int)> callback) {
    m_volumeChangeCallback = callback;
}

void VolumeWindow::ProcessMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void VolumeWindow::Destroy() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        m_slider = nullptr;
        m_label = nullptr;
        m_valueLabel = nullptr;
    }
}

void VolumeWindow::UpdateVolumeDisplay() {
    if (m_valueLabel) {
        wchar_t text[16];
        swprintf_s(text, L"%d%%", m_currentVolume);
        SetWindowText(m_valueLabel, text);
    }
}

LRESULT CALLBACK VolumeWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    VolumeWindow* window = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        window = (VolumeWindow*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    } else {
        window = (VolumeWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (window) {
        switch (uMsg) {
            case WM_HSCROLL:
                if ((HWND)lParam == window->m_slider && window->m_isClientMode) {
                    int newVolume = (int)SendMessage(window->m_slider, TBM_GETPOS, 0, 0);
                    window->m_currentVolume = newVolume;
                    window->UpdateVolumeDisplay();
                    
                    // 调用回调函数
                    if (window->m_volumeChangeCallback) {
                        window->m_volumeChangeCallback(newVolume);
                    }
                }
                break;
                
            case WM_CLOSE:
                window->Hide();
                return 0;
                
            case WM_DESTROY:
                if (window->m_hwnd == hwnd) {
                    window->m_hwnd = nullptr;
                }
                break;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
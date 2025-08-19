#pragma once

#include <windows.h>
#include <commctrl.h>
#include <functional>

class VolumeWindow {
public:
    VolumeWindow();
    ~VolumeWindow();

    // 创建服务端窗口（只显示音量）
    bool CreateServiceWindow(const char* title = "Current Volume");
    
    // 创建客户端窗口（可控制音量）
    bool CreateClientWindow(const char* title = "Volume Control");
    
    // 显示窗口
    void Show();
    
    // 隐藏窗口
    void Hide();
    
    // 设置音量值（0-100）
    void SetVolume(int volume);
    
    // 获取音量值
    int GetVolume();
    
    // 设置音量变化回调（用于客户端）
    void SetVolumeChangeCallback(std::function<void(int)> callback);
    
    // 处理窗口消息
    void ProcessMessages();
    
    // 销毁窗口
    void Destroy();
    
    // 检查窗口是否存在
    bool IsWindowCreated() const { return m_hwnd != nullptr; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void CreateControls();
    void UpdateVolumeDisplay();
    
    HWND m_hwnd;
    HWND m_slider;
    HWND m_label;
    HWND m_valueLabel;
    
    int m_currentVolume;
    bool m_isClientMode;  // true为客户端模式（可控制），false为服务端模式（只显示）
    
    std::function<void(int)> m_volumeChangeCallback;
    
    static const int WINDOW_WIDTH = 300;
    static const int WINDOW_HEIGHT = 150;
    static const int SLIDER_ID = 1001;
};
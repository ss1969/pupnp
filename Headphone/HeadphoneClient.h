#pragma once

#include <string>
#include <vector>

#define UPNP_STATIC_LIB

#include "upnp.h"
#include "ixml.h"
#include "VolumeWindow.h"

struct UpnpDeviceInfo {
    std::string deviceType;
    std::string friendlyName;
    std::string location;
    std::string deviceId;
    std::string controlURL;
    std::string eventURL;
};

class HeadphoneClient {
public:
    HeadphoneClient();
    ~HeadphoneClient();

    // 初始化客户端
    bool Init();
    // 停止客户端
    void Stop();
    // 搜索设备
    const std::vector<UpnpDeviceInfo>& SearchDevices();
    // 设置指定设备的音量
    bool SetVolume(const std::string& deviceId, unsigned char volume);
    // 获取指定设备的音量
    int GetVolume(const std::string& deviceId);
    // 执行音量渐变
    bool ExecuteVolumeRamp(const std::string& deviceId,
                          unsigned char startVolume,
                          unsigned char endVolume,
                          int steps,
                          int durationMs);
    
    // GUI相关方法
    void ShowVolumeControlWindow(const std::string& deviceId);
    void HideVolumeControlWindow();
    void ProcessWindowMessages();

private:
    static int DiscoveryCallback(Upnp_EventType eventType, const void* event, void* cookie);

    UpnpClient_Handle m_clientHandle;
    std::vector<UpnpDeviceInfo> m_discoveredDevices;
    VolumeWindow m_volumeWindow;
    std::string m_currentDeviceId;
    static HeadphoneClient* s_instance;
};

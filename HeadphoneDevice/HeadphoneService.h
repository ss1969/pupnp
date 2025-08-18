#pragma once

#define UPNP_STATIC_LIB

#include <mutex>
#include "upnpconfig.h"
#include "upnp.h"
#include "UpnpString.h"
#include "upnpactionrequest.h"
#include "ixml.h"
#include "upnptools.h"

class HeadphoneService {
public:
    HeadphoneService();
    ~HeadphoneService();

    // 启动服务
    bool Start();
    // 停止服务
    void Stop();
    // 获取当前音量
    unsigned char GetVolume();
    // 设置音量
    bool SetVolume(unsigned char volume);

private:
    // 处理UPnP动作请求的静态回调函数
    static int ActionHandler(Upnp_EventType eventType, UpnpActionRequest *request);
    static int EventHandler(Upnp_EventType eventType, const void* event, void* cookie);

    // 成员变量
    UpnpDevice_Handle m_deviceHandle;
    unsigned char m_currentVolume;
    std::mutex m_volumeMutex;
    bool m_isRunning;

    static HeadphoneService* s_instance;
};

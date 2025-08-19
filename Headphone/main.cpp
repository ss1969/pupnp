#pragma warning(disable: 4996) // 禁用scanf等不安全函数警告
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <locale.h>
#include <conio.h>
#include "HeadphoneService.h"
#include "HeadphoneClient.h"

// 函数前向声明
const UpnpDeviceInfo *SelectDevice( HeadphoneClient &client );

void ShowMenu()
{
    printf( "\n=== UPnP 虚拟耳机演示 ===\n" );
    printf( "1. 启动虚拟耳机设备服务\n" );
    printf( "2. 搜索并控制耳机设备\n" );
    printf( "3. 音量渐变测试\n" );
    printf( "4. 手动控制音量\n" );
    printf( "5. 启动服务并显示音量窗口\n" );
    printf( "6. 客户端GUI音量控制\n" );
    printf( "0. 退出\n" );
    printf( "请选择操作:" );
}

void StartHeadphoneService()
{
    HeadphoneService service;
    if( service.Start() )
    {
        printf( "\n虚拟耳机设备已启动。按回车键停止服务...\n" );
        getchar();
        getchar();
        service.Stop();
    }
}

void StartHeadphoneServiceWithGUI()
{
    HeadphoneService service;
    if( service.Start() )
    {
        service.ShowVolumeWindow();
        printf( "\n虚拟耳机设备已启动，音量窗口已显示。按回车键停止服务...\n" );
        
        // 处理窗口消息直到用户按回车
        bool running = true;
        while( running )
        {
            service.ProcessWindowMessages();
            
            // 检查是否有键盘输入
            if( _kbhit() )
            {
                int ch = _getch();
                if( ch == '\r' || ch == '\n' )
                {
                    running = false;
                }
            }
            
            Sleep(10); // 避免CPU占用过高
        }
        
        service.HideVolumeWindow();
        service.Stop();
    }
}

void ClientGUIVolumeControl()
{
    HeadphoneClient client;
    if( !client.Init() )
    {
        printf( "客户端初始化失败！\n" );
        return;
    }

    auto *device = SelectDevice( client );
    if( !device )
        return;

    client.ShowVolumeControlWindow( device->deviceId );
    printf( "\n音量控制窗口已显示。按回车键关闭窗口...\n" );
    
    // 处理窗口消息直到用户按回车
    bool running = true;
    while( running )
    {
        client.ProcessWindowMessages();
        
        // 检查是否有键盘输入
        if( _kbhit() )
        {
            int ch = _getch();
            if( ch == '\r' || ch == '\n' )
            {
                running = false;
            }
        }
        
        Sleep(10); // 避免CPU占用过高
    }
    
    client.HideVolumeControlWindow();
}

const UpnpDeviceInfo *SelectDevice( HeadphoneClient &client )
{
    const std::vector<UpnpDeviceInfo> &devices = client.SearchDevices();
    if( devices.empty() )
    {
        printf( "未找到任何耳机设备\n" );
        return nullptr;
    }

    for( size_t i = 0; i < devices.size(); ++i )
    {
        printf( "%zu. %s (%s)\n", i + 1, devices[i].friendlyName.c_str(), devices[i].deviceId.c_str() );
    }

    printf( "\n请选择设备 (0退出): " );
    int choice;
    scanf( "%d", &choice );

    if( choice > 0 && choice <= devices.size() )
    {
        return &devices[ choice - 1 ];
    }
    return nullptr;
}

void ControlDevice()
{
    HeadphoneClient client;
    if( !client.Init() )
    {
        printf( "客户端初始化失败！\n" );
        return;
    }

    auto *device = SelectDevice( client );
    if( !device )
        return;

    while( true )
    {
        printf( "\n=== 设备控制菜单 ===\n" );
        printf( "1. 获取当前音量\n" );
        printf( "2. 设置音量\n" );
        printf( "0. 返回上级菜单\n" );
        printf( "请选择操作: " );

        int choice;
        scanf( "%d", &choice );

        if( choice == 0 )
            break;

        switch( choice )
        {
            case 1:
            {
                int volume = client.GetVolume( device->deviceId );
                if( volume >= 0 )
                {
                    printf( "当前音量: %d%%\n", volume );
                }
                break;
            }
            case 2:
            {
                printf( "请输入新的音量值 (0-100): " );
                int newVolume;
                scanf( "%d", &newVolume );
                if( newVolume >= 0 && newVolume <= 100 )
                {
                    client.SetVolume( device->deviceId, ( unsigned char )newVolume );
                }
                else
                {
                    printf( "无效的音量值！\n" );
                }
                break;
            }
            default:
                printf( "无效的选项！\n" );
                break;
        }
    }
}

void ExecuteVolumeTest()
{
    HeadphoneClient client;
    if( !client.Init() )
    {
        printf( "客户端初始化失败！\n" );
        return;
    }

    auto *device = SelectDevice( client );
    if( !device )
        return;

    printf( "\n=== 音量渐变测试 ===\n" );
    printf( "1. 渐强 (20%% -> 80%%)\n" );
    printf( "2. 渐弱 (80%% -> 20%%)\n" );
    printf( "3. 自定义渐变\n" );
    printf( "0. 返回\n" );
    printf( "请选择: " );

    int choice;
    scanf( "%d", &choice );

    switch( choice )
    {
        case 1:
            printf( "执行渐强测试...\n" );
            client.ExecuteVolumeRamp( device->deviceId, 20, 80, 30, 3000 );
            break;
        case 2:
            printf( "执行渐弱测试...\n" );
            client.ExecuteVolumeRamp( device->deviceId, 80, 20, 30, 3000 );
            break;
        case 3:
        {
            unsigned char startVol, endVol;
            int           steps, duration;
            printf( "起始音量 (0-100): " );
            scanf( "%hhu", &startVol );
            printf( "结束音量 (0-100): " );
            scanf( "%hhu", &endVol );
            printf( "步数: " );
            scanf( "%d", &steps );
            printf( "持续时间(毫秒): " ); 
            scanf( "%d", &duration );

            if( startVol <= 100 && endVol <= 100 && steps > 0 && duration > 0 )
            {
                client.ExecuteVolumeRamp( device->deviceId, startVol, endVol, steps, duration );
            }
            else
            {
                printf( "输入参数无效！\n" );
            }
            break;
        }
    }
}

int main( int argc, const char *argv[] )
{
    // 设置中文支持
    // setlocale(LC_ALL, "chs");  // 中文简体
    // // SetConsoleCP(936);   // GBK输入
    // // SetConsoleOutputCP(936);  // GBK输出
    
     // 如果上面的设置不工作，可以尝试UTF-8
     //SetConsoleCP(65001);   // UTF-8输入
     //SetConsoleOutputCP(65001);  // UTF-8输出
    
    int ret = UpnpInit2( NULL, 0 );
    if( ret != UPNP_E_SUCCESS )
    {
        printf( "UPnP初始化失败: %d\n", ret );
        return -1;
    }

    printf( "UPnP初始化成功\n" );
    printf( "IP地址: %s\n", UpnpGetServerIpAddress() );
    printf( "端口号: %d\n", UpnpGetServerPort() );

    while( true )
    {
        ShowMenu();

        int choice;
        scanf( "%d", &choice );

        switch( choice )
        {
            case 0:
                goto cleanup;
            case 1:
                StartHeadphoneService();
                break;
            case 2:
                ControlDevice();
                break;
            case 3:
                ExecuteVolumeTest();
                break;
            case 4:
            {
                HeadphoneClient client;
                if( !client.Init() )
                {
                    printf( "客户端初始化失败！\n" );
                    break;
                }

                auto *device = SelectDevice( client );
                if( !device )
                    break;

                printf( "请输入音量值 (0-100):" );
                int volume;
                scanf( "%d", &volume );
                if( volume >= 0 && volume <= 100 )
                {
                    client.SetVolume( device->deviceId, ( unsigned char )volume );
                }
                else
                {
                    printf( "无效的音量值！\n" );
                }
                break;
            }
            case 5:
                StartHeadphoneServiceWithGUI();
                break;
            case 6:
                ClientGUIVolumeControl();
                break;
            default:
                printf( "无效的选项！\n" );
                break;
        }
    }

cleanup:
    printf( "正在关闭...\n" );
    UpnpFinish();
    return 0;
}

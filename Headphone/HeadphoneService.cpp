#include "HeadphoneService.h"
#include <stdio.h>
#include <string.h>

HeadphoneService *HeadphoneService::s_instance = nullptr;

HeadphoneService::HeadphoneService() : m_deviceHandle( -1 ), m_currentVolume( 50 ), m_isRunning( false )
{
    s_instance = this;
}

HeadphoneService::~HeadphoneService()
{
    Stop();
}

bool HeadphoneService::Start()
{
    if( m_isRunning )
        return true;

    int ret = UpnpSetWebServerRootDir("./");
    if(ret != UPNP_E_SUCCESS) {
        printf("设置服务描述文件目录失败: %d\n", ret);
        return false;
    }

    // 注册设备
    const char *deviceDesc = "headphone_device.xml";
    ret        = UpnpRegisterRootDevice2( UPNPREG_FILENAME_DESC, deviceDesc, 0, 1, EventHandler, NULL, &m_deviceHandle );

    if( ret != UPNP_E_SUCCESS )
    {
        printf( "设备注册失败: %d\n", ret );
        return false;
    }

    // 发送设备广告
    ret = UpnpSendAdvertisement( m_deviceHandle, 100 );
    if( ret != UPNP_E_SUCCESS )
    {
        printf( "设备广告发送失败: %d\n", ret );
        return false;
    }

    m_isRunning = true;
    printf( "虚拟耳机设备已启动...\n" );
    return true;
}

void HeadphoneService::Stop()
{
    if( !m_isRunning )
        return;

    UpnpUnRegisterRootDevice( m_deviceHandle );
    m_isRunning = false;
    printf( "虚拟耳机设备已停止\n" );
}

unsigned char HeadphoneService::GetVolume()
{
    std::lock_guard< std::mutex > lock( m_volumeMutex );
    return m_currentVolume;
}

bool HeadphoneService::SetVolume( unsigned char volume )
{
    if( volume > 100 )
    {
        printf( "无效的音量值: %d（应在0-100范围内）\n", volume );
        return false;
    }

    {
        std::lock_guard< std::mutex > lock( m_volumeMutex );
        m_currentVolume = volume;
    }
    
    // 更新GUI窗口显示
    if( m_volumeWindow.IsWindowCreated() )
    {
        m_volumeWindow.SetVolume( volume );
    }

    // 准备发送事件通知
    char xml[ 256 ];
    snprintf( xml,
              sizeof( xml ),
              "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">"
              "<e:property>"
              "<Volume>%d</Volume>"
              "</e:property>"
              "</e:propertyset>",
              volume );

    // 解析XML字符串为IXML文档
    IXML_Document *doc = ixmlParseBuffer( xml );
    if( !doc )
    {
        printf( "创建事件通知文档失败\n" );
        return false;
    }

    // 发送状态变更事件
    int ret = UpnpNotifyExt( m_deviceHandle, "uuid:JBLHeadphoneDevice-12345678", "urn:upnp-org:serviceId:HeadphoneVolume", doc );

    // 清理文档
    ixmlDocument_free( doc );

    printf( "音量已设置为: %d%%\n", volume );
    return ( ret == UPNP_E_SUCCESS );
}

void HeadphoneService::ShowVolumeWindow()
{
    if( !m_volumeWindow.IsWindowCreated() )
    {
        m_volumeWindow.CreateServiceWindow( "耳机设备 - 当前音量" );
        m_volumeWindow.SetVolume( m_currentVolume );
    }
    m_volumeWindow.Show();
}

void HeadphoneService::HideVolumeWindow()
{
    m_volumeWindow.Hide();
}

void HeadphoneService::ProcessWindowMessages()
{
    if( m_volumeWindow.IsWindowCreated() )
    {
        m_volumeWindow.ProcessMessages();
    }
}

int HeadphoneService::ActionHandler( Upnp_EventType eventType, UpnpActionRequest *request )
{
    const char    *devUDN       = UpnpString_get_String( UpnpActionRequest_get_DevUDN( request ) );
    const char    *serviceID    = UpnpString_get_String( UpnpActionRequest_get_ServiceID( request ) );
    const char    *actionName   = UpnpString_get_String( UpnpActionRequest_get_ActionName( request ) );
    IXML_Document *actionResult = NULL;

    // check sevice
    if( strcmp( serviceID, "urn:upnp-org:serviceId:HeadphoneVolume" ) == 0 )
    {
        if( strcmp( actionName, "SetVolume" ) == 0 )
        {
            // set volume
            IXML_Document *actionDoc = ( IXML_Document * )UpnpActionRequest_get_ActionRequest( request );
            if( actionDoc )
            {
                IXML_NodeList *volumeList = ixmlDocument_getElementsByTagName( actionDoc, "NewVolume" );
                if( volumeList && ixmlNodeList_length( volumeList ) > 0 )
                {
                    IXML_Node *volumeNode = ixmlNodeList_item( volumeList, 0 );
                    IXML_Node *textNode   = ixmlNode_getFirstChild( volumeNode );
                    if( textNode )
                    {
                        const char   *volumeStr = ixmlNode_getNodeValue( textNode );
                        unsigned char newVolume = ( unsigned char )atoi( volumeStr );
                        if( s_instance->SetVolume( newVolume ) )
                        {
                            // 成功响应 - 必须返回空的响应文档
                            const char *response = "<u:SetVolumeResponse xmlns:u=\"urn:schemas-upnp-org:service:HeadphoneVolume:1\"/>";

                            actionResult = ixmlParseBuffer( response );
                            if( !actionResult )
                            {
                                ixmlNodeList_free( volumeList );
                                UpnpActionRequest_set_ErrCode( request, UPNP_E_INTERNAL_ERROR );
                                return UPNP_E_INTERNAL_ERROR;
                            }

                            UpnpActionRequest_set_ActionResult( request, actionResult );
                            UpnpActionRequest_set_ErrCode( request, UPNP_E_SUCCESS );
                            ixmlNodeList_free( volumeList );
                            // 注意：不要在这里释放actionResult，UPnP库会自动处理
                            return UPNP_E_SUCCESS;
                        }
                        else
                        {
                            // SetVolume失败
                            ixmlNodeList_free( volumeList );
                            UpnpActionRequest_set_ErrCode( request, UPNP_E_INTERNAL_ERROR );
                            return UPNP_E_INTERNAL_ERROR;
                        }
                    }
                    else
                    {
                        // 无法获取音量值
                        if( volumeList )
                            ixmlNodeList_free( volumeList );
                        UpnpActionRequest_set_ErrCode( request, UPNP_SOAP_E_INVALID_ARGS );
                        return UPNP_SOAP_E_INVALID_ARGS; 
                    }
                }
                else
                {
                    // 无法找到NewVolume参数
                    if( volumeList )
                        ixmlNodeList_free( volumeList );
                    UpnpActionRequest_set_ErrCode( request, UPNP_SOAP_E_INVALID_ARGS );
                    return UPNP_SOAP_E_INVALID_ARGS;
                }
            }
            else
            {
                // 无法获取Action请求文档
                UpnpActionRequest_set_ErrCode( request, UPNP_SOAP_E_INVALID_ARGS );
                return UPNP_SOAP_E_INVALID_ARGS;
            }
        }
        else if( strcmp( actionName, "GetVolume" ) == 0 )
        {
            // get volume
            char response[ 256 ];
            snprintf( response,
                      sizeof( response ),
                      "<u:GetVolumeResponse xmlns:u=\"urn:schemas-upnp-org:service:HeadphoneVolume:1\">"
                      "<CurrentVolume>%d</CurrentVolume>"
                      "</u:GetVolumeResponse>",
                      s_instance->GetVolume() );

            // 将响应字符串解析为IXML文档
            actionResult = ixmlParseBuffer( response );
            if( actionResult )
            {
                UpnpActionRequest_set_ActionResult( request, actionResult );
                UpnpActionRequest_set_ErrCode( request, UPNP_E_SUCCESS );
                return UPNP_E_SUCCESS;
            }

            UpnpActionRequest_set_ErrCode( request, UPNP_E_INTERNAL_ERROR );
            return UPNP_E_INTERNAL_ERROR;
        }
    }

    UpnpActionRequest_set_ErrCode( request, UPNP_E_INVALID_ACTION );
    return UPNP_E_INVALID_ACTION;
}

int HeadphoneService::EventHandler( Upnp_EventType eventType, const void *event, void *cookie )
{
    switch( eventType )
    {
        case UPNP_CONTROL_ACTION_REQUEST:
            return ActionHandler( eventType, ( UpnpActionRequest * )event );
        case UPNP_CONTROL_GET_VAR_REQUEST:
            printf( "EventHandler 变量查询请求" );
            // 可以在这里处理状态变量查询请求
            break;
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            printf( "EventHandler 订阅请求" );
            // 可以在这里处理订阅请求
            break;
        default:
            break;
    }
    return UPNP_E_SUCCESS;
}

#include "HeadphoneClient.h"
#include <thread>
#include <chrono>
#include <algorithm>

HeadphoneClient *HeadphoneClient::s_instance = nullptr;

HeadphoneClient::HeadphoneClient() : m_clientHandle( -1 )
{
    s_instance = this;
}

HeadphoneClient::~HeadphoneClient()
{
    Stop();
}

bool HeadphoneClient::Init()
{
    int ret = UpnpRegisterClient( DiscoveryCallback, NULL, &m_clientHandle );
    if( ret != UPNP_E_SUCCESS )
    {
        printf( "客户端初始化失败: %d\n", ret );
        return false;
    }
    printf( "耳机控制客户端已初始化\n" );
    return true;
}

void HeadphoneClient::Stop()
{
    if( m_clientHandle != -1 )
    {
        UpnpUnRegisterClient( m_clientHandle );
        m_clientHandle = -1;
        printf( "耳机控制客户端已停止\n" );
    }
}

const std::vector<UpnpDeviceInfo>& HeadphoneClient::SearchDevices()
{
    m_discoveredDevices.clear();
    printf( "开始搜索虚拟耳机设备...\n" );

    // 搜索特定类型的设备
    int ret = UpnpSearchAsync( m_clientHandle, 3, "urn:schemas-upnp-org:device:HeadphoneDevice:1", NULL );

    if( ret != UPNP_E_SUCCESS )
    {
        printf( "设备搜索失败: %d\n", ret );
        return m_discoveredDevices;
    }

    // 2s
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );

    if( m_discoveredDevices.empty() )
    {
        printf( "未发现任何虚拟耳机设备\n" );
    }
    else
    {
        printf( "发现了 %zu 个虚拟耳机设备\n", m_discoveredDevices.size() );
        int index = 1;
        for( const UpnpDeviceInfo &device : m_discoveredDevices )
        {
            printf( "%d. %s (%s)\n", index++, device.friendlyName.c_str(), device.deviceId.c_str() );
        }
    }

    return m_discoveredDevices;
}

bool HeadphoneClient::SetVolume( const std::string &deviceId, unsigned char volume )
{
    if( volume > 100 )
    {
        printf( "无效的音量值: %d (应在0-100范围内)\n", volume );
        return false;
    }

    // 查找设备
    auto it = std::find_if( m_discoveredDevices.begin(), m_discoveredDevices.end(), [ &deviceId ]( const UpnpDeviceInfo &device ) { return device.deviceId == deviceId; } );
    if( it == m_discoveredDevices.end() )
    {
        printf( "未找到设备: %s\n", deviceId.c_str() );
        return false;
    }

    const UpnpDeviceInfo &device = *it;
    const char    *serviceType = "urn:schemas-upnp-org:service:HeadphoneVolume:1";
    IXML_Document *actionNode  = NULL;

    // 创建XML文档
    IXML_Document *actionDoc = ixmlDocument_createDocument();
    if( !actionDoc )
    {
        printf( "创建XML文档失败\n" );
        return false;
    }

    IXML_Element *actionElement = ixmlDocument_createElement( actionDoc, "u:SetVolume" );
    if( !actionElement )
    {
        printf( "创建SetVolume元素失败\n" );
        ixmlDocument_free( actionDoc );
        return false;
    }

    ixmlElement_setAttribute( actionElement, "xmlns:u", serviceType );

    // 创建音量元素
    IXML_Element *volumeElement = ixmlDocument_createElement( actionDoc, "NewVolume" );
    if( !volumeElement )
    {
        printf( "创建NewVolume元素失败\n" );
        ixmlDocument_free( actionDoc );
        return false;
    }

    char volumeStr[ 16 ];
    snprintf( volumeStr, sizeof( volumeStr ), "%d", volume );
    IXML_Node *volumeText = ixmlDocument_createTextNode( actionDoc, volumeStr );

    ixmlNode_appendChild( ( IXML_Node * )volumeElement, volumeText );
    ixmlNode_appendChild( ( IXML_Node * )actionElement, ( IXML_Node * )volumeElement );
    ixmlNode_appendChild( ( IXML_Node * )actionDoc, ( IXML_Node * )actionElement );

    // 发送动作请求
    int ret = UpnpSendAction( m_clientHandle,
                              device.controlURL.c_str(),
                              serviceType,
                              NULL,
                              actionDoc,
                              &actionNode );

    bool success = ( ret == UPNP_E_SUCCESS );
    if( success )
    {
        printf( "音量已设置为 %d%%\n", volume );
    }
    else
    {
        printf( "设置音量失败: %d\n", ret );
    }

    if( actionNode )
        ixmlDocument_free( actionNode );
    if( actionDoc )
        ixmlDocument_free( actionDoc );

    return success;
}

int HeadphoneClient::GetVolume( const std::string &deviceId )
{
    const char    *serviceType = "urn:schemas-upnp-org:service:HeadphoneVolume:1";
    IXML_Document *response    = NULL;

    // 查找设备
    auto it = std::find_if( m_discoveredDevices.begin(), m_discoveredDevices.end(), [ &deviceId ]( const UpnpDeviceInfo &device ) { return device.deviceId == deviceId; } );
    if( it == m_discoveredDevices.end() )
    {
        printf( "未找到设备: %s\n", deviceId.c_str() );
        return false;
    }

    const UpnpDeviceInfo &device = *it;

    // 创建XML文档
    IXML_Document *actionDoc = ixmlDocument_createDocument();
    if( !actionDoc )
    {
        printf( "创建XML文档失败\n" );
        return -1;
    }

    IXML_Element *actionElement = ixmlDocument_createElement( actionDoc, "u:GetVolume" );
    if( !actionElement )
    {
        printf( "创建GetVolume元素失败\n" );
        ixmlDocument_free( actionDoc );
        return -1;
    }

    ixmlElement_setAttribute( actionElement, "xmlns:u", serviceType );
    ixmlNode_appendChild( ( IXML_Node * )actionDoc, ( IXML_Node * )actionElement );

    // 发送动作请求
    int ret = UpnpSendAction( m_clientHandle, device.controlURL.c_str(), serviceType, NULL, actionDoc, &response );

    int volume = -1;
    if( ret == UPNP_E_SUCCESS && response )
    {
        IXML_NodeList *volumeList = ixmlDocument_getElementsByTagName( response, "CurrentVolume" );
        if( volumeList )
        {
            IXML_Node *volumeNode = ixmlNodeList_item( volumeList, 0 );
            if( volumeNode )
            {
                IXML_Node *textNode = ixmlNode_getFirstChild( volumeNode );
                if( textNode )
                {
                    const char *volumeValue = ixmlNode_getNodeValue( textNode );
                    volume                  = atoi( volumeValue );
                    printf( "当前音量: %d%%\n", volume );
                }
            }
            ixmlNodeList_free( volumeList );
        }
        ixmlDocument_free( response );
    }
    else
    {
        printf( "获取音量失败: %d\n", ret );
    }

    if( actionDoc )
        ixmlDocument_free( actionDoc );
    return volume;
}

bool HeadphoneClient::ExecuteVolumeRamp( const std::string &deviceId, unsigned char startVolume, unsigned char endVolume, int steps, int durationMs )
{
    printf( "开始音量渐变: %d%% -> %d%% (步数: %d, 持续时间: %dms)\n", startVolume, endVolume, steps, durationMs );

    if( !SetVolume( deviceId, startVolume ) )
    {
        printf( "设置起始音量失败\n" );
        return false;
    }

    float volumeDelta = ( float )( endVolume - startVolume ) / steps;
    int   delayMs     = durationMs / steps;

    for( int i = 1; i <= steps; ++i )
    {
        unsigned char currentVolume = ( unsigned char )( startVolume + ( volumeDelta * i ) );
        if( !SetVolume( deviceId, currentVolume ) )
        {
            //printf( "音量渐变过程中失败\n" );
            //return false;
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( delayMs ) );
    }

    printf( "音量渐变完成\n" );
    return true;
}

int HeadphoneClient::DiscoveryCallback( Upnp_EventType eventType, const void *event, void *cookie )
{
    if( !s_instance )
        return UPNP_E_SUCCESS;

    switch( eventType )
    {
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
        {
            const UpnpDiscovery *discovery = ( const UpnpDiscovery * )event;
            UpnpDeviceInfo       deviceInfo;
            deviceInfo.deviceType = UpnpDiscovery_get_DeviceType_cstr( discovery );
            deviceInfo.location   = UpnpDiscovery_get_Location_cstr( discovery );
            deviceInfo.deviceId   = UpnpDiscovery_get_DeviceID_cstr( discovery );
            size_t pos = deviceInfo.location.find( "/headphone_device.xml" );
            std::string baseURL;
            if( pos != std::string::npos )
                baseURL   = deviceInfo.location.substr( 0, pos );

            // 检查设备是否已存在
            auto it = std::find_if(s_instance->m_discoveredDevices.begin(), s_instance->m_discoveredDevices.end(),
                [&deviceInfo](const UpnpDeviceInfo& device) {
                    return device.deviceId == deviceInfo.deviceId;
                });
            if (it != s_instance->m_discoveredDevices.end()) {
                // 设备已存在，跳过
                break;
            }

            // 获取设备描述文档
            IXML_Document *doc = NULL;
            int            ret = UpnpDownloadXmlDoc( deviceInfo.location.c_str(), &doc );
            if( ret == UPNP_E_SUCCESS && doc )
            {
                // 获取友好名称
                IXML_NodeList *friendlyNameList = ixmlDocument_getElementsByTagName( doc, "friendlyName" );
                if( friendlyNameList )
                {
                    IXML_Node *nameNode = ixmlNodeList_item( friendlyNameList, 0 );
                    if( nameNode )
                    {
                        IXML_Node *textNode = ixmlNode_getFirstChild( nameNode );
                        if( textNode )
                        {
                            deviceInfo.friendlyName = ixmlNode_getNodeValue( textNode );
                            printf( "发现设备: %s (%s)\n", deviceInfo.friendlyName.c_str(), deviceInfo.deviceId.c_str() );
                        }
                    }
                    ixmlNodeList_free( friendlyNameList );
                }

                // 获取服务列表 (serviceList) 并提取 controlURL
                IXML_NodeList *serviceList = ixmlDocument_getElementsByTagName( doc, "serviceList" );
                if( serviceList && ixmlNodeList_length( serviceList ) > 0 )
                {
                    IXML_Node     *serviceListNode = ixmlNodeList_item( serviceList, 0 );
                    IXML_NodeList *services        = ixmlNode_getChildNodes( serviceListNode );
                    if( services )
                    {
                        for( unsigned i = 0; i < ixmlNodeList_length( services ); i++ )
                        {
                            IXML_Node *serviceNode = ixmlNodeList_item( services, i );
                            if( serviceNode )
                            {
                                // 检查服务类型是否为 HeadphoneVolume
                                IXML_Node  *serviceTypeNode  = ixmlNode_getFirstChild( serviceNode );
                                const char *serviceTypeValue = ixmlNode_getNodeValue( ixmlNode_getFirstChild( serviceTypeNode ) );
                                if( serviceTypeValue && strstr( serviceTypeValue, "HeadphoneVolume:1" ) )
                                {
                                    // Get controlURL and eventURL
                                    IXML_Node *childNode = serviceTypeNode;
                                    while( ( childNode = ixmlNode_getNextSibling( childNode ) ) != nullptr )
                                    {
                                        const char *nodeName = ixmlNode_getNodeName( childNode );
                                        if( nodeName && strcmp( nodeName, "controlURL" ) == 0 )
                                        {
                                            const char *controlURLValue = ixmlNode_getNodeValue( ixmlNode_getFirstChild( childNode ) );
                                            deviceInfo.controlURL = controlURLValue ? baseURL + controlURLValue : "";
                                        }
                                        if( nodeName && strcmp( nodeName, "eventURL" ) == 0 )
                                        {
                                            const char *eventURLValue = ixmlNode_getNodeValue( ixmlNode_getFirstChild( childNode ) );
                                            deviceInfo.eventURL = eventURLValue ? baseURL + eventURLValue : "";
                                        }
                                    }
                                }
                            }
                        }
                        ixmlNodeList_free( services );
                    }
                }
                ixmlNodeList_free( serviceList );


                ixmlDocument_free( doc );
            }

            s_instance->m_discoveredDevices.push_back( deviceInfo );
            break;
        }
        default:
            break;
    }

    return UPNP_E_SUCCESS;
}
